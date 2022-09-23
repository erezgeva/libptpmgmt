/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

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

#ifndef __PTPMGMT_CFG_H
#define __PTPMGMT_CFG_H

#include <map>
#include "bin.h"

__PTPMGMT_NAMESPACE_BEGIN

class ConfigFile;

#ifndef SWIG
/**
 * @cond internal
 * Internal class
 */
class ConfigSection
{
  protected:
    enum {
        val_base_val,
        transportSpecific_val = val_base_val,
        domainNumber_val,
        udp6_scope_val,
        udp_ttl_val,
        socket_priority_val,
        network_transport_val,
        str_base_val,
        uds_address_val = str_base_val,
        bin_base_val,
        ptp_dst_mac_val = bin_base_val,
        p2p_dst_mac_val,
        last_val,
    };
    /* new values must be add to ranges[] */
    struct range_t {
        const char *name;
        const char *defStr;
        uint8_t defVal;
        uint8_t min;
        uint8_t max;
    };
    /* ranges and default value */
    static const range_t ranges[];
    /* String values */
    std::string m_str_vals[bin_base_val - str_base_val];
    /* Binaries values */
    Binary m_bin_vals[last_val - bin_base_val];
    /* integer values in the range 0-255 */
    uint8_t m_vals[str_base_val - val_base_val];
    /* Determine if a value is set in the configuration file.
     * Relevant for non global sections. */
    bool m_set[last_val];

    friend class ConfigFile;
    void setGlobal();
    bool set_val(char *line);

  public:
    ConfigSection() : m_set{0} {} /* Must be public for map usage */
};
/**< @endcond */
#endif /* SWIG */

/**
 * @brief Hold configuration parameters
 * @details
 *  Stores provides and parses parameters
 *  from a configuration file for all sections
 */
class ConfigFile
{
  private:
    std::map<std::string, ConfigSection> cfgSec;
    ConfigSection *cfgGlobal; /* Not the owner, just a shortcut */
    void clear_sections();

    uint8_t get_num(int idx, const std::string &section) const;
    const std::string &get_str(int idx, const std::string &section) const;
    const Binary &get_bin(int idx, const std::string &section) const;
    bool is_global(int idx, const std::string &section) const;

  public:
    ConfigFile() { clear_sections(); }
    /**
     * Read a configuration file and parse it
     * @param[in] file name with path
     * @return true if parse success
     */
    bool read_cfg(const std::string &file);
    /**
     * Get the transportSpecific value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from @"global@" section
     */
    uint8_t transportSpecific(const std::string &section = "") const;
    /**
     * Get the domainNumber value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from @"global@" section
     */
    uint8_t domainNumber(const std::string &section = "") const;
    /**
     * Get the udp6_scope value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from @"global@" section
     */
    uint8_t udp6_scope(const std::string &section = "") const;
    /**
     * Get the udp_ttl value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from @"global@" section
     */
    uint8_t udp_ttl(const std::string &section = "") const;
    /**
     * Get the socket_priority value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from @"global@" section
     */
    uint8_t socket_priority(const std::string &section = "") const;
    /**
     * Get the network_transport value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from @"global@" section
     */
    uint8_t network_transport(const std::string &section = "") const;
    /**
     * Get the uds_address value
     * @param[in] section (optional)
     * @return string object with value
     * @note calling without section will fetch value from @"global@" section
     */
    const std::string &uds_address(const std::string &section = "") const;
    /**
     * Get the ptp_dst_mac value
     * @param[in] section (optional)
     * @return Binary object with value
     * @note calling without section will fetch value from @"global@" section
     */
    const Binary &ptp_dst_mac(const std::string &section = "") const;
    /**
     * Get the p2p_dst_mac value
     * @param[in] section (optional)
     * @return Binary object with value
     * @note calling without section will fetch value from @"global@" section
     */
    const Binary &p2p_dst_mac(const std::string &section = "") const;
};

__PTPMGMT_NAMESPACE_END

#endif /* __PTPMGMT_CFG_H */
