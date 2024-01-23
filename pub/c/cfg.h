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

#ifndef __cplusplus
#include <stdbool.h>
#endif /* __cplusplus */
#include "name.h"

/** pointer to ptpmgmt configuration structure */
typedef struct ptpmgmt_cfg_t *ptpmgmt_cfg;

/** pointer to constant ptpmgmt configuration structure */
typedef const struct ptpmgmt_cfg_t *const_ptpmgmt_cfg;

/**
 * The ptpmgmt configuration structure hold the configuration object
 *  and call backs to call C++ methods
 */
struct ptpmgmt_cfg_t {
    /**< @cond internal */
    void *_this; /**< pointer to actual C++ configuration object */
    /**< @endcond */

    /**
     * Free a configuration object
     * @param[in] cfg configuration object to free
     */
    void (*free)(ptpmgmt_cfg cfg);
    /**
     * Read a configuration file and parse it
     * @param[in] cfg configuration object
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
     * Get the uds_address value
     * @param[in] cfg configuration object
     * @param[in] section (optional)
     * @return null terminated string witg UDS address
     * @note calling with null section will fetch value from @"global@" section
     */
    const char *(*uds_address)(const_ptpmgmt_cfg cfg, const char *section);
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

#endif /* __PTPMGMT_C_CFG_H */
