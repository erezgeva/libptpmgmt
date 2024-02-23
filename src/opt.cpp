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
#include "ver.h"
#include "comp.h"

using namespace std;
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
    { 0 },
};

string Options::helpStore::get(size_t length) const
{
    string ret = start;
    if(!end.empty()) {
        ret += string(length - start.length(), ' ');
        ret += end;
    }
    ret += "\n";
    return ret;
}

Options::Options(bool useDef) : max_arg_name(0), net_select(0), m_useDef(false),
    helpUpdate(false)
{
    if(useDef)
        useDefOption();
}

void Options::useDefOption()
{
    if(m_useDef)
        return;
    net_options = "u246";
    all_options += net_options;
    all_short_options += net_options;
    helpVec.push_back(helpStore(" Network Transport\n"));
    helpVec.push_back(helpStore(" -2", "IEEE 802.3"));
    helpVec.push_back(helpStore(" -4", "UDP IPV4 (default)"));
    helpVec.push_back(helpStore(" -6", "UDP IPV6"));
    helpVec.push_back(helpStore(" -u", "UDS local\n"));
    helpVec.push_back(helpStore(" Other Options\n"));
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
    if(all_options.find(opt.short_name) != string::npos)
        return false;
    bool have_long_name = !opt.long_name.empty();
    if(opt.long_only) {
        if(!have_long_name)
            return false;
    } else {
        if((opt.have_arg && opt.arg_help.empty()) || opt.help_msg.empty())
            return false;
        all_short_options += opt.short_name;
        helpStore h(" -");
        h.addStart(opt.short_name);
        if(opt.have_arg) {
            all_short_options += ':';
            h.addStart(" [").addStart(opt.arg_help).addStart("]");
            max_arg_name = max(max_arg_name, opt.arg_help.length());
        }
        h.addEnd(opt.help_msg);
        if(opt.have_arg && !opt.def_val.empty())
            h.addEnd(", default ").addEnd(opt.def_val);
        helpVec.push_back(h);
        helpUpdate = true;
    }
    if(opt.have_arg)
        with_options += opt.short_name;
    all_options += opt.short_name;
    if(have_long_name) {
        option nopt;
        auto iter = long_options_list_string.insert(long_options_list_string.end(),
                opt.long_name);
        nopt.name = iter->c_str();
        nopt.has_arg = opt.have_arg ? required_argument : no_argument;
        nopt.flag = nullptr;
        nopt.val = opt.short_name;
        long_options_list.push_back(nopt);
    }
    return true;
}

const char *Options::get_help()
{
    if(helpUpdate) {
        help = "";
        for(const helpStore &a : helpVec)
            help += a.get(max_arg_name + 7);
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
    while((c = getopt_long(argc, argv, all_short_options.c_str(),
                    long_options_list.data(), nullptr)) != -1) {
        if(c == '?') { // Error handling
            msg = "invalid option -- '";
            msg += argv[optind - 1];
            msg += "'";
            return OPT_ERR;
        }
        if(m_useDef) {
            switch(c) {
                case 'v':
                    msg = getVersion();
                    return OPT_MSG;
                case 'h':
                    return OPT_HELP;
                case 'n':
                    if(strcasecmp(optarg, "UDPv4") == 0)
                        net_select = '4';
                    else if(strcasecmp(optarg, "UDPv6") == 0)
                        net_select = '6';
                    else if(strcasecmp(optarg, "L2") == 0)
                        net_select = '2';
                    else {
                        msg = "Wrong network transport -- '";
                        msg += optarg;
                        msg += "'";
                        return OPT_ERR;
                    }
                    continue; // To next option
                default:
                    break;
            }
        }
        if(net_options.find(c) != string::npos)
            net_select = c; // Network Transport value
        else if(with_options.find(c) != string::npos)
            options[c] = optarg;
        else if(all_options.find(c) != string::npos)
            options[c] = "1";
        else {
            msg = "error";
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
    return have(opt) ? options.at(opt) : empty;
}

__PTPMGMT_NAMESPACE_END

__PTPMGMT_NAMESPACE_USE;

extern "C" {

#include "c/opt.h"

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
    static bool ptpmgmt_opt_insert(ptpmgmt_opt me, struct ptpmgmt_opt_option *opt)
    {
        if(me != nullptr && me->_this != nullptr && opt != nullptr) {
            Pmc_option o = {
                .short_name = opt->short_name,
                .have_arg = opt->have_arg,
                .long_only = opt->long_only
            };
            if(opt->long_name != nullptr)
                o.long_name = opt->long_name;
            if(opt->help_msg != nullptr)
                o.help_msg = opt->help_msg;
            if(opt->arg_help != nullptr)
                o.arg_help = opt->arg_help;
            if(opt->def_val != nullptr)
                o.def_val = opt->def_val;
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
    static enum ptpmgmt_opt_loop_val ptpmgmt_opt_parse_options(ptpmgmt_opt me,
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
        me->free = ptpmgmt_opt_free;
        me->useDefOption = ptpmgmt_opt_useDefOption;
        me->insert = ptpmgmt_opt_insert;
        me->get_help = ptpmgmt_opt_get_help;
        me->get_msg = ptpmgmt_opt_get_msg;
        me->parse_options = ptpmgmt_opt_parse_options;
        me->have = ptpmgmt_opt_have;
        me->val = ptpmgmt_opt_val;
        me->val_i = ptpmgmt_opt_val_i;
        me->get_net_transport = ptpmgmt_opt_get_net_transport;
        me->have_more = ptpmgmt_opt_have_more;
        me->process_next = ptpmgmt_opt_process_next;
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
