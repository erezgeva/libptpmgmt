/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* cfg.h Read ptp4l Configuration file
 *
 * Authors: Erez Geva <ErezGeva2@gmail.com>
 *
 */

#ifndef __CFG_H
#define __CFG_H

#include <map>
#include <string>
#include <stdint.h>

/*
 * Classes to read ptp4l configuration file
 * We only read the 3 values needed by Managment
 */

class configFile;

class configSection
{
  protected:
    enum {
        transportSpecific_val,
        domainNumber_val,
        udp6_scope_val,
        udp_ttl_val,
        socket_priority_val,
        network_transport_val,
        // last uint8_t val
        // first string value
        uds_address_val,
        ptp_dst_mac_val,
    };
    uint8_t m_vals[network_transport_val + 1];
    bool m_set[ptp_dst_mac_val + 1];
    std::string m_str_vals[2];

    friend class configFile;
    void setGlobal();
    bool set_val(char *line);

  public:
    configSection();
};

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
    bool read_cfg(const char *file);
    bool read_cfg(std::string &file) { return read_cfg(file.c_str()); }
    uint8_t transportSpecific(const char *section = nullptr);
    uint8_t transportSpecific(const std::string &section);
    uint8_t domainNumber(const char *section = nullptr);
    uint8_t domainNumber(const std::string &section);
    uint8_t udp6_scope(const char *section = nullptr);
    uint8_t udp6_scope(const std::string &section);
    uint8_t udp_ttl(const char *section = nullptr);
    uint8_t udp_ttl(const std::string &section);
    uint8_t socket_priority(const char *section = nullptr);
    uint8_t socket_priority(const std::string &section);
    uint8_t network_transport(const char *section = nullptr);
    uint8_t network_transport(const std::string &section);
    const std::string &uds_address(const char *section = nullptr);
    const std::string &uds_address(const std::string &section);
    const char *uds_address_c(const char *section = nullptr);
    const char *uds_address_c(const std::string &section);
    const std::string &ptp_dst_mac(const char *section = nullptr);
    const std::string &ptp_dst_mac(const std::string &section);
    const char *ptp_dst_mac_c(const char *section = nullptr);
    const char *ptp_dst_mac_c(const std::string &section);
};

#endif /*__CFG_H*/
