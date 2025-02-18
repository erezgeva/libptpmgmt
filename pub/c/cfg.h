/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief C interface to configuration class
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2023 Erez Geva
 *
 */

#ifndef __PTPMGMT_C_CFG_H
#define __PTPMGMT_C_CFG_H

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif
#include "name.h"

/** pointer to ptpmgmt configuration structure */
typedef struct ptpmgmt_cfg_t *ptpmgmt_cfg;

/** pointer to constant ptpmgmt configuration structure */
typedef const struct ptpmgmt_cfg_t *const_ptpmgmt_cfg;

/** pointer to ptpmgmt SPP structure */
typedef struct ptpmgmt_spp_t *ptpmgmt_spp;

/** pointer to constant ptpmgmt SPP structure */
typedef const struct ptpmgmt_spp_t *const_ptpmgmt_spp;

/** pointer to ptpmgmt SA file structure */
typedef struct ptpmgmt_safile_t *ptpmgmt_safile;

/** pointer to constant ptpmgmt SA file structure */
typedef const struct ptpmgmt_safile_t *const_ptpmgmt_safile;

/**
 * The ptpmgmt configuration structure hold the configuration object
 *  and call backs to call C++ methods
 */
struct ptpmgmt_cfg_t {
    /**< @cond internal */
    void *_this; /**< pointer to actual C++ configuration object */
    /**< @endcond */

    /**
     * Free this configuration object
     * @param[in, out] cfg configuration object to free
     */
    void (*free)(ptpmgmt_cfg cfg);
    /**
     * Read a configuration file and parse it
     * @param[in, out] cfg configuration object
     * @param[in] file name with path
     * @return true if parse success
     */
    bool (*read_cfg)(ptpmgmt_cfg cfg, const char *file);
    /**
     * Get the transportSpecific value
     * @param[in] cfg configuration object
     * @param[in] section (optional)
     * @return value
     * @note calling with null section will fetch value from @"global@" section
     */
    uint8_t (*transportSpecific)(const_ptpmgmt_cfg cfg, const char *section);
    /**
     * Get the domainNumber value
     * @param[in] cfg configuration object
     * @param[in] section (optional)
     * @return value
     * @note calling with null section will fetch value from @"global@" section
     */
    uint8_t (*domainNumber)(const_ptpmgmt_cfg cfg, const char *section);
    /**
     * Get the udp6_scope value
     * @param[in] cfg configuration object
     * @param[in] section (optional)
     * @return value
     * @note calling with null section will fetch value from @"global@" section
     */
    uint8_t (*udp6_scope)(const_ptpmgmt_cfg cfg, const char *section);
    /**
     * Get the udp_ttl value
     * @param[in] cfg configuration object
     * @param[in] section (optional)
     * @return value
     * @note calling with null section will fetch value from @"global@" section
     */
    uint8_t (*udp_ttl)(const_ptpmgmt_cfg cfg, const char *section);
    /**
     * Get the socket_priority value
     * @param[in] cfg configuration object
     * @param[in] section (optional)
     * @return value
     * @note calling with null section will fetch value from @"global@" section
     */
    uint8_t (*socket_priority)(const_ptpmgmt_cfg cfg, const char *section);
    /**
     * Get the network_transport value
     * @param[in] cfg configuration object
     * @param[in] section (optional)
     * @return value
     * @note calling with null section will fetch value from @"global@" section
     */
    uint8_t (*network_transport)(const_ptpmgmt_cfg cfg, const char *section);
    /**
     * Get the authentication key ID value
     * @param[in] cfg configuration object
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from @"global@" section
     */
    uint16_t (*active_key_id)(const_ptpmgmt_cfg cfg, const char *section);
    /**
     * Get the authentication security parameter pointer value
     * @param[in] cfg configuration object
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from @"global@" section
     */
    uint8_t (*spp)(const_ptpmgmt_cfg cfg, const char *section);
    /**
     * Get accept unauthenticated response messages value
     * @param[in] cfg configuration object
     * @param[in] section (optional)
     * @return the allow unauthenticated messages value
     * @note calling without section will fetch value from @"global@" section
     */
    uint8_t (*allow_unauth)(const_ptpmgmt_cfg cfg, const char *section);
    /**
     * Query whether the authentication security parameter pointer is set
     * @param[in] cfg configuration object
     * @param[in] section (optional)
     * @return true if value is set
     * @note calling without section will Query the @"global@" section
     */
    bool (*haveSpp)(const_ptpmgmt_cfg cfg, const char *section);
    /**
     * Get the uds_address value
     * @param[in] cfg configuration object
     * @param[in] section (optional)
     * @return null terminated string witg UDS address
     * @note calling with null section will fetch value from @"global@" section
     */
    const char *(*uds_address)(const_ptpmgmt_cfg cfg, const char *section);
    /**
     * Get the authentication security association file value
     * @param[in] cfg configuration object
     * @param[in] section (optional)
     * @return null terminated string witg UDS address
     * @note calling with null section will fetch value from @"global@" section
     */
    const char *(*sa_file)(const_ptpmgmt_cfg cfg, const char *section);
    /**
     * Get the ptp_dst_mac value
     * @param[in] cfg configuration object
     * @param[out] len Mac address length
     * @param[in] section (optional)
     * @return pointer to binary mac address or null
     * @note calling with null section will fetch value from @"global@" section
     */
    const void *(*ptp_dst_mac)(const_ptpmgmt_cfg cfg, size_t *len,
        const char *section);
    /**
     * Get the p2p_dst_mac value
     * @param[in] cfg configuration object
     * @param[out] len Mac address length
     * @param[in] section (optional)
     * @return pointer to binary mac address or null
     * @note calling with null section will fetch value from @"global@" section
     */
    const void *(*p2p_dst_mac)(const_ptpmgmt_cfg cfg, size_t *len,
        const char *section);
};

/**
 * Allocate new configuration structure
 * @return new configuration structure or null on error
 */
ptpmgmt_cfg ptpmgmt_cfg_alloc();

/**
 * @brief Authentication key type
 * @details
 *  For Hashed Message Authentication Mode (HMAC) digests.
 */
enum PTPMGMT_HMAC_t {
    PTPMGMT_HMAC_SHA256, /**< Using Secure Hash Algorithm 2 with 256 bits */
    /**
     * Using NIST Advanced Encryption Standard with 128 bits key
     */
    PTPMGMT_HMAC_AES128,
    /**
     * Using NIST Advanced Encryption Standard with 256 bits key
     */
    PTPMGMT_HMAC_AES256
};

/**
 * The ptpmgmt spp structure hold a authentication security parameters object
 *  and call backs to call C++ methods
 */
struct ptpmgmt_spp_t {
    /**< @cond internal */
    void *_this; /**< pointer to actual C++ SPP object */
    /**< @endcond */

    /**
     * Free a authentication security parameters object
     * @param[in, out] spp authentication security parameters object
     */
    void (*free)(ptpmgmt_spp spp);
    /**
     * Set a new key
     * @param[in, out] spp authentication security parameters object
     * @param[in] id id the Key
     * @param[in] type of the key
     * @param[in] value of the key
     * @param[in] size of the key
     * @param[in] digest size to use with the key
     * @param[in] replace an existing key if exist
     * @return true, if new key is valid
     */
    bool (*addKey)(ptpmgmt_spp spp, uint32_t id, enum PTPMGMT_HMAC_t type,
        const void *value, size_t size, size_t digest, bool replace);
    /**
     * Verify key exist
     * @param[in] spp authentication security parameters object
     * @param[in] key ID
     * @return true if key exist
     */
    bool (*have)(const_ptpmgmt_spp spp, uint32_t key);
    /**
     * Get MAC digest size to use with the key
     * @param[in] spp authentication security parameters object
     * @param[in] key ID
     * @return MAC digest size or 0 if the key does not exist
     */
    size_t (*mac_size)(const_ptpmgmt_spp spp, uint32_t key);
    /**
     * Get key
     * @param[in] spp authentication security parameters object
     * @param[in] key ID
     * @return Key or nullptr if key does not exist
     */
    const void *(*key)(const_ptpmgmt_spp spp, uint32_t key);
    /**
     * Get key size
     * @param[in] spp authentication security parameters object
     * @param[in] key ID
     * @return Key size or zero if key does not exist
     */
    size_t (*key_size)(const_ptpmgmt_spp spp, uint32_t key);
    /**
     * Get number of keys
     * @param[in] spp authentication security parameters object
     * @param[in] key ID
     * @return Key or empty binary if key does not exist
     */
    size_t (*keys)(const_ptpmgmt_spp spp);
    /**
     * Get key type
     * @param[in] spp authentication security parameters object
     * @param[in] key ID
     * @return Key type
     */
    enum PTPMGMT_HMAC_t (*htype)(const_ptpmgmt_spp spp, uint32_t key);
    /**
     * Return own SPP ID
     * @param[in] spp authentication security parameters object
     * @return own SPP ID
     */
    uint8_t (*ownID)(const_ptpmgmt_spp spp);
};

/**
 * Allocate new spp structure
 * @return new spp structure or null on error
 */
ptpmgmt_spp ptpmgmt_spp_alloc(uint8_t id);
/**
 * Copy spp structure to a new spp
 * @return new spp structure or null on error
 * @note This function can be used to a const spp object
 */
ptpmgmt_spp ptpmgmt_spp_alloc_cp(const_ptpmgmt_spp spp);

/**
 * The ptpmgmt SA file structure hold an authentication security
 *  association file object and call backs to call C++ methods
 */
struct ptpmgmt_safile_t {
    /**< @cond internal */
    void *_this; /**< pointer to actual C++ SA file object */
    ptpmgmt_spp _all[UINT8_MAX]; /**< pointer to spp structure objects */
    /**< @endcond */

    /**
     * Free an an authentication security association file object
     * @param[in, out] sf authentication security association file object
     */
    void (*free)(ptpmgmt_safile sf);
    /**
     * Read a SA file and parse it
     * @param[in, out] sf authentication security association file object
     * @param[in] file name with path
     * @return true if parse success
     */
    bool (*read_sa)(ptpmgmt_safile sf, const char *file);
    /**
     * Read a SA file and parse it.
     * Get file name from the configuration file.
     * @param[in, out] sf authentication security association file object
     * @param[in] cfg reference to configuration file object
     * @param[in] section (optional)
     * @return true if parse success, false if fail
     *         or file does not exist
     *         or configuration file have an empty sa_file.
     */
    bool (*read_sa_cfg)(ptpmgmt_safile sf, const_ptpmgmt_cfg cfg,
        const char *section);
    /**
     * Verify SPP is in the SA_file and have at least one key
     * @param[in] sf authentication security association file object
     * @param[in] spp ID
     * @return true if SPP exist with at least one key
     */
    bool (*have)(const_ptpmgmt_safile sf, uint8_t spp);
    /**
     * Verify key exist in an SPP
     * @param[in] sf authentication security association file object
     * @param[in] spp ID
     * @param[in] key ID
     * @return true if key exist
     */
    bool (*have_key)(const_ptpmgmt_safile sf, uint8_t spp, uint32_t key);
    /**
     * Get SPP by spp ID
     * @param[in, out] sf authentication security association file object
     * @param[in] spp ID
     * @return SPP if exist or empty SPP
     */
    const_ptpmgmt_spp(*spp)(ptpmgmt_safile sf, uint8_t spp);
};

/**
 * Allocate new SA file structure
 * @return new SA file structure or null on error
 */
ptpmgmt_safile ptpmgmt_safile_alloc();

#ifdef __cplusplus
}
#endif

#endif /* __PTPMGMT_C_CFG_H */
