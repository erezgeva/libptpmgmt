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
static atomic_size_t hmacCount(0); // Count how many objects exist

// Each library have one HMAC_lib structre
// ptpm_hmac_p is the pointer to that structre
// The HMAC_lib structre provides:
//  - The static name
//  - load and unload functions
//  - Allocation of an HMAC_Key object

#ifdef PIC // Shared library code
static mutex hmacLoadLock; // Lock loading and unloading
#define LIB_LOCK unique_lock<mutex> lock(hmacLoadLock)

static HMAC_lib *ptpm_hmac_p = nullptr;
static void *hmacLib = nullptr; // handle for the loaded library
static const char *useLib = nullptr; // loaded library name
// List of available HMAP libraries passed
// from Make file based on configuration probing
static const char *list[] = { HMAC_LIBS nullptr };
// pointer of the ptpm_hmac() prototype
extern "C" { ptpm_hmac_fech_t ptpm_hmac_fech_p; }
static void doLibRm() // Unload library
{
    if(dlclose(hmacLib) != 0)
        PTPMGMT_ERROR("Fail to unload the libptpmngt HMAC library: %s",
            dlerror());
}
static void doLibNull() // Zero all library related pointers
{
    hmacLib = nullptr;
    useLib = nullptr;
    ptpm_hmac_p = nullptr;
}
static inline bool loadFuncs()
{
    // Fetch pointer to the function that return pointer
    // to the library HMAC_lib structure
    ptpm_hmac_fech_p = (ptpm_hmac_fech_t)dlsym(hmacLib, "ptpm_hmac");
    if(ptpm_hmac_fech_p != (ptpm_hmac_fech_t)nullptr) {
        ptpm_hmac_p = ptpm_hmac_fech_p();
        return ptpm_hmac_p != nullptr;
    }
    return false;
}
// Try to load library and call the library load
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
// Try to load library with name
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
    }
    LIB_LOCK;
    if(hmacLib == nullptr) {
        // Try loading
        if(!tryLib(found)) {
            doLibNull(); // Ensure all pointers stay null
            if(!Error::isError())
                PTPMGMT_ERROR("Fail loading the matched HMAC library"
                    " '%s' for pattern '%s'", found, libMatch);
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
// Try to load any library
static bool doLoadLibrary()
{
    PTPMGMT_ERROR_CLR;
    LIB_LOCK;
    if(hmacLib != nullptr)
        // A Library already loaded
        return false;
    for(const char **cur = list; *cur != nullptr; cur++) {
        if(tryLib(*cur))
            // We manage to load a proper library
            return false;
    }
    doLibNull(); // Ensure all pointers stay null
    if(!Error::isError())
        PTPMGMT_ERROR("Fail loading an HMAC library");
    return true;
}
bool hmac_selectLib(const string &libMatch)
{
    PTPMGMT_ERROR_CLR;
    return doReloadLibrary(libMatch);
}
static void freeLib()
{
    PTPMGMT_ERROR_CLR;
    LIB_LOCK;
    if(hmacCount.load() == 0 && hmacLib != nullptr) {
        ptpm_hmac_p->m_unload();
        doLibRm();
        doLibNull(); // mark all pointers null
    }
}
void hmac_freeLib() { freeLib(); }
#define HMAC_NAME useLib
#define HMAC_IS_SHARED true
#else // PIC
// To enable on link, use the '-uptpm_hmac' flag on linker
extern "C" { extern HMAC_lib *ptpm_hmac() WEAK; }
// Here ptpm_hmac_p is initilize staticly on load
static HMAC_lib *ptpm_hmac_p =
    ptpm_hmac == (ptpm_hmac_fech_t)nullptr ? nullptr : ptpm_hmac();
static bool doLoadLibrary()
{
    PTPMGMT_ERROR_CLR;
    if(ptpm_hmac_p == nullptr) {
        PTPMGMT_ERROR("Link without any HMAC library");
        return true;
    }
    return false;
}
bool hmac_selectLib(const string &libMatch)
{
    PTPMGMT_ERROR_CLR;
    return false;
}
static void freeLib()
{
    PTPMGMT_ERROR_CLR;
    if(hmacCount.load() == 0 && ptpm_hmac_p != nullptr)
        ptpm_hmac_p->m_unload();
}
void hmac_freeLib()
{
}
#define HMAC_NAME ptpm_hmac_p->m_name
#define HMAC_IS_SHARED false
#endif // PIC

const char *hmac_loadLibrary() { return doLoadLibrary() ? nullptr : HMAC_NAME; }
bool hmac_isLibShared() { return HMAC_IS_SHARED; }
size_t hmac_count() { return hmacCount.load(); }
HMAC_Key::~HMAC_Key() { hmacCount--; }
HMAC_Key *hmac_allocHMAC(HMAC_t type, const Binary &key)
{
    if(doLoadLibrary())
        return nullptr;
    HMAC_Key *hmac = ptpm_hmac_p->m_alloc_key();
    if(hmac == nullptr) {
        PTPMGMT_ERROR("allocation of HMAC_Key failed");
        return nullptr;
    }
    hmacCount++;
    hmac->m_type = type;
    hmac->m_key = key;
    if(!hmac->init()) {
        delete hmac;
        return nullptr;
    }
    return hmac;
}
// Release on exit
ON_EXIT_ATTR static void unLoadHmac() { freeLib(); }

__PTPMGMT_NAMESPACE_END
