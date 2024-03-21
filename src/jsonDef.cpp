/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Convert management messages to json
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 */

#include "comp.h"
#include <stack>
#include <cmath>
#include <mutex>
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif

__PTPMGMT_NAMESPACE_BEGIN

// From JSON part
static int Json2msgCount = 0; // Count how many objects exist
#ifdef PIC // Shared library code
static void *jsonLib = nullptr;
static const char *useLib = nullptr;
static mutex jsonLoadLock; // Lock loading and unloading
Json_lib *ptpm_json = nullptr;
extern "C" {
    typedef Json_lib *(*ptpm_json_fech_t)();
    ptpm_json_fech_t ptpm_json_fech_p;
}
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
    ptpm_json = nullptr;
}
static inline bool loadFuncs()
{
    ptpm_json_fech_p = (ptpm_json_fech_t)dlsym(jsonLib, "ptpm_json_fech");
    if(ptpm_json_fech_p == nullptr)
        return false;
    ptpm_json = ptpm_json_fech_p();
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
    else if(useLib != found) { // We compare pointers, not strings!
        PTPMGMT_ERROR("Already load a different library '%s', not the "
            "matched '%s' to pattern '%s'", useLib, found, libMatch);
        return false;
    }
    return true;
}
static bool doLoadLibrary(const char *libMatch = nullptr)
{
    unique_lock<mutex> lock(jsonLoadLock);
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
    unique_lock<mutex> lock(jsonLoadLock);
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
extern "C" { extern Json_lib *ptpm_json; }
#define LIB_LOAD(a)
#define LIB_FREE
#define LIB_NAME ptpm_json->m_name
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
bool Json2msg::selectLib(const string &libMatch)
{
    LIB_LOAD(libMatch.c_str());
    return LIB_SHARED;
}
bool Json2msg::isLibShared()
{
    return LIB_SHARED;
}
bool Json2msg::fromJson(const string &json)
{
    LIB_LOAD();
    void *jobj = ptpm_json->m_parse(json.c_str());
    if(jobj == nullptr) {
        PTPMGMT_ERROR("JSON parse fail");
        return false;
    }
    bool ret = fromJsonObj(jobj);
    ptpm_json->m_free(jobj);
    return ret;
}
bool Json2msg::fromJsonObj(const void *jobj)
{
    LIB_LOAD();
    unique_ptr<JsonProcFrom> hold;
    JsonProcFrom *pproc = ptpm_json->m_alloc_proc();
    if(pproc == nullptr) {
        PTPMGMT_ERROR("fromJsonObj fail allocation of JsonProcFrom");
        return false;
    }
    hold.reset(pproc); // delete the object once we return :-)
    PTPMGMT_ERROR_CLR;
    if(!pproc->mainProc(jobj))
        return false;
    const char *str = pproc->getActionField();
    if(str == nullptr || *str == 0) {
        PTPMGMT_ERROR("Message do not have an action field");
        return false;
    } else if(strcmp(str, "GET") == 0)
        m_action = GET;
    else if(strcmp(str, "SET") == 0)
        m_action = SET;
    else if(strcmp(str, "COMMAND") == 0)
        m_action = COMMAND;
    else {
        PTPMGMT_ERROR("Message have wrong action field '%s'", str);
        return false;
    }
    if(!pproc->procMng(m_managementId))
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
            PTPMGMT_ERROR("%s do use dataField",
                Message::mng2str_c(m_managementId));
            return false;
        }
    } else {
        if(!have_data) {
            PTPMGMT_ERROR("%s must use dataField",
                Message::mng2str_c(m_managementId));
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

__PTPMGMT_NAMESPACE_USE;

extern "C" {

#include "c/json.h"

#define C2CPP_ret(ret, func, def)\
    static ret ptpmgmt_json_##func(const_ptpmgmt_json j) {\
        if(j != nullptr && j->_this != nullptr)\
            return ((Json2msg*)j->_this)->func();\
        return def; }
#define C2CPP_cret(func, cast, def)\
    static ptpmgmt_##cast ptpmgmt_json_##func(const_ptpmgmt_json j) {\
        if(j != nullptr && j->_this != nullptr)\
            return (ptpmgmt_##cast)((Json2msg*)j->_this)->func();\
        return PTPMGMT_##def; }
#define C2CPP_ptr(func, typ)\
    static bool ptpmgmt_json_##func(ptpmgmt_json j, const typ *arg) {\
        if(j != nullptr && j->_this != nullptr && arg != nullptr)\
            return ((Json2msg*)j->_this)->func(arg);\
        return false; }
#define C2CPP_port(n)\
    static const ptpmgmt_PortIdentity_t *ptpmgmt_json_##n##Port(ptpmgmt_json j) {\
        if(j != nullptr && j->_this != nullptr) {\
            if(j->_##n##Port == nullptr)\
                j->_##n##Port = (ptpmgmt_PortIdentity_t *)\
                    malloc(sizeof(ptpmgmt_PortIdentity_t));\
            if(j->_##n##Port != nullptr) {\
                const PortIdentity_t & p = ((Json2msg*)j->_this)->n##Port();\
                j->_##n##Port->portNumber = p.portNumber;\
                memcpy(j->_##n##Port->clockIdentity.v, p.clockIdentity.v,\
                    ClockIdentity_t::size());\
                return j->_##n##Port;\
            }\
        } return nullptr; }
#define C_SWP(n, m) free(j->n); j->n = m
    static void ptpmgmt_json_free(ptpmgmt_json j)
    {
        if(j != nullptr) {
            if(j->_this != nullptr) {
                delete(Json2msg *)j->_this;
                j->_this = nullptr;
            }
            C_SWP(_srcPort, nullptr);
            C_SWP(_dstPort, nullptr);
            C_SWP(data, nullptr);
            C_SWP(dataTbl, nullptr);
        }
    }
    bool ptpmgmt_json_selectLib(const char *libName)
    {
        if(libName != nullptr)
            return Json2msg::selectLib(libName);
        return false;
    }
    const char *ptpmgmt_json_loadLibrary()
    {
        return Json2msg::loadLibrary();
    }
    bool ptpmgmt_json_isLibShared()
    {
        return Json2msg::isLibShared();
    }
    C2CPP_ptr(fromJson, char)
    C2CPP_ptr(fromJsonObj, void)
    C2CPP_cret(managementId, mng_vals_e, NULL_PTP_MANAGEMENT)
    static const void *ptpmgmt_json_dataField(ptpmgmt_json j)
    {
        if(j != nullptr && j->_this != nullptr) {
            Json2msg &me = *(Json2msg *)j->_this;
            const BaseMngTlv *t = me.dataField();
            if(t != nullptr) {
                void *x = nullptr;
                void *ret = cpp2cMngTlv(me.managementId(), t, x);
                if(ret != nullptr) {
                    C_SWP(data, ret);
                    C_SWP(dataTbl, x);
                    return ret;
                }
            }
        }
        return nullptr;
    }
    C2CPP_cret(actionField, actionField_e, GET)
    C2CPP_ret(bool, isUnicast, false)
    C2CPP_ret(bool, haveIsUnicast, false)
    C2CPP_ret(uint8_t, PTPProfileSpecific, 0)
    C2CPP_ret(bool, havePTPProfileSpecific, false)
    C2CPP_ret(uint8_t, domainNumber, 0)
    C2CPP_ret(bool, haveDomainNumber, false)
    C2CPP_ret(uint8_t, versionPTP, 0)
    C2CPP_ret(bool, haveVersionPTP, false)
    C2CPP_ret(uint8_t, minorVersionPTP, 0)
    C2CPP_ret(bool, haveMinorVersionPTP, false)
    C2CPP_ret(uint16_t, sequenceId, 0)
    C2CPP_ret(bool, haveSequenceId, false)
    C2CPP_ret(uint32_t, sdoId, 0)
    C2CPP_ret(bool, haveSdoId, false)
    C2CPP_port(src)
    C2CPP_ret(bool, haveSrcPort, false);
    C2CPP_port(dst);
    C2CPP_ret(bool, haveDstPort, false);
    static bool ptpmgmt_json_setAction(const_ptpmgmt_json j, ptpmgmt_msg m)
    {
        if(j != nullptr && j->_this != nullptr &&
            m != nullptr && m->_this != nullptr)
            return ((Json2msg *)j->_this)->setAction(*(Message *)m->_this);
        return false;
    }
    ptpmgmt_json ptpmgmt_json_alloc()
    {
        ptpmgmt_json j = (ptpmgmt_json)malloc(sizeof(ptpmgmt_json_t));
        if(j == nullptr)
            return nullptr;
        j->_this = (void *)(new Json2msg);
        if(j->_this == nullptr) {
            free(j);
            return nullptr;
        }
        j->_srcPort = nullptr;
        j->_dstPort = nullptr;
        j->data = nullptr;
        j->dataTbl = nullptr;
#define C_ASGN(n) j->n = ptpmgmt_json_##n
        C_ASGN(free);
        C_ASGN(selectLib);
        C_ASGN(loadLibrary);
        C_ASGN(isLibShared);
        C_ASGN(fromJson);
        C_ASGN(fromJsonObj);
        C_ASGN(managementId);
        C_ASGN(dataField);
        C_ASGN(actionField);
        C_ASGN(isUnicast);
        C_ASGN(haveIsUnicast);
        C_ASGN(PTPProfileSpecific);
        C_ASGN(havePTPProfileSpecific);
        C_ASGN(domainNumber);
        C_ASGN(haveDomainNumber);
        C_ASGN(versionPTP);
        C_ASGN(haveVersionPTP);
        C_ASGN(minorVersionPTP);
        C_ASGN(haveMinorVersionPTP);
        C_ASGN(sequenceId);
        C_ASGN(haveSequenceId);
        C_ASGN(sdoId);
        C_ASGN(haveSdoId);
        C_ASGN(srcPort);
        C_ASGN(haveSrcPort);
        C_ASGN(dstPort);
        C_ASGN(haveDstPort);
        C_ASGN(setAction);
        return j;
    }
}
