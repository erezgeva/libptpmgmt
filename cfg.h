/* SPDX-License-Identifier: LGPL-3.0-or-later */

/** @file
 * @brief Read ptp4l Configuration file
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 * @details
 *  Classes to read ptp4l configuration file.
 *  We @b only read the values needed by Management
 *
 */

#ifndef __PMC_CFG_H
#define __PMC_CFG_H

#include <map>
#include <string>
#include <cstdint>
#include "bin.h"

class ConfigFile;

/**< @cond internal */
class ConfigSection
{
  protected:
    enum {
        transportSpecific_val,
        domainNumber_val,
        udp6_scope_val,
        udp_ttl_val,
        socket_priority_val,
        network_transport_val, /* last uint8_t m_vals */
        uds_address_val, /* last string m_str_vals */
        ptp_dst_mac_val,
        p2p_dst_mac_val,
        last_val,
    };
    std::string m_str_vals[1];
    Binary m_bin_vals[last_val - uds_address_val];
    uint8_t m_vals[uds_address_val];
    bool m_set[last_val];
    static const int val_limit;    /* limit of m_vals */
    static const int str_base_val; /* first string m_str_vals */
    static const int bin_base_val; /* first binart m_bin_vals */

    friend class ConfigFile;
    void setGlobal();
    bool set_val(char *line);

  public:
    ConfigSection(); /* Must be public for map usage */
};
/**< @endcond */

/**
 * @brief hold configuration parameters
 * @details
 *  Stores provides and parses parameters
 *  from a configuration file for all sections
 */
class ConfigFile
{
  private:
    std::map<std::string, ConfigSection> cfgSec;
    ConfigSection &cfgGlobal;

    uint8_t get(int idx, const std::string &section);
    const std::string &get_str(int idx, const std::string &section);
    const Binary &get_bin(int idx, const std::string &section);
    bool is_global(int idx, const std::string &section);

  public:
    ConfigFile();
    /**
     * Read a configuration file and parse it
     * @param[in] file name with path
     * @return true if parse success
     */
    bool read_cfg(const std::string file);
    /**
     * Get the transportSpecific value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from @"global@" section
     */
    uint8_t transportSpecific(const std::string section = "");
    /**
     * Get the domainNumber value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from @"global@" section
     */
    uint8_t domainNumber(const std::string section = "");
    /**
     * Get the udp6_scope value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from @"global@" section
     */
    uint8_t udp6_scope(const std::string section = "");
    /**
     * Get the udp_ttl value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from @"global@" section
     */
    uint8_t udp_ttl(const std::string section = "");
    /**
     * Get the socket_priority value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from @"global@" section
     */
    uint8_t socket_priority(const std::string section = "");
    /**
     * Get the network_transport value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from @"global@" section
     */
    uint8_t network_transport(const std::string section = "");
    /**
     * Get the uds_address value
     * @param[in] section (optional)
     * @return string object with value
     * @note calling without section will fetch value from @"global@" section
     */
    const std::string &uds_address(const std::string section = "");
    /**
     * Get the ptp_dst_mac value
     * @param[in] section (optional)
     * @return Binary object with value
     * @note calling without section will fetch value from @"global@" section
     */
    const Binary &ptp_dst_mac(const std::string section = "");
    /**
     * Get the p2p_dst_mac value
     * @param[in] section (optional)
     * @return Binary object with value
     * @note calling without section will fetch value from @"global@" section
     */
    const Binary &p2p_dst_mac(const std::string section = "");
};

#endif /*__PMC_CFG_H*/
