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

const size_t lineSize = 512;

#define get_func(n)\
    uint8_t ConfigFile::n(const string &section) const\
    { return get_num(ConfigSection::n##_val, section); }
#define get_str_func(n)\
    const string &ConfigFile:: n(const string &section) const\
    { return get_str(ConfigSection::n##_val, section); }
#define get_bin_func(n)\
    const Binary &ConfigFile:: n(const string &section) const\
    { return get_bin(ConfigSection::n##_val, section); }

// Holds all values from enumerator
#define rang_val(n, dVal, minVal, maxVal)\
    [ConfigSection::n##_val] = {#n, nullptr, dVal, minVal, maxVal}
#define rang_str(n, dStr)\
    [ConfigSection::n##_val] = {#n, dStr}
const ConfigSection::range_t ConfigSection::ranges[] = {
    rang_val(transportSpecific, 0, 0, 0xf),
    // IEEE 1588-2019 permit upto 239 based on sdoId value
    //  (transportSpecific == majorSdoId)
    rang_val(domainNumber, 0, 0, UINT8_MAX),
    rang_val(udp6_scope, 0xe, 0, 0xf),
    rang_val(udp_ttl, 1, 1, UINT8_MAX),
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
static inline char *next_space_token(char *cur)
{
    while(!isspace(*cur))
        cur++;
    *cur = 0;
    cur++;
    return skip_spaces(cur); // skip spaces at start of value
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
    char *val = next_space_token(line); // find value
    int idx = val_base_val - 1;
    for(size_t i = val_base_val; i < last_val; i++) {
        if(strcmp(line, ranges[i].name) == 0) {
            idx = i;
            break;
        }
    }
    if(idx < val_base_val)
        return true;
    strip_end_spaces(val + strlen(val));
    if(*val == 0) // empty value after chomp
        return false;
    switch(idx) {
        // String values
        case uds_address_val:
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
        // integer values
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
bool ConfigFile::read_cfg(const string &_file)
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
    char buf[lineSize];
    clear_sections(); // remove old configuration
    string curSection = globalSection;
    size_t lineNum = 0;
    while(fgets(buf, lineSize, f) != nullptr) {
        char *cur = skip_spaces(buf);
        lineNum++;
        if(*cur == '[') {
            cur = skip_spaces(cur + 1);
            char *end = strchr(cur, ']');
            if(end == nullptr) {
                PTPMGMT_ERROR("wrong line %s(%d)", file, lineNum);
                return false;
            }
            strip_end_spaces(end);
            curSection = cur;
        } else if(*cur != 0 && *cur != '#' &&
            !cfgSec[curSection].set_val(cur)) {
            PTPMGMT_ERROR("wrong line %s(%d)", file, lineNum);
            return false;
        }
    }
    fclose(f);
    PTPMGMT_ERROR_CLR;
    return true;
}
bool ConfigFile::is_global(int idx, const string &section) const
{
    if(section.empty() || cfgSec.count(section) == 0 ||
        !cfgSec.at(section).m_set[idx])
        return true;
    return false;
}
uint8_t ConfigFile::get_num(int idx, const string &section) const
{
    if(is_global(idx, section))
        return cfgGlobal->m_vals[idx - ConfigSection::val_base_val];
    return cfgSec.at(section).m_vals[idx - ConfigSection::val_base_val];
}
const string &ConfigFile::get_str(int idx, const string &section) const
{
    if(is_global(idx, section))
        return cfgGlobal->m_str_vals[idx - ConfigSection::str_base_val];
    return cfgSec.at(section).m_str_vals[idx - ConfigSection::str_base_val];
}
const Binary &ConfigFile::get_bin(int idx, const string &section) const
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

static const char ptpm_empty_str[] = "";
__PTPMGMT_NAMESPACE_USE;

extern "C" {

#include "c/cfg.h"

    // C interfaces
    static void ptpmgmt_cfg_free(ptpmgmt_cfg me)
    {
        if(me != nullptr) {
            if(me->_this != nullptr)
                delete(ConfigFile *)me->_this;
            free(me);
        }
    }
    static void ptpmgmt_cfg_free_wrap(ptpmgmt_cfg me)
    {
    }
    static bool ptpmgmt_cfg_read_cfg(ptpmgmt_cfg me, const char *file)
    {
        if(me != nullptr && me->_this != nullptr && file != nullptr)
            return ((ConfigFile *)me->_this)->read_cfg(file);
        return false;
    }
#define C2CPP_funcN(n, func)\
    static uint##n##_t ptpmgmt_cfg_##func(const_ptpmgmt_cfg me,\
        const char *section) {\
        if(me != nullptr && me->_this != nullptr) {\
            if (section == nullptr)\
                return (( ConfigFile*)me->_this)->func();\
            return (( ConfigFile*)me->_this)->func(section);\
        } return 0; }
#define C2CPP_func(func) C2CPP_funcN(8, func)
    C2CPP_func(transportSpecific)
    C2CPP_func(domainNumber)
    C2CPP_func(udp6_scope)
    C2CPP_func(udp_ttl)
    C2CPP_func(socket_priority)
    C2CPP_func(network_transport)
#define C2CPP_str(func)\
    static const char *ptpmgmt_cfg_##func(const_ptpmgmt_cfg me,\
        const char *section) {\
        if(me != nullptr && me->_this != nullptr) {\
            const char *a = section != nullptr ? section : ptpm_empty_str;\
            const string &s = ((ConfigFile *)me->_this)->func(a);\
            if(!s.empty()) return s.c_str();\
        }   return nullptr; }
    C2CPP_str(uds_address)
#define C2CPP_bfunc(func)\
    static const void *ptpmgmt_cfg_##func(const_ptpmgmt_cfg me, size_t *len,\
        const char *section) {\
        if(me != nullptr && me->_this != nullptr) {\
            const char *a = section != nullptr ? section : ptpm_empty_str;\
            const Binary &b = ((ConfigFile*)me->_this)->func(a);\
            if(b.length() > 0) {\
                if (len != nullptr)\
                    *len = b.length();\
                return b.get();\
            }\
        }\
        if (len != nullptr)\
            *len = 0;\
        return nullptr; }
    C2CPP_bfunc(ptp_dst_mac)
    C2CPP_bfunc(p2p_dst_mac)
    static inline void ptpmgmt_cfg_asign_cb(ptpmgmt_cfg me)
    {
#define C_ASGN(n) me->n = ptpmgmt_cfg_##n
        C_ASGN(read_cfg);
        C_ASGN(transportSpecific);
        C_ASGN(domainNumber);
        C_ASGN(udp6_scope);
        C_ASGN(udp_ttl);
        C_ASGN(socket_priority);
        C_ASGN(network_transport);
        C_ASGN(uds_address);
        C_ASGN(ptp_dst_mac);
        C_ASGN(p2p_dst_mac);
    }
    ptpmgmt_cfg ptpmgmt_cfg_alloc()
    {
        ptpmgmt_cfg me = (ptpmgmt_cfg)malloc(sizeof(ptpmgmt_cfg_t));
        if(me == nullptr)
            return nullptr;
        me->_this = (void *)(new ConfigFile);
        if(me->_this == nullptr) {
            free(me);
            return nullptr;
        }
        me->free = ptpmgmt_cfg_free;
        ptpmgmt_cfg_asign_cb(me);
        return me;
    }
    ptpmgmt_cfg ptpmgmt_cfg_alloc_wrap(void *cfg)
    {
        if(cfg == nullptr)
            return nullptr;
        ptpmgmt_cfg me = (ptpmgmt_cfg)malloc(sizeof(ptpmgmt_cfg_t));
        if(me == nullptr)
            return nullptr;
        me->_this = cfg;
        me->free = ptpmgmt_cfg_free_wrap;
        ptpmgmt_cfg_asign_cb(me);
        return me;
    }
}
