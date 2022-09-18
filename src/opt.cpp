/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/** @file
 * @brief Handle command line options
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#include <regex>
#include "opt.h"
#include "ver.h"

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

std::string Options::helpStore::get(size_t length) const
{
    std::string ret = start;
    if(!end.empty()) {
        ret += std::string(length - start.length(), ' ');
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
    for(auto *cur = startOptions; cur->short_name; cur++)
        insert(*cur);
    m_useDef = true;
}

bool Options::insert(const Pmc_option &opt)
{
    // Verify we use legal character for short
    if(opt.short_name == 0 || strchr(":+-W", opt.short_name) != nullptr)
        return false;
    // short_name must be uniq
    if(all_options.find(opt.short_name) != std::string::npos)
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
            max_arg_name = std::max(max_arg_name, opt.arg_help.length());
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
        for(const auto &a : helpVec)
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
        if(net_options.find(c) != std::string::npos)
            net_select = c; // Network Transport value
        else if(with_options.find(c) != std::string::npos)
            options[c] = optarg;
        else if(all_options.find(c) != std::string::npos)
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

__PTPMGMT_NAMESPACE_END
