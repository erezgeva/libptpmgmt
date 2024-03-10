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
    unique_ptr<JsonProcFrom> hold;
    JsonProcFrom *pproc = funcName(alloc_proc)();
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

#define C2CPP_ret(func, def)\
    if(j != nullptr && j->_this != nullptr)\
        return ((Json2msg*)j->_this)->func();\
    return def
#define C2CPP_cret(func, cast, def)\
    if(j != nullptr && j->_this != nullptr)\
        return (ptpmgmt_##cast)((Json2msg*)j->_this)->func();\
    return PTPMGMT_##def
#define C2CPP_ptr(func, arg)\
    if(j != nullptr && j->_this != nullptr && arg != nullptr)\
        return ((Json2msg*)j->_this)->func(arg);\
    return false
#define C2CPP_port(n)\
    if(j != nullptr && j->_this != nullptr) {\
        if(j->_##n##Port == nullptr)\
            j->_##n##Port =\
                (ptpmgmt_PortIdentity_t *)malloc(sizeof(ptpmgmt_PortIdentity_t));\
        if(j->_##n##Port != nullptr) {\
            const PortIdentity_t & p = ((Json2msg*)j->_this)->n##Port();\
            j->_##n##Port->portNumber = p.portNumber;\
            memcpy(j->_##n##Port->clockIdentity.v, p.clockIdentity.v,\
                ClockIdentity_t::size());\
            return j->_##n##Port;\
        }\
    } return nullptr
    static void ptpmgmt_json_free(ptpmgmt_json j)
    {
        if(j != nullptr) {
            if(j->_this != nullptr) {
                delete(Json2msg *)j->_this;
                j->_this = nullptr;
            }
            free(j->_srcPort);
            free(j->_dstPort);
            j->_srcPort = nullptr;
            j->_dstPort = nullptr;
            free(j->data);
            free(j->dataTbl);
            j->data = nullptr;
            j->dataTbl = nullptr;
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
    static bool ptpmgmt_json_fromJson(ptpmgmt_json j, const char *json)
    {
        C2CPP_ptr(fromJson, json);
    }
    static bool ptpmgmt_json_fromJsonObj(ptpmgmt_json j, const void *jobj)
    {
        C2CPP_ptr(fromJsonObj, jobj);
    }
    static ptpmgmt_mng_vals_e ptpmgmt_json_managementId(const_ptpmgmt_json j)
    {
        C2CPP_cret(managementId, mng_vals_e, NULL_PTP_MANAGEMENT);
    }
    static const void *ptpmgmt_json_dataField(ptpmgmt_json j)
    {
        if(j != nullptr && j->_this != nullptr) {
            Json2msg &me = *(Json2msg *)j->_this;
            const BaseMngTlv *t = me.dataField();
            if(t != nullptr) {
                void *x = nullptr;
                void *ret = cpp2cMngTlv(me.managementId(), t, x);
                if(ret != nullptr) {
                    free(j->data);
                    free(j->dataTbl);
                    j->data = ret;
                    j->dataTbl = x;
                    return ret;
                }
            }
        }
        return nullptr;
    }
    static ptpmgmt_actionField_e ptpmgmt_json_actionField(const_ptpmgmt_json j)
    {
        C2CPP_cret(actionField, actionField_e, GET);
    }
    static bool ptpmgmt_json_isUnicast(const_ptpmgmt_json j)
    {
        C2CPP_ret(isUnicast, false);
    }
    static bool ptpmgmt_json_haveIsUnicast(const_ptpmgmt_json j)
    {
        C2CPP_ret(haveIsUnicast, false);
    }
    static uint8_t ptpmgmt_json_PTPProfileSpecific(const_ptpmgmt_json j)
    {
        C2CPP_ret(PTPProfileSpecific, 0);
    }
    static bool ptpmgmt_json_havePTPProfileSpecific(const_ptpmgmt_json j)
    {
        C2CPP_ret(havePTPProfileSpecific, false);
    }
    static uint8_t ptpmgmt_json_domainNumber(const_ptpmgmt_json j)
    {
        C2CPP_ret(domainNumber, 0);
    }
    static bool ptpmgmt_json_haveDomainNumber(const_ptpmgmt_json j)
    {
        C2CPP_ret(haveDomainNumber, false);
    }
    static uint8_t ptpmgmt_json_versionPTP(const_ptpmgmt_json j)
    {
        C2CPP_ret(versionPTP, 0);
    }
    static bool ptpmgmt_json_haveVersionPTP(const_ptpmgmt_json j)
    {
        C2CPP_ret(haveVersionPTP, false);
    }
    static uint8_t ptpmgmt_json_minorVersionPTP(const_ptpmgmt_json j)
    {
        C2CPP_ret(minorVersionPTP, 0);
    }
    static bool ptpmgmt_json_haveMinorVersionPTP(const_ptpmgmt_json j)
    {
        C2CPP_ret(haveMinorVersionPTP, false);
    }
    static uint16_t ptpmgmt_json_sequenceId(const_ptpmgmt_json j)
    {
        C2CPP_ret(sequenceId, 0);
    }
    static bool ptpmgmt_json_haveSequenceId(const_ptpmgmt_json j)
    {
        C2CPP_ret(haveSequenceId, false);
    }
    static uint32_t ptpmgmt_json_sdoId(const_ptpmgmt_json j)
    {
        C2CPP_ret(sdoId, 0);
    }
    static bool ptpmgmt_json_haveSdoId(const_ptpmgmt_json j)
    {
        C2CPP_ret(haveSdoId, false);
    }
    static const ptpmgmt_PortIdentity_t *ptpmgmt_json_srcPort(ptpmgmt_json j)
    {
        C2CPP_port(src);
    }
    static bool ptpmgmt_json_haveSrcPort(const_ptpmgmt_json j)
    {
        C2CPP_ret(haveSrcPort, false);
    }
    static const ptpmgmt_PortIdentity_t *ptpmgmt_json_dstPort(ptpmgmt_json j)
    {
        C2CPP_port(dst);
    }
    static bool ptpmgmt_json_haveDstPort(const_ptpmgmt_json j)
    {
        C2CPP_ret(haveDstPort, false);
    }
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
        j->free = ptpmgmt_json_free;
        j->selectLib = ptpmgmt_json_selectLib;
        j->loadLibrary = ptpmgmt_json_loadLibrary;
        j->isLibShared = ptpmgmt_json_isLibShared;
        j->fromJson = ptpmgmt_json_fromJson;
        j->fromJsonObj = ptpmgmt_json_fromJsonObj;
        j->managementId = ptpmgmt_json_managementId;
        j->dataField = ptpmgmt_json_dataField;
        j->actionField = ptpmgmt_json_actionField;
        j->isUnicast = ptpmgmt_json_isUnicast;
        j->haveIsUnicast = ptpmgmt_json_haveIsUnicast;
        j->PTPProfileSpecific = ptpmgmt_json_PTPProfileSpecific;
        j->havePTPProfileSpecific = ptpmgmt_json_havePTPProfileSpecific;
        j->domainNumber = ptpmgmt_json_domainNumber;
        j->haveDomainNumber = ptpmgmt_json_haveDomainNumber;
        j->versionPTP = ptpmgmt_json_versionPTP;
        j->haveVersionPTP = ptpmgmt_json_haveVersionPTP;
        j->minorVersionPTP = ptpmgmt_json_minorVersionPTP;
        j->haveMinorVersionPTP = ptpmgmt_json_haveMinorVersionPTP;
        j->sequenceId = ptpmgmt_json_sequenceId;
        j->haveSequenceId = ptpmgmt_json_haveSequenceId;
        j->sdoId = ptpmgmt_json_sdoId;
        j->haveSdoId = ptpmgmt_json_haveSdoId;
        j->srcPort = ptpmgmt_json_srcPort;
        j->haveSrcPort = ptpmgmt_json_haveSrcPort;
        j->dstPort = ptpmgmt_json_dstPort;
        j->haveDstPort = ptpmgmt_json_haveDstPort;
        j->setAction = ptpmgmt_json_setAction;
        return j;
    }
}
