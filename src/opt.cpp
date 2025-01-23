/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Handle command line options
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 */

#include <regex>
#include "opt.h"
#include "c/opt.h"
#include "ver.h"
#include "comp.h"

__PTPMGMT_NAMESPACE_BEGIN

#define _tf true, false  // with argument
#define _ff false, false // without argument
Pmc_option Options::startOptions[] = {
    { 'b', "", _tf, "boundary hops", "num", "1" },
    { 'd', "domainNumber", _tf, "domain number", "num", "0" },
    { 'f', "", _tf, "read configuration from 'file'", "file" },
    { 'h', "", _ff, "prints this message and exits" },
    { 'i', "", _tf, "interface device to use", "dev" },
    {
        's', "uds_address", _tf, "server address for UDS",
        "path", "'/var/run/ptp4l'"
    },
    {
        't', "transportSpecific", _tf, "transport specific field",
        "hex", "0x0"
    },
    { 'v', "", _ff, "prints the software version and exits" },
    /* See Interpretation Response #29 in
     * IEEE Standards Interpretations for IEEE Std 1588-2008
     * https://standards.ieee.org/content/dam/ieee-standards/
     * standards/web/documents/interpretations/1588-2008_interp.pdf */
    { 'z', "", _ff, "send zero length TLV values with the GET actions" },
    // long only options
    { 'n', "network_transport", true, true },
    { 'M', "ptp_dst_mac", true, true },
    { 'S', "udp6_scope", true, true },
    { 'T', "udp_ttl", true, true },
    { 'P', "socket_priority", true, true },
    { 'A', "active_key_id", true, true },
    { 'B', "spp", true, true },
    { 'U', "allow_unauth", true, true },
    { 'F', "sa_file", true, true },
    { 0 },
};

string Options::helpStore::get(size_t length) const
{
    string ret = m_start;
    if(!m_end.empty()) {
        ret += string(length - m_start.length(), ' ');
        ret += m_end;
    }
    ret += "\n";
    return ret;
}

Options::Options(bool useDef)
{
    if(useDef)
        useDefOption();
}

void Options::useDefOption()
{
    if(m_useDef)
        return;
    m_net_opts = "u246";
    m_all_opts += m_net_opts;
    m_all_short_opts += m_net_opts;
    size_t sz = helpVec.size();
    helpVec.reserve(sz + std::max((size_t)6, sz));
    helpVec.emplace_back(" Network Transport\n");
    helpVec.emplace_back(" -2", "IEEE 802.3");
    helpVec.emplace_back(" -4", "UDP IPV4 (default)");
    helpVec.emplace_back(" -6", "UDP IPV6");
    helpVec.emplace_back(" -u", "UDS local\n");
    helpVec.emplace_back(" Other Options\n");
    helpUpdate = true;
    for(Pmc_option *cur = startOptions; cur->short_name; cur++)
        insert(*cur);
    m_useDef = true;
}

bool Options::insert(const Pmc_option &opt)
{
    // Verify we use legal character for short
    if(opt.short_name == 0 || strchr(":+-W", opt.short_name) != nullptr)
        return false;
    // short_name must be uniq
    if(m_all_opts.find(opt.short_name) != string::npos)
        return false;
    bool have_long_name = !opt.long_name.empty();
    if(opt.long_only) {
        if(!have_long_name)
            return false;
    } else {
        if((opt.have_arg && opt.arg_help.empty()) || opt.help_msg.empty())
            return false;
        m_all_short_opts += opt.short_name;
        helpStore h(" -");
        h.addStart(opt.short_name);
        if(opt.have_arg) {
            m_all_short_opts += ':';
            h.addStart(" [").addStart(opt.arg_help).addStart("]");
            m_max_arg_name = max(m_max_arg_name, opt.arg_help.length());
        }
        h.addEnd(opt.help_msg);
        if(opt.have_arg && !opt.def_val.empty())
            h.addEnd(", default ").addEnd(opt.def_val);
        helpVec.push_back(std::move(h));
        helpUpdate = true;
    }
    if(opt.have_arg)
        m_with_opts += opt.short_name;
    m_all_opts += opt.short_name;
    if(have_long_name) {
        option nopt;
        size_t len = opt.long_name.size();
        size_t size = m_last_strings + len + 1;
        if(size > m_strings.size() && !m_strings.alloc(size + 250))
            return false;
        char *cur =  m_last_strings + (char *)m_strings.get();
        memcpy(cur, opt.long_name.c_str(), len);
        m_last_strings = size;
        cur[len] = 0;
        nopt.name = cur;
        nopt.has_arg = opt.have_arg ? required_argument : no_argument;
        nopt.flag = nullptr;
        nopt.val = opt.short_name;
        m_long_opts_list.push_back(nopt);
    }
    return true;
}

const char *Options::get_help()
{
    if(helpUpdate) {
        help = "";
        for(const helpStore &a : helpVec)
            help += a.get(m_max_arg_name + 7);
        helpUpdate = false;
    }
    return help.c_str();
}

Options::loop_val Options::parse_options(int argc, char *const argv[])
{
    int c;
    // Prevent getopt_long() printing errors
    // Handle errors with '?'
    opterr = 0;
    optind = 1; // ensure we start from first argument!
    while((c = getopt_long(argc, argv, m_all_short_opts.c_str(),
                    m_long_opts_list.data(), nullptr)) != -1) {
        if(c == '?') { // Error handling
            m_msg = "invalid option -- '";
            m_msg += argv[optind - 1];
            m_msg += "'";
            return OPT_ERR;
        }
        if(m_useDef) {
            switch(c) {
                case 'v':
                    m_msg = getVersion();
                    return OPT_MSG;
                case 'h':
                    return OPT_HELP;
                case 'n':
                    if(strcasecmp(optarg, "UDPv4") == 0)
                        m_net_select = '4';
                    else if(strcasecmp(optarg, "UDPv6") == 0)
                        m_net_select = '6';
                    else if(strcasecmp(optarg, "L2") == 0)
                        m_net_select = '2';
                    else {
                        m_msg = "Wrong network transport -- '";
                        m_msg += optarg;
                        m_msg += "'";
                        return OPT_ERR;
                    }
                    continue; // To next option
                default:
                    break;
            }
        }
        if(m_net_opts.find(c) != string::npos)
            m_net_select = c; // Network Transport value
        else if(m_with_opts.find(c) != string::npos)
            m_opts[c] = optarg;
        else if(m_all_opts.find(c) != string::npos)
            m_opts[c] = "1";
        else {
            m_msg = "error";
            return OPT_ERR;
        }
    }
    m_argc = argc;
    m_end_optind = optind;
    return OPT_DONE;
}
const string &Options::val(char opt) const
{
    static const string empty;
    return have(opt) ? m_opts.at(opt) : empty;
}

__PTPMGMT_NAMESPACE_END

__PTPMGMT_NAMESPACE_USE;

extern "C" {
    static void ptpmgmt_opt_free(ptpmgmt_opt me)
    {
        if(me != nullptr) {
            if(me->_this != nullptr)
                delete(Options *)me->_this;
            free(me);
        }
    }
    static void ptpmgmt_opt_useDefOption(ptpmgmt_opt me)
    {
        if(me != nullptr && me->_this != nullptr)
            return ((Options *)me->_this)->useDefOption();
    }
    static bool ptpmgmt_opt_insert(ptpmgmt_opt me, ptpmgmt_opt_option *opt)
    {
        if(me != nullptr && me->_this != nullptr && opt != nullptr) {
#define _asn(a) .a = opt->a
#define _asn_s(a) .a = (opt->a == nullptr) ? n : opt->a
            const char *n = "";
            Pmc_option o = {
                _asn(short_name),
                _asn_s(long_name),
                _asn(have_arg),
                _asn(long_only),
                _asn_s(help_msg),
                _asn_s(arg_help),
                _asn_s(def_val),
            };
            return ((Options *)me->_this)->insert(o);
        }
        return false;
    }
    static const char *ptpmgmt_opt_get_help(ptpmgmt_opt me)
    {
        if(me != nullptr && me->_this != nullptr)
            return ((Options *)me->_this)->get_help();
        return nullptr;
    }
    static const char *ptpmgmt_opt_get_msg(ptpmgmt_opt me)
    {
        if(me != nullptr && me->_this != nullptr)
            return ((Options *)me->_this)->get_msg().c_str();
        return nullptr;
    }
    static ptpmgmt_opt_loop_val ptpmgmt_opt_parse_options(ptpmgmt_opt me,
        int argc, char *argv[])
    {
        if(me != nullptr && me->_this != nullptr && argc > 0 && argv != nullptr) {
            Options::loop_val v = ((Options *)me->_this)->parse_options(argc, argv);
            switch(v) {
                case Options::OPT_MSG:
                    return PTPMGMT_OPT_MSG;
                case Options::OPT_HELP:
                    return PTPMGMT_OPT_HELP;
                case Options::OPT_DONE:
                    return PTPMGMT_OPT_DONE;
                default:
                    break;
            }
        }
        return PTPMGMT_OPT_ERR;
    }
    static bool ptpmgmt_opt_have(ptpmgmt_opt me, char opt)
    {
        if(me != nullptr && me->_this != nullptr)
            return ((Options *)me->_this)->have(opt);
        return false;
    }
    static const char *ptpmgmt_opt_val(ptpmgmt_opt me, char opt)
    {
        if(me != nullptr && me->_this != nullptr) {
            const string &v = ((Options *)me->_this)->val(opt);
            if(!v.empty())
                return v.c_str();
        }
        return nullptr;
    }
    static int ptpmgmt_opt_val_i(ptpmgmt_opt me, char opt)
    {
        if(me != nullptr && me->_this != nullptr)
            return ((Options *)me->_this)->val_i(opt);
        return 0;
    }
    static char ptpmgmt_opt_get_net_transport(ptpmgmt_opt me)
    {
        if(me != nullptr && me->_this != nullptr)
            return ((Options *)me->_this)->get_net_transport();
        return 0;
    }
    static bool ptpmgmt_opt_have_more(ptpmgmt_opt me)
    {
        if(me != nullptr && me->_this != nullptr)
            return ((Options *)me->_this)->have_more();
        return false;
    }
    static int ptpmgmt_opt_process_next(ptpmgmt_opt me)
    {
        if(me != nullptr && me->_this != nullptr)
            return ((Options *)me->_this)->process_next();
        return 0;
    }
    static inline ptpmgmt_opt alloc_opt(bool useDef)
    {
        ptpmgmt_opt me = (ptpmgmt_opt)malloc(sizeof(ptpmgmt_opt_t));
        if(me == nullptr)
            return nullptr;
        me->_this = (void *)new Options(useDef);
        if(me->_this == nullptr) {
            free(me);
            return nullptr;
        }
#define C_ASGN(n) me->n = ptpmgmt_opt_##n
        C_ASGN(free);
        C_ASGN(useDefOption);
        C_ASGN(insert);
        C_ASGN(get_help);
        C_ASGN(get_msg);
        C_ASGN(parse_options);
        C_ASGN(have);
        C_ASGN(val);
        C_ASGN(val_i);
        C_ASGN(get_net_transport);
        C_ASGN(have_more);
        C_ASGN(process_next);
        return me;
    }
    ptpmgmt_opt ptpmgmt_opt_alloc()
    {
        return alloc_opt(true);
    }
    ptpmgmt_opt ptpmgmt_opt_alloc_empty()
    {
        return alloc_opt(false);
    }
}
