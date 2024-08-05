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
#include "c/cfg.h"
#include "comp.h"

__PTPMGMT_NAMESPACE_BEGIN

const size_t lineSize = 512;

#define get_func(n)\
    uint8_t ConfigFile::n(const string &section) const\
    { return (uint8_t)get_num(ConfigSection::n##_val, section); }
#define get_func32(n)\
    uint32_t ConfigFile::n(const string &section) const\
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
    rang_val(active_key_id, 0, 0, UINT32_MAX),
    rang_val(spp, 0, 0, UINT8_MAX),
    rang_val(allow_unauth, 0, 0, 2),
    rang_str(uds_address, "/var/run/ptp4l"),
    rang_str(sa_file, ""),
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
            FALLTHROUGH;
        case sa_file_val:
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
            if UNLIKELY_COND(idx >= str_base_val)
                return false; // Should not happen
            char *endptr;
            long ret = strtol(val, &endptr, 0);
            if(ret < ranges[idx].min || ret > ranges[idx].max || *endptr != 0)
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
            if(end == nullptr)
                goto lineErrWrLn1;
            strip_end_spaces(end);
            curSection = cur;
        } else if(*cur != 0 && *cur != '#' &&
            !cfgSec[curSection].set_val(cur))
            goto lineErrWrLn1;
    }
    fclose(f);
    PTPMGMT_ERROR_CLR;
    return true;
lineErrWrLn1:
    fclose(f);
    PTPMGMT_ERROR("wrong line %s(%zu)", file, lineNum);
    return false;
}
bool ConfigFile::is_global(int idx, const string &section) const
{
    if(section.empty() || cfgSec.count(section) == 0 ||
        !cfgSec.at(section).m_set[idx])
        return true;
    return false;
}
uint32_t ConfigFile::get_num(int idx, const string &section) const
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
get_func32(active_key_id)
get_func(spp)
get_func(allow_unauth)
bool ConfigFile::haveSpp(const std::string &section) const
{
    if(cfgGlobal->m_set[ConfigSection::spp_val])
        return true;
    if(section.empty())
        return false;
    return cfgSec.at(section).m_set[ConfigSection::spp_val];
}
get_str_func(uds_address)
get_str_func(sa_file)
get_bin_func(ptp_dst_mac)
get_bin_func(p2p_dst_mac)

static const Binary empty_key;
static const Spp empty_spp;
struct key_cfg_t {
    const char *name;
    HMAC_t type;
    size_t digest;
};
static key_cfg_t key_cfg[] = {
    {"SHA256-128", HMAC_SHA256, 16},
    {"SHA256",     HMAC_SHA256, 32},
    {"AES128",     HMAC_AES128, 16},
    {"AES256",     HMAC_AES256, 16},
};

bool Spp::set_val(char *line)
{
    char *cur = next_space_token(line); // First value
    if(isdigit(*line)) { // Only key ID start with a digit
        // Look for a key ID
        char *endptr;
        long long ret = strtoll(line, &endptr, 0);
        if(*endptr != 0 || ret < 1 || ret > UINT32_MAX)
            return false;
        uint32_t keyID = (uint32_t)ret;
        // Second value is the key type
        Token tkn(spaceToken, cur);
        cur = tkn.first();
        if(cur == nullptr)
            return false;
        key_cfg_t *match = nullptr;
        for(key_cfg_t &k : key_cfg) {
            if(strcmp(k.name, cur) == 0) {
                match = &k;
                break;
            }
        }
        if(match == nullptr)
            return false;
        // Third value is optional key size or key value
        cur = tkn.next();
        if(cur == nullptr)
            return false;
        // Fouth value is key value if the third is key size
        char *cur2 = tkn.next();
        // There should not be any fifth value
        if(tkn.next() != nullptr)
            return false;
        size_t keySize;
        if(cur2 != nullptr) {
            long long ret = strtoll(cur, &endptr, 0);
            if(*endptr != 0 || ret < 1)
                return false;
            keySize = (size_t)ret;
            cur = cur2;
        } else
            keySize = 0;
        Binary key;
        if(strncmp(cur, "ASCII:", 6) == 0)
            key.setBin(cur + 6);
        else if(strncmp(cur, "HEX:", 4) == 0) {
            if(!key.fromHex(cur + 4)) {
                PTPMGMT_ERROR("wrong key value %s", cur);
                return false;
            }
        } else if(strncmp(cur, "B64:", 4) == 0) {
            if(!key.fromBase64(cur + 4)) {
                PTPMGMT_ERROR("wrong key value %s", cur);
                return false;
            }
        } else
            key.setBin(cur);
        if(keySize > 0 && key.size() != keySize) {
            PTPMGMT_ERROR("wrong key size mismatch: value size is %zu not %zu",
                key.size(), keySize);
            return false;
        }
        return addKey(keyID, match->type, key, match->digest, false);
    } else {
        /**
         * = Not relevant:
         * allow_mutable: is not used as Managment 'correctionField' is always zero
         * seqid_window: is not relevant to Managment or Signaling messages
         * = Future linuxptp use:
         * seqnum_length: sequenceNo (Sequence number) size
         * res_length:    Reserved size
         * = Not planed:
         * disclosedKey Disclosed key
         */
    }
    return true;
}
struct key_limit_t {
    size_t keySize;
    size_t maxDigest;
};
static key_limit_t key_limit[] = {
    [HMAC_SHA256] = { 0, 32},
    [HMAC_AES128] = {16, 16},
    [HMAC_AES256] = {32, 16}
};
bool Spp::addKey(uint32_t id, HMAC_t type, Binary &value, size_t digest,
    bool replace)
{
    // 0 reserved for disabled
    if(id == 0) {
        PTPMGMT_ERROR("Wrong key id");
        return false;
    }
    if(value.empty()) {
        PTPMGMT_ERROR("Empty key");
        return false;
    }
    // digest value need to be positive and even
    if(digest < 2 || (digest & 1) != 0) {
        PTPMGMT_ERROR("Wrong digest %zu", digest);
        return false;
    }
    if(have(id) && !replace) {
        PTPMGMT_ERROR("key %u exist", id);
        return false;
    }
    switch(type) {
        case HMAC_SHA256:
            FALLTHROUGH;
        case HMAC_AES128:
            FALLTHROUGH;
        case HMAC_AES256:
            break;
        default:
            PTPMGMT_ERROR("Wrong key type %d", type);
            return false;
    }
    if(key_limit[type].keySize > 0 && key_limit[type].keySize != value.size()) {
        PTPMGMT_ERROR("Wrong key size %zu", value.size());
        return false;
    }
    if(digest > key_limit[type].maxDigest) {
        PTPMGMT_ERROR("digest is too big %zu", digest);
        return false;
    }
    m_keys[id] = {type, value, digest};
    PTPMGMT_ERROR_CLR;
    return true;
}
size_t Spp::mac_size(uint32_t id) const
{
    if(have(id))
        return m_keys.at(id).mac_size;
    return 0;
}
const Binary &Spp::key(uint32_t id) const
{
    if(have(id))
        return m_keys.at(id).key;
    return empty_key;
}
HMAC_t Spp::htype(uint32_t id) const
{
    if(have(id))
        return m_keys.at(id).type;
    return HMAC_SHA256; // Does not really matter
}
bool SaFile::read_sa(const std::string &_file)
{
    if(_file.empty()) {
        PTPMGMT_ERROR("Empty file name");
        return false;
    }
    const char *file = _file.c_str();
    if(file == nullptr) {
        PTPMGMT_ERROR("fail to duplicate string %s", _file.c_str());
        return false;
    }
    FILE *f = fopen(file, "r");
    if(f == nullptr) {
        PTPMGMT_ERROR("fail to open %s: %m", file);
        return false;
    }
    char buf[lineSize];
    Spp cspp;
    std::map<uint8_t, Spp> spps;
    uint8_t step = 0;
    size_t lineNum = 0;
    while(fgets(buf, lineSize, f) != nullptr) {
        char *cur = skip_spaces(buf);
        lineNum++;
        if(*cur == '[') {
            cur = skip_spaces(cur + 1);
            char *end = strchr(cur, ']');
            if(end == nullptr)
                goto lineErrWrLn2;
            strip_end_spaces(end);
            if(step == 2) {
                if(cspp.keys() < 1) {
                    PTPMGMT_ERROR("try to add spp %d without keys in %s(%zu)",
                        cspp.ownID(), file, lineNum);
                    goto lineErr2;
                }
                spps[cspp.ownID()] = cspp;
            }
            step = (strcmp(cur, "security_association") == 0) ? 1 : 0;
        } else if(*cur != 0 && *cur != '#') {
            char *val;
            switch(step) {
                case 1:
                    val = next_space_token(cur);
                    if(strcmp(cur, "spp") == 0) {
                        strip_end_spaces(val + strlen(val));
                        char *endptr;
                        long ret = strtol(val, &endptr, 0);
                        if(*endptr != 0 || ret < 0 || ret > UINT8_MAX) {
                            PTPMGMT_ERROR("wrong spp value %ld in %s(%zu)",
                                ret, file, lineNum);
                            goto lineErr2;
                        }
                        step = 2;
                        cspp.set(ret);
                    } else
                        goto lineErrWrLn2;
                    break;
                case 2:
                    if(cspp.set_val(cur))
                        break;
                    goto lineErrWrLn2;
                default:
                    goto lineErrWrLn2;
            }
        }
    }
    fclose(f);
    if(step == 2) {
        if(cspp.keys() < 1) {
            PTPMGMT_ERROR("Try to add SPP %d without keys in %s(%zu)",
                cspp.ownID(), file, lineNum);
            return false;
        }
        spps[cspp.ownID()] = cspp;
    }
    m_spps.clear(); // remove old configuration
    m_spps = std::move(spps);
    PTPMGMT_ERROR_CLR;
    return true;
lineErrWrLn2:
    PTPMGMT_ERROR("wrong line %s(%zu)", file, lineNum);
lineErr2:
    fclose(f);
    return false;
}
bool SaFile::read_sa(const ConfigFile &cfg, const std::string &section)
{
    const std::string &file = cfg.sa_file(section);
    if(file.empty()) {
        PTPMGMT_ERROR("No sa_file in configuration file (%s)", section.c_str());
        return false;
    }
    return read_sa(file);
}
const Spp &SaFile::spp(uint8_t sppID) const
{
    if(have(sppID))
        return m_spps.at(sppID);
    return empty_spp;
}

__PTPMGMT_NAMESPACE_END

static const char ptpm_empty_str[] = "";
__PTPMGMT_NAMESPACE_USE;

extern "C" {
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
    C2CPP_funcN(16, active_key_id)
    C2CPP_func(spp)
    C2CPP_func(allow_unauth)
    bool ptpmgmt_cfg_haveSpp(const_ptpmgmt_cfg me, const char *section)
    {
        if(me != nullptr && me->_this != nullptr) {
            if(section == nullptr)
                return ((ConfigFile *)me->_this)->haveSpp();
            return ((ConfigFile *)me->_this)->haveSpp(section);
        }
        return false;
    }
#define C2CPP_str(func)\
    static const char *ptpmgmt_cfg_##func(const_ptpmgmt_cfg me,\
        const char *section) {\
        if(me != nullptr && me->_this != nullptr) {\
            const char *a = section != nullptr ? section : ptpm_empty_str;\
            const string &s = ((ConfigFile *)me->_this)->func(a);\
            if(!s.empty()) return s.c_str();\
        }   return nullptr; }
    C2CPP_str(uds_address)
    C2CPP_str(sa_file)
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
        C_ASGN(active_key_id);
        C_ASGN(spp);
        C_ASGN(allow_unauth);
        C_ASGN(haveSpp);
        C_ASGN(uds_address);
        C_ASGN(sa_file);
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
    ptpmgmt_cfg ptpmgmt_cfg_alloc_wrap(const ConfigFile &cfg)
    {
        ptpmgmt_cfg me = (ptpmgmt_cfg)malloc(sizeof(ptpmgmt_cfg_t));
        if(me == nullptr)
            return nullptr;
        me->_this = (void *)&cfg;
        me->free = ptpmgmt_cfg_free_wrap;
        ptpmgmt_cfg_asign_cb(me);
        return me;
    }
    static void ptpmgmt_spp_free(ptpmgmt_spp me)
    {
        if(me != nullptr) {
            if(me->_this != nullptr)
                delete(Spp *)me->_this;
            free(me);
        }
    }
    static void ptpmgmt_spp_free_wrap(ptpmgmt_spp me)
    {
    }
    bool ptpmgmt_spp_addKey(ptpmgmt_spp spp, uint32_t id, PTPMGMT_HMAC_t type,
        const void *value, size_t size, size_t digest, bool replace)
    {
        if(spp != nullptr && spp->_this != nullptr && value != nullptr &&
            size > 0 && digest > 1) {
            Binary k(value, size);
            return ((Spp *)spp->_this)->addKey(id, (HMAC_t)type, k,
                    digest, replace);
        }
        return false;
    }
#define C2CPP_key(ret, nm, def)\
    ret ptpmgmt_spp_##nm(const_ptpmgmt_spp spp, uint32_t key)\
    {\
        if(spp != nullptr && spp->_this != nullptr)\
            return (ret)(((const Spp *)spp->_this)->nm(key));\
        return def;\
    }
    C2CPP_key(bool, have, false)
    C2CPP_key(size_t, mac_size, 0)
    C2CPP_key(PTPMGMT_HMAC_t, htype, PTPMGMT_HMAC_SHA256)
#define C2CPP_key_fn(ret, nm, fn, def)\
    ret ptpmgmt_spp_##nm(const_ptpmgmt_spp spp, uint32_t key)\
    {\
        if(spp != nullptr && spp->_this != nullptr)\
            return ((const Spp *)spp->_this)->key(key).fn();\
        return def;\
    }
    C2CPP_key_fn(const void *, key, get, nullptr)
    C2CPP_key_fn(size_t, key_size, size, 0)
#define C2CPP_num(ret, nm)\
    ret ptpmgmt_spp_##nm(const_ptpmgmt_spp spp)\
    {\
        if(spp != nullptr && spp->_this != nullptr)\
            return ((const Spp *)spp->_this)->nm();\
        return 0;\
    }
    C2CPP_num(size_t, keys)
    C2CPP_num(uint8_t, ownID)
    static inline void ptpmgmt_spp_asign_cb(ptpmgmt_spp me)
    {
#undef C_ASGN
#define C_ASGN(n) me->n = ptpmgmt_spp_##n
        C_ASGN(addKey);
        C_ASGN(have);
        C_ASGN(mac_size);
        C_ASGN(key);
        C_ASGN(key_size);
        C_ASGN(keys);
        C_ASGN(htype);
        C_ASGN(ownID);
    }
    ptpmgmt_spp ptpmgmt_spp_alloc(uint8_t id)
    {
        ptpmgmt_spp me = (ptpmgmt_spp)malloc(sizeof(ptpmgmt_spp_t));
        if(me == nullptr)
            return nullptr;
        me->_this = (void *)(new Spp(id));
        if(me->_this == nullptr) {
            free(me);
            return nullptr;
        }
        me->free = ptpmgmt_spp_free;
        ptpmgmt_spp_asign_cb(me);
        return me;
    }
    ptpmgmt_spp ptpmgmt_spp_alloc_cp(const_ptpmgmt_spp spp)
    {
        if(spp == nullptr || spp->_this == nullptr)
            return nullptr;
        ptpmgmt_spp me = (ptpmgmt_spp)malloc(sizeof(ptpmgmt_spp_t));
        if(me == nullptr)
            return nullptr;
        me->_this = (void *)(new Spp(*(const Spp *)spp->_this));
        if(me->_this == nullptr) {
            free(me);
            return nullptr;
        }
        me->free = ptpmgmt_spp_free;
        ptpmgmt_spp_asign_cb(me);
        return me;
    }
    ptpmgmt_spp ptpmgmt_spp_alloc_wrap(const Spp &spp)
    {
        ptpmgmt_spp me = (ptpmgmt_spp)malloc(sizeof(ptpmgmt_spp_t));
        if(me == nullptr)
            return nullptr;
        me->_this = (void *)&spp;
        me->free = ptpmgmt_spp_free_wrap;
        ptpmgmt_spp_asign_cb(me);
        return me;
    }
    static void ptpmgmt_safile_free(ptpmgmt_safile me)
    {
        if(me != nullptr) {
            if(me->_this != nullptr)
                delete(SaFile *)me->_this;
            for(size_t i = 0; i < UINT8_MAX; i++)
                free(me->_all[i]);
            free(me);
        }
    }
    static void ptpmgmt_safile_free_wrap(ptpmgmt_safile me)
    {
        if(me != nullptr) {
            for(size_t i = 0; i < UINT8_MAX; i++)
                free(me->_all[i]);
        }
    }
    bool ptpmgmt_safile_read_sa(ptpmgmt_safile sf, const char *file)
    {
        if(sf != nullptr && sf->_this != nullptr && file != nullptr)
            return ((SaFile *)sf->_this)->read_sa(file);
        return false;
    }
    bool ptpmgmt_safile_read_sa_cfg(ptpmgmt_safile sf, const_ptpmgmt_cfg cfg,
        const char *section)
    {
        if(sf != nullptr && sf->_this != nullptr && cfg != nullptr) {
            SaFile &s = *(SaFile *)sf->_this;
            ConfigFile &c = *(ConfigFile *)cfg->_this;
            if(section == nullptr)
                return s.read_sa(c);
            return s.read_sa(c, section);
        }
        return false;
    }
    bool ptpmgmt_safile_have(const_ptpmgmt_safile sf, uint8_t spp)
    {
        if(sf != nullptr && sf->_this != nullptr)
            return ((SaFile *)sf->_this)->have(spp);
        return false;
    }
    bool ptpmgmt_safile_have_key(const_ptpmgmt_safile sf, uint8_t spp, uint32_t key)
    {
        if(sf != nullptr && sf->_this != nullptr)
            return ((SaFile *)sf->_this)->have(spp, key);
        return false;
    }
    const_ptpmgmt_spp ptpmgmt_safile_spp(ptpmgmt_safile sf, uint8_t sppID)
    {
        if(sf != nullptr && sf->_this != nullptr) {
            SaFile &s = *(SaFile *)sf->_this;
            if(s.have(sppID)) {
                if(sf->_all[sppID] == nullptr)
                    sf->_all[sppID] = ptpmgmt_spp_alloc_wrap(s.spp(sppID));
                return sf->_all[sppID];
            }
        }
        return nullptr;
    }
    static inline void ptpmgmt_safile_asign_cb(ptpmgmt_safile me)
    {
        memset(me->_all, 0, sizeof me->_all);
#undef C_ASGN
#define C_ASGN(n) me->n = ptpmgmt_safile_##n
        C_ASGN(read_sa);
        C_ASGN(read_sa_cfg);
        C_ASGN(have);
        C_ASGN(have_key);
        C_ASGN(spp);
    }
    ptpmgmt_safile ptpmgmt_safile_alloc()
    {
        ptpmgmt_safile me = (ptpmgmt_safile)malloc(sizeof(ptpmgmt_safile_t));
        if(me == nullptr)
            return nullptr;
        me->_this = (void *)(new SaFile());
        if(me->_this == nullptr) {
            free(me);
            return nullptr;
        }
        me->free = ptpmgmt_safile_free;
        ptpmgmt_safile_asign_cb(me);
        return me;
    }
    ptpmgmt_safile ptpmgmt_safile_alloc_wrap(const SaFile &sa)
    {
        ptpmgmt_safile me = (ptpmgmt_safile)malloc(sizeof(ptpmgmt_safile_t));
        if(me == nullptr)
            return nullptr;
        me->_this = (void *)&sa;
        me->free = ptpmgmt_safile_free_wrap;
        ptpmgmt_safile_asign_cb(me);
        return me;
    }
}
