/* SPDX-License-Identifier: LGPL-3.0-or-later */

/** @file
 * @brief Read ptp4l Configuration file
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 * @copyright GNU Lesser General Public License 3.0 or later
 *
 */

#include <cstring>
#include "err.h"
#include "cfg.h"
#include "ptp.h"

#define get_func(n)\
    uint8_t ConfigFile::n(const std::string section)\
    {\
        return get(ConfigSection::n##_val, section);\
    }
#define get_str_func(n)\
    const std::string &ConfigFile:: n(const std::string section)\
    {\
        return get_str(ConfigSection::n##_val, section);\
    }
#define get_bin_func(n)\
    const Binary &ConfigFile:: n(const std::string section)\
    {\
        return get_bin(ConfigSection::n##_val, section);\
    }

struct range_t {
    const char *name;
    uint8_t def;
    uint8_t min;
    uint8_t max;
};
const range_t ranges[] = { // same order as enum !
    { "transportSpecific", 0, 0, 0xf },
    // IEEE 1588-2019 permit upto 239 based on sdoId value
    //  (transportSpecific == majorSdoId)
    { "domainNumber", 0, 0, 239 },
    { "udp6_scope", 0xe, 0, 0xf },
    { "udp_ttl", 1, 1, 255 },
    { "socket_priority", 0, 0, 15 },
    { "network_transport", '4', '2', '6' },
    { "uds_address", 0, 0, 0 }, // string
    { "ptp_dst_mac", 0, 0, 0 }, // MAC address
    { "p2p_dst_mac", 0, 0, 0 }, // MAC address
};
const char *ptp_dst_mac_def = "1:1b:19:0:0:0";
const char *p2p_dst_mac_def = "1:80:c2:0:0:e";
const char *uds_address_def = "/var/run/ptp4l";
const char globalSection[] = "global";
const int ConfigSection::val_limit = ConfigSection::uds_address_val;
const int ConfigSection::str_base_val = ConfigSection::uds_address_val;
const int ConfigSection::bin_base_val = ConfigSection::uds_address_val + 1;

static inline char *skip_spaces(char *cur)
{
    while(isspace(*cur))
        cur++;
    return cur;
}
static inline void strip_end_spaces(char *end)
{
    while(isspace(*--end))
        ;
    *(end + 1) = 0;
}

ConfigSection::ConfigSection() : m_set{0}
{
}
void ConfigSection::setGlobal()
{
    // Use default values from linuxptp
    m_str_vals[uds_address_val - str_base_val] = uds_address_def;
    m_bin_vals[ptp_dst_mac_val - bin_base_val].fromMac(ptp_dst_mac_def);
    m_bin_vals[p2p_dst_mac_val - bin_base_val].fromMac(p2p_dst_mac_def);
    for(int i = 0; i < val_limit; i++)
        m_vals[i] = ranges[i].def;
}
bool ConfigSection::set_val(char *line)
{
    char *val = line;
    while(!isspace(*val))
        val++; // find value
    *val = 0;  // leave key in line
    int idx = -1;
    for(size_t i = 0; i < (sizeof(ranges) / sizeof(range_t)); i++) {
        if(strcmp(line, ranges[i].name) == 0) {
            idx = i;
            break;
        }
    }
    if(idx < 0)
        return true;
    val++;
    val = skip_spaces(val); // skip spaces at start of value
    strip_end_spaces(val + strlen(val));
    if(*val == 0) // empty value after chomp
        return false;
    char *endptr;
    Binary id;
    long ret;
    switch(idx) {
        case uds_address_val:
            if(*val != '/' || strlen(val) < 2)
                return false;
            m_str_vals[idx - str_base_val] = val;
            break;
        case ptp_dst_mac_val:
        case p2p_dst_mac_val:
            if(!id.fromMac(val))
                return false;
            m_bin_vals[idx - bin_base_val] = id;
            break;
        case network_transport_val:
            if(strcmp(val, "UDPv4") == 0)
                m_vals[idx] = '4';
            else if(strcmp(val, "UDPv6") == 0)
                m_vals[idx] = '6';
            else if(strcmp(val, "L2") == 0)
                m_vals[idx] = '2';
            else
                return false;
            break;
        default:
            ret = strtol(val, &endptr, 0);
            if(ret >= ranges[idx].min && ret <= ranges[idx].max)
                m_vals[idx] = ret;
            else
                return false;
            break;
    }
    m_set[idx] = true;
    return true;
}
ConfigFile::ConfigFile() : cfgGlobal(cfgSec[globalSection])
{
    cfgGlobal.setGlobal(); /* For default */
}
// read PTP configuration from file
bool ConfigFile::read_cfg(const std::string _file)
{
    const char *file = _file.c_str();
    FILE *f = fopen(file, "r");
    if(f == nullptr) {
        PMC_ERRORA("fail to open %s: %m", file);
        return false;
    }
    char buf[512];
    // remove old configuration
    cfgSec.clear();
    cfgGlobal = cfgSec[globalSection];
    cfgGlobal.setGlobal();
    std::string curSection = globalSection;
    while(fgets(buf, sizeof(buf), f) != nullptr) {
        char *cur = skip_spaces(buf);
        if(*cur == '[') {
            cur = skip_spaces(cur + 1);
            char *end = strchr(cur, ']');
            if(end == nullptr) {
                PMC_ERRORA("wrong line in %s: '%s'", file, buf);
                return false;
            }
            strip_end_spaces(end);
            curSection = cur;
        } else if(*cur != 0 && *cur != '#' &&
            !cfgSec[curSection].set_val(cur)) {
            PMC_ERRORA("wrong line in %s: '%s'", file, buf);
            return false;
        }
    }
    fclose(f);
    return true;
}
bool ConfigFile::is_global(int idx, const std::string &section)
{
    if(section.empty() || cfgSec.count(section) == 0 || !cfgSec[section].m_set[idx])
        return true;
    return false;
}
uint8_t ConfigFile::get(int idx, const std::string &section)
{
    if(is_global(idx, section))
        return cfgGlobal.m_vals[idx];
    return cfgSec[section].m_vals[idx];
}
const std::string &ConfigFile::get_str(int idx, const std::string &section)
{
    if(is_global(idx, section))
        return cfgGlobal.m_str_vals[idx - ConfigSection::str_base_val];
    return cfgSec[section].m_str_vals[idx - ConfigSection::str_base_val];
}
const Binary &ConfigFile::get_bin(int idx, const std::string &section)
{
    if(is_global(idx, section))
        return cfgGlobal.m_bin_vals[idx - ConfigSection::bin_base_val];
    return cfgSec[section].m_bin_vals[idx - ConfigSection::bin_base_val];
}
get_func(transportSpecific)
get_func(domainNumber)
get_func(udp6_scope)
get_func(udp_ttl)
get_func(socket_priority)
get_func(network_transport)
get_str_func(uds_address)
get_bin_func(ptp_dst_mac)
get_bin_func(p2p_dst_mac)
