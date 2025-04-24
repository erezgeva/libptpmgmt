/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief HMAC libraries locader and caller
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 *
 */

#include "comp.h"
#include <mutex>
#ifdef HAVE_DLFCN_H
#include <dlfcn.h>
#endif
#include <atomic>

__PTPMGMT_NAMESPACE_BEGIN

extern "C" { typedef HMAC_lib *(*ptpm_hmac_fech_t)(); }
HMAC_lib *ptpm_hmac_p = nullptr;
static mutex hmacLoadLock; // Lock loading and unloading
#ifdef PIC // Shared library code
// Here hmacCount is protected by hmacLoadLock
static size_t hmacCount = 0; // Count how many objects exist
static void *hmacLib = nullptr;
static const char *useLib = nullptr;
// List of available HMAP libraries passed
// from Make file based on configuration probing
static const char *list[] = { HMAC_LIBS nullptr };
extern "C" { ptpm_hmac_fech_t ptpm_hmac_fech_p; }
static void doLibRm()
{
    if(dlclose(hmacLib) != 0)
        PTPMGMT_ERROR("Fail to unload the libptpmngt HMAC library: %s",
            dlerror());
}
static void doLibNull()
{
    hmacLib = nullptr;
    useLib = nullptr;
    ptpm_hmac_p = nullptr;
}
static inline bool loadFuncs()
{
    ptpm_hmac_fech_p = (ptpm_hmac_fech_t)dlsym(hmacLib, "ptpm_hmac");
    if(ptpm_hmac_fech_p == (ptpm_hmac_fech_t)nullptr)
        return false;
    ptpm_hmac_p = ptpm_hmac_fech_p();
    return ptpm_hmac_p != nullptr;
}
static bool tryLib(const char *name)
{
    hmacLib = dlopen(name, RTLD_LAZY);
    if(hmacLib != nullptr) {
        if(loadFuncs()) {
            useLib = name;
            return true;
        }
        doLibRm();
    }
    return false;
}
static inline bool doReloadLibrary(const string &libMatchCpp)
{
    if(libMatchCpp.empty()) {
        PTPMGMT_ERROR("Empty match pattern");
        return false;
    }
    const char *libMatch = libMatchCpp.c_str();
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
    if(found == nullptr) {
        PTPMGMT_ERROR("Fail to find any library to matche pattern '%s'", libMatch);
        return false;
    } else if(hmacLib == nullptr) {
        // Try loading
        if(!tryLib(found)) {
            doLibNull(); // Ensure all pointers stay null
            PTPMGMT_ERROR("Fail loading the matched HMAC library '%s' for "
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
    PTPMGMT_ERROR_CLR;
    return true;
}
static bool doLoadLibrary()
{
    if(hmacLib != nullptr) {
        // A Library already loaded
        PTPMGMT_ERROR_CLR;
        return true;
    }
    for(const char **cur = list; *cur != nullptr; cur++) {
        if(tryLib(*cur)) {
            // We manage to load a proper library
            PTPMGMT_ERROR_CLR;
            return true;
        }
    }
    doLibNull(); // Ensure all pointers stay null
    PTPMGMT_ERROR("fail loading an HMAC library");
    return false;
}
static inline void libFree()
{
    if(hmacCount == 0) {
        PTPMGMT_ERROR_CLR;
        if(hmacLib != nullptr) {
            doLibRm();
            doLibNull(); // mark all pointers null
        }
    }
}
#define LIB_LOCK unique_lock<mutex> lock(hmacLoadLock)
#define LIB_LOAD if(hmacLib == nullptr && !doLoadLibrary()) return nullptr
#define LIB_RELOAD(match) if(!doReloadLibrary(match)) return false
#define LIB_FREE libFree()
#define LIB_NAME useLib
#define LIB_SHARED true
#define LIB_HMAC_COUNT_VAL hmacCount
#else // PIC
// Here hmacCount is not protected by other means, so we use atomic
static atomic_size_t hmacCount(0); // Count how many objects exist
extern "C" { extern HMAC_lib *ptpm_hmac() WEAK; }
static bool staticLink()
{
    if(ptpm_hmac_p == nullptr) {
        PTPMGMT_ERROR_CLR;
        unique_lock<mutex> lock(hmacLoadLock);
        // Just in case other thread allocate, while we were locked
        if(ptpm_hmac_p != nullptr)
            return false;
        if(ptpm_hmac == (ptpm_hmac_fech_t)nullptr) {
            PTPMGMT_ERROR("Link without any HMAC library");
            return true;
        }
        ptpm_hmac_p = ptpm_hmac();
        return ptpm_hmac_p == nullptr;
    }
    return false;
}
#define LIB_LOCK
#define LIB_LOAD if(staticLink()) return nullptr
#define LIB_RELOAD(match)
#define LIB_FREE
#define LIB_NAME ptpm_hmac_p->m_name
#define LIB_SHARED false
#define LIB_HMAC_COUNT_VAL hmacCount.load()
#endif // PIC

const char *hmac_loadLibrary()
{
    LIB_LOCK;
    LIB_LOAD;
    return LIB_NAME;
}
bool hmac_selectLib(const string &libMatch)
{
    LIB_LOCK;
    LIB_RELOAD(libMatch);
    return LIB_SHARED;
}
bool hmac_isLibShared()
{
    return LIB_SHARED;
}
void hmac_freeLib()
{
    LIB_LOCK;
    LIB_FREE;
}
size_t hmac_count()
{
    LIB_LOCK;
    return LIB_HMAC_COUNT_VAL;
}
HMAC_Key *hmac_allocHMAC(HMAC_t type, const Binary &key)
{
    HMAC_Key *hmac;
    {
        LIB_LOCK;
        LIB_LOAD;
        hmac = ptpm_hmac_p->m_alloc_key();
        if(hmac == nullptr) {
            PTPMGMT_ERROR("allocation of HMAC_Key failed");
            return nullptr;
        }
        hmacCount++;
    }
    hmac->m_type = type;
    hmac->m_key = key;
    if(!hmac->init()) {
        delete hmac;
        return nullptr;
    }
    PTPMGMT_ERROR_CLR;
    return hmac;
}
HMAC_Key::~HMAC_Key()
{
    LIB_LOCK;
    hmacCount--;
}

__PTPMGMT_NAMESPACE_END
