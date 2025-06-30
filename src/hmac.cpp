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

#ifdef PIC // Shared library code
static mutex hmacLoadLock; // Lock loading and unloading
#define LIB_LOCK unique_lock<mutex> lock(hmacLoadLock)
// Here hmacCount and ptpm_hmac_p are protected by hmacLoadLock
static HMAC_lib *ptpm_hmac_p = nullptr;
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
        return false;
    }
    for(const char **cur = list; *cur != nullptr; cur++) {
        if(tryLib(*cur)) {
            // We manage to load a proper library
            PTPMGMT_ERROR_CLR;
            return false;
        }
    }
    doLibNull(); // Ensure all pointers stay null
    PTPMGMT_ERROR("fail loading an HMAC library");
    return true;
}
static inline HMAC_Key *internAlloc()
{
    LIB_LOCK;
    if(doLoadLibrary())
        return nullptr;
    HMAC_Key *hmac = ptpm_hmac_p->m_alloc_key();
    if(hmac == nullptr) {
        PTPMGMT_ERROR("allocation of HMAC_Key failed");
        return nullptr;
    }
    hmacCount++;
    return hmac;
}
const char *hmac_loadLibrary()
{
    LIB_LOCK;
    return doLoadLibrary() ? nullptr : useLib;
}
bool hmac_selectLib(const string &libMatch)
{
    LIB_LOCK;
    return doReloadLibrary(libMatch);
}
bool hmac_isLibShared()
{
    return true;
}
void hmac_freeLib()
{
    LIB_LOCK;
    if(hmacCount == 0) {
        PTPMGMT_ERROR_CLR;
        if(hmacLib != nullptr) {
            doLibRm();
            doLibNull(); // mark all pointers null
        }
    }
}
size_t hmac_count()
{
    LIB_LOCK;
    return hmacCount;
}
HMAC_Key::~HMAC_Key()
{
    LIB_LOCK;
    hmacCount--;
}
#else // PIC
extern "C" { extern HMAC_lib *ptpm_hmac() WEAK; }
// Here ptpm_hmac_p is initilize on library loading
static HMAC_lib *ptpm_hmac_p =
    ptpm_hmac == (ptpm_hmac_fech_t)nullptr ? nullptr : ptpm_hmac();
// Here hmacCount is atomic
static atomic_size_t hmacCount_a(0); // Count how many objects exist
static bool staticLink()
{
    if(ptpm_hmac_p == nullptr) {
        PTPMGMT_ERROR("Link without any HMAC library");
        return true;
    }
    return false;
}
static inline HMAC_Key *internAlloc()
{
    if(staticLink())
        return nullptr;
    HMAC_Key *hmac = ptpm_hmac_p->m_alloc_key();
    if(hmac == nullptr) {
        PTPMGMT_ERROR("allocation of HMAC_Key failed");
        return nullptr;
    }
    hmacCount_a++;
    return hmac;
}
const char *hmac_loadLibrary()
{
    return staticLink() ? nullptr : ptpm_hmac_p->m_name;
}
bool hmac_selectLib(const string &libMatch)
{
    return false;
}
bool hmac_isLibShared()
{
    return false;
}
void hmac_freeLib()
{
}
size_t hmac_count()
{
    return hmacCount_a.load();
}
HMAC_Key::~HMAC_Key()
{
    hmacCount_a--;
}
#endif // PIC
HMAC_Key *hmac_allocHMAC(HMAC_t type, const Binary &key)
{
    HMAC_Key *hmac = internAlloc();
    if(hmac == nullptr)
        return nullptr;
    hmac->m_type = type;
    hmac->m_key = key;
    if(!hmac->init()) {
        delete hmac;
        return nullptr;
    }
    PTPMGMT_ERROR_CLR;
    return hmac;
}

__PTPMGMT_NAMESPACE_END
