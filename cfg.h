/* SPDX-License-Identifier: LGPL-3.0-or-later */

/** @file
 * @brief Read ptp4l Configuration file
 *
 * @author Erez Geva <ErezGeva2@gmail.com>
 * @copyright 2021 Erez Geva
 *
 * @details
 *  Classes to read ptp4l configuration file.
 *  We @b only read the values needed by Managment
 *
 */

#ifndef __CFG_H
#define __CFG_H

#include <map>
#include <string>
#include <stdint.h>

class configFile;

/**> @cond internal */
class configSection
{
  protected:
    enum {
        transportSpecific_val,
        domainNumber_val,
        udp6_scope_val,
        udp_ttl_val,
        socket_priority_val,
        network_transport_val, /* last uint8_t m_vals */
        uds_address_val,
        ptp_dst_mac_val,
        p2p_dst_mac_val,
        last_val,
    };
    static const int str_base_val; /* first string m_str_vals */
    uint8_t m_vals[network_transport_val + 1];
    bool m_set[last_val];
    std::string m_str_vals[last_val - network_transport_val];

    friend class configFile;
    void setGlobal();
    bool set_val(char *line);

  public:
    configSection();
};
/**< @endcond */

/**
 * @brief hold configuration parameters
 * @details
 *  Store provide and parse parameters
 *  from a configuration file for all sections
 */
class configFile
{
  private:
    std::map<std::string, configSection> configPerSection;
    configSection &configGlobal;

    uint8_t get(int idx, const char *section);
    uint8_t get(int idx, const std::string &section);
    const std::string &get_str(int idx, const char *section);
    const std::string &get_str(int idx, const std::string &section);
    bool is_global(int idx, const char *section);
    bool is_global(int idx, const std::string &section);

  public:
    configFile();
    /**
     * Read a configuration file and parse it
     * @param[in] file file name with path
     * @return true if parse success
     */
    bool read_cfg(const char *file);
    /**
     * Read a configuration file and parse it
     * @param[in] file file name with path
     * @return true if parse success
     */
    bool read_cfg(std::string &file) { return read_cfg(file.c_str()); }
    /**
     * Get the transportSpecific value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from "global" section
     */
    uint8_t transportSpecific(const char *section = nullptr);
    /**
     * Get the transportSpecific value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from "global" section
     */
    uint8_t transportSpecific(const std::string &section);
    /**
     * Get the domainNumber value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from "global" section
     */
    uint8_t domainNumber(const char *section = nullptr);
    /**
     * Get the domainNumber value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from "global" section
     */
    uint8_t domainNumber(const std::string &section);
    /**
     * Get the udp6_scope value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from "global" section
     */
    uint8_t udp6_scope(const char *section = nullptr);
    /**
     * Get the udp6_scope value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from "global" section
     */
    uint8_t udp6_scope(const std::string &section);
    /**
     * Get the udp_ttl value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from "global" section
     */
    uint8_t udp_ttl(const char *section = nullptr);
    /**
     * Get the udp_ttl value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from "global" section
     */
    uint8_t udp_ttl(const std::string &section);
    /**
     * Get the socket_priority value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from "global" section
     */
    uint8_t socket_priority(const char *section = nullptr);
    /**
     * Get the socket_priority value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from "global" section
     */
    uint8_t socket_priority(const std::string &section);
    /**
     * Get the network_transport value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from "global" section
     */
    uint8_t network_transport(const char *section = nullptr);
    /**
     * Get the network_transport value
     * @param[in] section (optional)
     * @return value
     * @note calling without section will fetch value from "global" section
     */
    uint8_t network_transport(const std::string &section);
    /**
     * Get the uds_address value
     * @param[in] section (optional)
     * @return string object with value
     * @note calling without section will fetch value from "global" section
     */
    const std::string &uds_address(const char *section = nullptr);
    /**
     * Get the uds_address value
     * @param[in] section (optional)
     * @return string object with value
     * @note calling without section will fetch value from "global" section
     */
    const std::string &uds_address(const std::string &section);
    /**
     * Get the uds_address value
     * @param[in] section (optional)
     * @return string value
     * @note calling without section will fetch value from "global" section
     */
    const char *uds_address_c(const char *section = nullptr);
    /**
     * Get the uds_address value
     * @param[in] section (optional)
     * @return string value
     * @note calling without section will fetch value from "global" section
     */
    const char *uds_address_c(const std::string &section);
    /**
     * Get the ptp_dst_mac value
     * @param[in] section (optional)
     * @return string object with value
     * @note calling without section will fetch value from "global" section
     */
    const std::string &ptp_dst_mac(const char *section = nullptr);
    /**
     * Get the ptp_dst_mac value
     * @param[in] section (optional)
     * @return string object with value
     * @note calling without section will fetch value from "global" section
     */
    const std::string &ptp_dst_mac(const std::string &section);
    /**
     * Get the ptp_dst_mac value
     * @param[in] section (optional)
     * @return string value
     * @note calling without section will fetch value from "global" section
     */
    const char *ptp_dst_mac_c(const char *section = nullptr);
    /**
     * Get the ptp_dst_mac value
     * @param[in] section (optional)
     * @return string value
     * @note calling without section will fetch value from "global" section
     */
    const char *ptp_dst_mac_c(const std::string &section);
    /**
     * Get the p2p_dst_mac value
     * @param[in] section (optional)
     * @return string object with value
     * @note calling without section will fetch value from "global" section
     */
    const std::string &p2p_dst_mac(const char *section = nullptr);
    /**
     * Get the p2p_dst_mac value
     * @param[in] section (optional)
     * @return string object with value
     * @note calling without section will fetch value from "global" section
     */
    const std::string &p2p_dst_mac(const std::string &section);
    /**
     * Get the p2p_dst_mac value
     * @param[in] section (optional)
     * @return string value
     * @note calling without section will fetch value from "global" section
     */
    const char *p2p_dst_mac_c(const char *section = nullptr);
    /**
     * Get the p2p_dst_mac value
     * @param[in] section (optional)
     * @return string value
     * @note calling without section will fetch value from "global" section
     */
    const char *p2p_dst_mac_c(const std::string &section);
};

#endif /*__CFG_H*/
