/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/** @file
 * @brief handle command line options
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#include <regex>
#include "opt.h"
#include "ver.h"

namespace ptpmgmt
{

#define _c(s) const_cast<char*>(#s)
#define _n nullptr
#define _tf true, false  // with argument
#define _ff false, false // without argument
#define _tt true, true   // long only option
Pmc_option Options::start[] = {
    { 'b', _n, _tf, _c(boundary hops), _c(num), _c(1) },
    { 'd', _c(domainNumber), _tf, _c(domain number), _c(num), _c(0) },
    { 'f', _n, _tf, _c(read configuration from 'file'), _c(file) },
    { 'h', _n, _ff, _c(prints this message and exits) },
    { 'i', _n, _tf, _c(interface device to use), _c(dev) },
    {
        's', _c(uds_address), _tf, _c(server address for UDS),
            _c(path), _c('/var/run/ptp4l')
        },
    {
        't', _c(transportSpecific), _tf, _c(transport specific field),
        _c(hex), _c(0x0)
    },
    { 'v', _n, _ff, _c(prints the software version and exits) },
    /* See Interpretation Response #29 in
     * IEEE Standards Interpretations for IEEE Std 1588-2008
     * https://standards.ieee.org/content/dam/ieee-standards/
     * standards/web/documents/interpretations/1588-2008_interp.pdf */
    { 'z', _n, _ff, _c(send zero length TLV values with the GET actions) },
    { 'n', _c(network_transport), _tt },
    { 'M', _c(ptp_dst_mac), _tt },
    { 'S', _c(udp6_scope), _tt },
    { 'T', _c(udp_ttl), _tt },
    { 'P', _c(socket_priority), _tt },
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

Options::Options(bool useDef) : max_arg_name(0), net_select(0), m_useDef(false)
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
    for(auto *cur = start; cur->short_name; cur++)
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
    bool have_long = opt.long_name != nullptr && opt.long_name[0] != 0;
    if(opt.long_only) {
        if(!have_long)
            return false;
    } else {
        if((opt.have_arg && opt.arg_help == nullptr) || opt.help_msg == nullptr)
            return false;
        all_short_options += opt.short_name;
        helpStore h(" -");
        h.addStart(opt.short_name);
        if(opt.have_arg) {
            all_short_options += ':';
            h.addStart(" [").addStart(opt.arg_help).addStart("]");
            max_arg_name = std::max(max_arg_name, strlen(opt.arg_help));
        }
        h.addEnd(opt.help_msg);
        if(opt.have_arg && opt.def_val != nullptr)
            h.addEnd(", default ").addEnd(opt.def_val);
        helpVec.push_back(h);
    }
    if(opt.have_arg)
        with_options += opt.short_name;
    all_options += opt.short_name;
    if(have_long) {
        option nopt;
        nopt.name = opt.long_name;
        nopt.has_arg = opt.have_arg ? required_argument : no_argument;
        nopt.flag = nullptr;
        nopt.val = opt.short_name;
        long_options_list.push_back(nopt);
    }
    return true;
}

const char *Options::get_help()
{
    help = "";
    for(const auto &a : helpVec)
        help += a.get(max_arg_name + 7);
    return help.c_str();
}

Options::loop_val Options::parse_options(int argc, char *const argv[])
{
    int c;
    opterr = 0; // Prevent printing the error
    while((c = getopt_long(argc, argv, all_short_options.c_str(),
                    long_options_list.data(), nullptr)) != -1) {
        switch(c) {
            case '?':
                msg = "invalid option -- '";
                msg += argv[optind - 1];
                msg += "'";
                return OPT_ERR;
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
                continue;
            default:
                break;
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

}; /* namespace ptpmgmt */
