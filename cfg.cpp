/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* cfg.cpp Read ptp4l Configuration file
 *
 * Authors: Erez Geva <ErezGeva2@gmail.com>
 *
 */

#include <cstring>
#include "cfg.h"
#include "ptp.h"

#define get_func(n)\
    uint8_t configFile::n(const char *section)\
    {\
        return get(configSection::n##_val, section);\
    }\
    uint8_t configFile::n(const std::string &section)\
    {\
        return get(configSection::n##_val, section);\
    }
#define get_str_func(n)\
    const std::string &configFile:: n(const char *section)\
    {\
        return get_str(configSection::n##_val, section);\
    }\
    const std::string &configFile:: n(const std::string &section)\
    {\
        return get_str(configSection::n##_val, section);\
    }\
    const char *configFile::n##_c(const char *section)\
    {\
        return n(section).c_str();\
    }\
    const char *configFile::n##_c(const std::string &section)\
    {\
        return n(section).c_str();\
    }

struct range_t {
    const char *name;
    uint8_t def;
    uint8_t min;
    uint8_t max;
};
const range_t ranges[] = { // same order as enum !
    { "transportSpecific", 0, 0, 0xf },
    { "domainNumber", 0, 0, 127 },
    { "udp6_scope", 0xe, 0, 0xf },
    { "udp_ttl", 1, 1, 255 },
    { "socket_priority", 0, 0, 15 },
    { "network_transport", '4', '2', '6' },
    { "uds_address", 0, 0, 0 }, // string
    { "ptp_dst_mac", 0, 0, 0 }, // MAC address
};
const int ranges_size = sizeof(ranges) / sizeof(range_t);
const char *ptp_dst_mac_def = "1:1b:19:0:0:0";
const char *uds_address_def = "/var/run/ptp4l";

const char globalSection[] = "global";

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

configSection::configSection() : m_set{0}
{
}
void configSection::setGlobal()
{
    // Use default values from linuxptp
    m_str_vals[0] = uds_address_def;
    m_str_vals[1] = ptpIf::str2mac(ptp_dst_mac_def);
    for(int i = 0; i < ranges_size; i++)
        m_vals[i] = ranges[i].def;
}
bool configSection::set_val(char *line)
{
    char *val = line;
    while(!isspace(*val))
        val++; // find value
    *val = 0;  // leave key in line
    int idx = -1;
    for(int i = 0; i < ranges_size; i++) {
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
    switch(idx) {
        case uds_address_val:
            if(*val != '/' || strlen(val) < 2)
                return false;
            m_str_vals[0] = val;
            break;
        case ptp_dst_mac_val: {
            auto ret = ptpIf::str2mac(val);
            if(ret.empty())
                return false;
            m_str_vals[1] = ret;
            break;
        }
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
        default: {
            char *endptr;
            auto ret = strtol(val, &endptr, 0);
            if(ret >= ranges[idx].min && ret <= ranges[idx].max)
                m_vals[idx] = ret;
            else
                return false;
            break;
        }
    }
    m_set[idx] = true;
    return true;
}
configFile::configFile() : configGlobal(configPerSection[globalSection])
{
    configGlobal.setGlobal();
}
// read PTP configuration from file
bool configFile::read_cfg(const char *file)
{
    FILE *f = fopen(file, "r");
    if(f == nullptr) {
        fprintf(stderr, "fail to open %s: %m\n", file);
        return false;
    }
    char buf[512];
    // remove old configuration
    configPerSection.clear();
    configGlobal = configPerSection[globalSection];
    configGlobal.setGlobal();
    std::string curSection = globalSection;
    while(fgets(buf, sizeof(buf), f) != nullptr) {
        char *cur = skip_spaces(buf);
        if(*cur == '[') {
            cur = skip_spaces(cur + 1);
            char *end = strchr(cur, ']');
            if(end == nullptr) {
                fprintf(stderr, "wrong line in %s: '%s'\n", file, buf);
                return false;
            }
            strip_end_spaces(end);
            curSection = cur;
        } else if(*cur != 0 && *cur != '#' &&
            !configPerSection[curSection].set_val(cur)) {
            fprintf(stderr, "wrong line in %s: '%s'\n", file, buf);
            return false;
        }
    }
    fclose(f);
    return true;
}
bool configFile::is_global(int idx, const char *section)
{
    if(section == nullptr || !configPerSection[section].m_set[idx])
        return true;
    return false;
}
bool configFile::is_global(int idx, const std::string &section)
{
    if(section.empty() || !configPerSection[section].m_set[idx])
        return true;
    return false;
}
uint8_t configFile::get(int idx, const char *section)
{
    if(is_global(idx, section))
        return configGlobal.m_vals[idx];
    return configPerSection[section].m_vals[idx];
}
uint8_t configFile::get(int idx, const std::string &section)
{
    if(is_global(idx, section))
        return configGlobal.m_vals[idx];
    return configPerSection[section].m_vals[idx];
}
const std::string &configFile::get_str(int idx, const char *section)
{
    if(is_global(idx, section))
        return configGlobal.m_str_vals[idx - configSection::uds_address_val];
    return configPerSection[section].m_str_vals[idx -
            configSection::uds_address_val];
}
const std::string &configFile::get_str(int idx, const std::string &section)
{
    if(is_global(idx, section))
        return configGlobal.m_str_vals[idx - configSection::uds_address_val];
    return configPerSection[section].m_str_vals[idx -
            configSection::uds_address_val];
}
get_func(transportSpecific)
get_func(domainNumber)
get_func(udp6_scope)
get_func(udp_ttl)
get_func(socket_priority)
get_func(network_transport)
get_str_func(uds_address)
get_str_func(ptp_dst_mac)
