/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/** @file
 * @brief Convert management messages to json
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#include <stack>
#include <cmath>
#include <mutex>
#include <dlfcn.h>
#include "comp.h"

__PTPMGMT_NAMESPACE_BEGIN

// From JSON part
static int Json2msgCount = 0; // Count how many objects exist
#ifdef PIC // Shared library code
static void *jsonLib = nullptr;
static const char *useLib = nullptr;
static std::mutex jsonLoadLock; // Lock loading and unloading
#define funcName(fname) ptpm_json_##fname##_p
#define funcDeclare0(fret, fname, fargs)\
    typedef fret(*fname##_t)(fargs);\
    fname##_t fname##_p = nullptr
#define funcAssign0(fname)\
    fname##_p = (fname##_t)dlsym(jsonLib, #fname);\
    if(fname##_p == nullptr)\
        return false
#define funcAssign(fname) funcAssign0(ptpm_json_##fname)
#else // PIC
#define funcName(fname) ptpm_json_##fname
#define funcDeclare0(fret, fname, fargs) fret fname(fargs)
#endif // PIC

#define funcDeclare(fret, fname, fargs) funcDeclare0(fret, ptpm_json_##fname, fargs)
extern "C" {
    funcDeclare(void *, parse, const char *json);
    funcDeclare(void, free, void *jobj);
    funcDeclare(JsonProcFrom *, alloc_proc,);
    funcDeclare(const char *, name,); // Used in static only
}

#ifdef PIC
static void doLibRm()
{
    if(dlclose(jsonLib) != 0)
        PTPMGMT_ERROR("Fail to unload the libptpmngt from JSON library: %s",
            dlerror());
}
static void doLibNull()
{
    jsonLib = nullptr;
    useLib = nullptr;
    funcName(parse) = nullptr;
    funcName(free) = nullptr;
    funcName(alloc_proc) = nullptr;
}
static inline bool loadFuncs()
{
    funcAssign(parse);
    funcAssign(free);
    funcAssign(alloc_proc);
    return true;
}
static bool tryLib(const char *name)
{
    jsonLib = dlopen(name, RTLD_LAZY);
    if(jsonLib != nullptr) {
        if(loadFuncs()) {
            useLib = name;
            return true;
        }
        doLibRm();
    }
    return false;
}
static inline bool loadMatchLibrary(const char *libMatch, const char *found)
{
    if(found == nullptr) {
        PTPMGMT_ERROR("Fail to find any library to matche pattern '%s'", libMatch);
        return false;
    } else if(jsonLib == nullptr) {
        // Try loading
        if(!tryLib(found)) {
            doLibNull(); // Ensure all pointers stay null
            PTPMGMT_ERROR("Fail loading the matched fromJson library '%s' for "
                "pattern '%s'", found, libMatch);
            return false;
        }
    }
    // Already load, just check if it is the library we want
    else if(useLib != found) {  // We compare pointers, not strings!
        PTPMGMT_ERROR("Already load a different library '%s', not the "
            "matched '%s' to pattern '%s'", useLib, found, libMatch);
        return false;
    }
    return true;
}
static bool doLoadLibrary(const char *libMatch = nullptr)
{
    std::unique_lock<std::mutex> lock(jsonLoadLock);
    const char *list[] = { JSON_C nullptr };
    if(libMatch != nullptr) {
        const char *found = nullptr;
        for(const char **cur = list; *cur != nullptr; cur++) {
            if(strcasestr(*cur, libMatch) != nullptr) {
                if(found != nullptr) {
                    PTPMGMT_ERROR("Found multiple libraries match to pattern '%s'",
                        libMatch);
                    return false;
                }
                found = *cur;
            }
        }
        return loadMatchLibrary(libMatch, found);
    }
    if(jsonLib != nullptr)
        return true;
    for(const char **cur = list; *cur != nullptr; cur++) {
        if(tryLib(*cur))
            return true;
    }
    doLibNull(); // Ensure all pointers stay null
    PTPMGMT_ERROR("fail loading a fromJson library");
    return false;
}
static inline void libFree()
{
    std::unique_lock<std::mutex> lock(jsonLoadLock);
    if(Json2msgCount <= 0) {
        if(jsonLib != nullptr) {
            doLibRm();
            doLibNull(); // mark all pointers null
        }
        Json2msgCount = 0;
    }
}
#define LIB_LOAD(a) \
    if(!doLoadLibrary(a)) \
        return false
#define LIB_FREE libFree()
#define LIB_NAME useLib
#define LIB_SHARED true
#else // PIC
#define LIB_LOAD(a)
#define LIB_FREE
#define LIB_NAME funcName(name)()
#define LIB_SHARED false
#endif // PIC

Json2msg::Json2msg():
    m_managementId(NULL_PTP_MANAGEMENT),
    m_action(GET),
    m_have{0}
{
    Json2msgCount++;
}
Json2msg::~Json2msg()
{
    Json2msgCount--;
    LIB_FREE;
}
const char *Json2msg::loadLibrary()
{
    return LIB_NAME;
}
bool Json2msg::selectLib(const std::string &libMatch)
{
    LIB_LOAD(libMatch.c_str());
    return false;
}
bool Json2msg::isLibShared()
{
    return LIB_SHARED;
}
bool Json2msg::fromJson(const std::string &json)
{
    LIB_LOAD();
    void *jobj = funcName(parse)(json.c_str());
    if(jobj == nullptr) {
        PTPMGMT_ERROR("JSON parse fail");
        return false;
    }
    bool ret = fromJsonObj(jobj);
    funcName(free)(jobj);
    return ret;
}
bool Json2msg::fromJsonObj(const void *jobj)
{
    LIB_LOAD();
    std::unique_ptr<JsonProcFrom> hold;
    JsonProcFrom *pproc = funcName(alloc_proc)();
    if(pproc == nullptr) {
        PTPMGMT_ERROR("fromJsonObj fail allocation of JsonProcFrom");
        return false;
    }
    hold.reset(pproc); // delete the object once we return :-)
    PTPMGMT_ERROR_CLR;
    if(!pproc->mainProc(jobj))
        return false;
    const std::string &str = pproc->getActionField();
    if(str.empty()) {
        PTPMGMT_ERROR("Message do not have an action field");
        return false;
    } else if(str == "GET")
        m_action = GET;
    else if(str == "SET")
        m_action = SET;
    else if(str == "COMMAND")
        m_action = COMMAND;
    else {
        PTPMGMT_ERROR("Message have wrong action field '%s'", str.c_str());
        return false;
    }
    const char *mngStrID;
    if(!pproc->procMng(m_managementId, mngStrID))
        return false;
    // Optional
    int64_t val;
#define optProc(key)\
    if(pproc->getIntVal(#key, val)) {\
        m_have[have_##key] = true;\
        m_##key = val;\
    }
    optProc(sequenceId)
    optProc(sdoId)
    optProc(domainNumber)
    optProc(versionPTP)
    optProc(minorVersionPTP)
    optProc(PTPProfileSpecific)
    if(pproc->getUnicastFlag(m_unicastFlag))
        m_have[have_unicastFlag] = true;
    bool have;
#define portProc(key, var)\
    if(pproc->parsePort(#key, have, m_##var)) {\
        if(have)\
            m_have[have_##var] = true;\
    } else\
        return false;
    portProc(sourcePortIdentity, srcPort)
    portProc(targetPortIdentity, dstPort)
    bool have_data = pproc->haveData();
    if(m_action == GET) {
        if(have_data) {
            PTPMGMT_ERROR("GET use dataField with zero values only, "
                "do not send dataField over JSON");
            return false;
        }
    } else if(Message::isEmpty(m_managementId)) {
        if(have_data) {
            PTPMGMT_ERROR("%s do use dataField", mngStrID);
            return false;
        }
    } else {
        if(!have_data) {
            PTPMGMT_ERROR("%s must use dataField", mngStrID);
            return false;
        }
        if(!pproc->parseData())
            return false;
        const BaseMngTlv *data = nullptr;
        if(!pproc->procData(m_managementId, data)) {
            if(data != nullptr)
                delete data;
            return false;
        }
        m_tlvData.reset(const_cast<BaseMngTlv *>(data));
    }
    return true;
}

__PTPMGMT_NAMESPACE_END
