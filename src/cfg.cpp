/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Read ptp4l Configuration file
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 */

#include "cfg.h"
#include "comp.h"

__PTPMGMT_NAMESPACE_BEGIN

#define get_func(n)\
    uint8_t ConfigFile::n(const std::string &section) const\
    {\
        return get_num(ConfigSection::n##_val, section);\
    }
#define get_str_func(n)\
    const std::string &ConfigFile:: n(const std::string &section) const\
    {\
        return get_str(ConfigSection::n##_val, section);\
    }
#define get_bin_func(n)\
    const Binary &ConfigFile:: n(const std::string &section) const\
    {\
        return get_bin(ConfigSection::n##_val, section);\
    }

// Holds all values from enumerator
#define rang_val(n, dVal, minVal, maxVal)\
    [ConfigSection::n##_val] = {#n, nullptr, dVal, minVal, maxVal}
#define rang_str(n, dStr)\
    [ConfigSection::n##_val] = {#n, dStr}
const ConfigSection::range_t ConfigSection::ranges[] = {
    rang_val(transportSpecific, 0, 0, 0xf),
    // IEEE 1588-2019 permit upto 239 based on sdoId value
    //  (transportSpecific == majorSdoId)
    rang_val(domainNumber, 0, 0, 239),
    rang_val(udp6_scope, 0xe, 0, 0xf),
    rang_val(udp_ttl, 1, 1, 255),
    rang_val(socket_priority, 0, 0, 15),
    rang_val(network_transport, '4', '2', '6'),
    rang_str(uds_address, "/var/run/ptp4l"),
    rang_str(ptp_dst_mac, "1:1b:19:0:0:0"),
    rang_str(p2p_dst_mac, "1:80:c2:0:0:e"),
};
static const char globalSection[] = "global";

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

void ConfigSection::setGlobal()
{
    int i;
    // Use default values from linuxptp
    for(i = str_base_val; i < bin_base_val; i++)
        m_str_vals[i - str_base_val] = ranges[i].defStr;
    for(i = bin_base_val; i < last_val; i++)
        m_bin_vals[i - bin_base_val].fromMac(ranges[i].defStr);
    for(i = val_base_val; i < str_base_val; i++)
        m_vals[i - val_base_val] = ranges[i].defVal;
}
bool ConfigSection::set_val(char *line)
{
    char *val = line;
    while(!isspace(*val))
        val++; // find value
    *val = 0;  // leave key in line
    int idx = val_base_val - 1;
    for(size_t i = val_base_val; i < last_val; i++) {
        if(strcmp(line, ranges[i].name) == 0) {
            idx = i;
            break;
        }
    }
    if(idx < val_base_val)
        return true;
    val++;
    val = skip_spaces(val); // skip spaces at start of value
    strip_end_spaces(val + strlen(val));
    if(*val == 0) // empty value after chomp
        return false;
    switch(idx) {
        // String values
        case uds_address_val:
            if(*val != '/' || strlen(val) < 2)
                return false;
            m_str_vals[idx - str_base_val] = val;
            break;
        // MAC address values
        case ptp_dst_mac_val:
            FALLTHROUGH;
        case p2p_dst_mac_val: {
            Binary id;
            if(!id.fromMac(val))
                return false;
            m_bin_vals[idx - bin_base_val] = id;
            break;
        }
        // Network transport type
        case network_transport_val:
            if(strcmp(val, "UDPv4") == 0)
                m_vals[idx - val_base_val] = '4';
            else if(strcmp(val, "UDPv6") == 0)
                m_vals[idx - val_base_val] = '6';
            else if(strcmp(val, "L2") == 0)
                m_vals[idx - val_base_val] = '2';
            else
                return false;
            break;
        // integer values in the range 0-255
        default: {
            char *endptr;
            long ret = strtol(val, &endptr, 0);
            if(*endptr != 0 || ret < ranges[idx].min || ret > ranges[idx].max)
                return false;
            m_vals[idx - val_base_val] = ret;
            break;
        }
    }
    m_set[idx] = true;
    return true;
}
void ConfigFile::clear_sections()
{
    cfgSec.clear();
    cfgGlobal = &cfgSec[globalSection];
    cfgGlobal->setGlobal(); // default values
}
// read PTP configuration from file
bool ConfigFile::read_cfg(const std::string &_file)
{
    if(_file.empty()) {
        PTPMGMT_ERROR("Empty file name");
        return false;
    }
    const char *file = _file.c_str();
    FILE *f = fopen(file, "r");
    if(f == nullptr) {
        PTPMGMT_ERROR("fail to open %s: %m", file);
        return false;
    }
    char buf[512];
    clear_sections(); // remove old configuration
    std::string curSection = globalSection;
    while(fgets(buf, sizeof buf, f) != nullptr) {
        char *cur = skip_spaces(buf);
        if(*cur == '[') {
            cur = skip_spaces(cur + 1);
            char *end = strchr(cur, ']');
            if(end == nullptr) {
                PTPMGMT_ERROR("wrong line in %s: '%s'", file, buf);
                return false;
            }
            strip_end_spaces(end);
            curSection = cur;
        } else if(*cur != 0 && *cur != '#' &&
            !cfgSec[curSection].set_val(cur)) {
            PTPMGMT_ERROR("wrong line in %s: '%s'", file, buf);
            return false;
        }
    }
    fclose(f);
    PTPMGMT_ERROR_CLR;
    return true;
}
bool ConfigFile::is_global(int idx, const std::string &section) const
{
    if(section.empty() || cfgSec.count(section) == 0 ||
        !cfgSec.at(section).m_set[idx])
        return true;
    return false;
}
uint8_t ConfigFile::get_num(int idx, const std::string &section) const
{
    if(is_global(idx, section))
        return cfgGlobal->m_vals[idx - ConfigSection::val_base_val];
    return cfgSec.at(section).m_vals[idx - ConfigSection::val_base_val];
}
const std::string &ConfigFile::get_str(int idx,
    const std::string &section) const
{
    if(is_global(idx, section))
        return cfgGlobal->m_str_vals[idx - ConfigSection::str_base_val];
    return cfgSec.at(section).m_str_vals[idx - ConfigSection::str_base_val];
}
const Binary &ConfigFile::get_bin(int idx, const std::string &section) const
{
    if(is_global(idx, section))
        return cfgGlobal->m_bin_vals[idx - ConfigSection::bin_base_val];
    return cfgSec.at(section).m_bin_vals[idx - ConfigSection::bin_base_val];
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

__PTPMGMT_NAMESPACE_END
