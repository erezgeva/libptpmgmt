/* SPDX-License-Identifier: LGPL-3.0-or-later */

/** @file
 * @brief handle command line options
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 * @copyright GNU Lesser General Public License 3.0 or later
 *
 */

#include <cstring>
#include <regex>
#include "opt.h"
#include "ver.h"

pmc_option Options::start[] = {
    { 'b', nullptr, true, false, "boundary hops", "num", "1" },
    { 'd', "domainNumber", true, false, "domain number", "num", "0" },
    { 'f', nullptr, true, false, "read configuration from 'file'", "file" },
    { 'h', nullptr, false, false, "prints this message and exits" },
    { 'i', nullptr, true, false, "interface device to use", "dev", "'eth0'" },
    {
        's', "uds_address", true, false, "server address for UDS",
        "path", "'/var/run/ptp4l'"
    },
    {
        't', "transportSpecific", true, false, "transport specific field",
        "hex", "0x0"
    },
    { 'v', nullptr, false, false, "prints the software version and exits" },
    {
        /* See Interpretation Response #29 in
         * IEEE Standards Interpretations for IEEE Std 1588-2008
         * https://standards.ieee.org/content/dam/ieee-standards/
         * standards/web/documents/interpretations/1588-2008_interp.pdf */
        'z', nullptr, false, false,
        "send zero length TLV values with the GET actions"
    },
    { 'n', "network_transport", true, true },
    { 'M', "ptp_dst_mac", true, true },
    { 'S', "udp6_scope", true, true },
    { 'T', "udp_ttl", true, true },
    { 'P', "socket_priority", true, true },
    { 0 },
};

Options::Options(size_t l_max_arg_name) : max_arg_name(l_max_arg_name),
    net_options("u246"), all_options(net_options),
    all_short_options(net_options), net_select(0)
{
    help = std::regex_replace(
            " Network Transport\n\n"
            " -2 @!@ IEEE 802.3\n"
            " -4 @!@ UDP IPV4 (default)\n"
            " -6 @!@ UDP IPV6\n"
            " -u @!@ UDS local\n\n"
            " Other Options\n\n", std::regex("@!@"),
            std::string(2 + max_arg_name, ' '));
    for(auto *cur = start; cur->short_name; cur++)
        insert(*cur);
}

bool Options::insert(const pmc_option &opt)
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
        if(opt.have_arg && opt.arg_help == nullptr)
            return false;
        all_short_options += opt.short_name;
        help += " -";
        help += opt.short_name;
        if(opt.have_arg) {
            all_short_options += ':';
            size_t len = strlen(opt.arg_help);
            help += " [";
            help += opt.arg_help;
            help += "] ";
            if(len < max_arg_name)
                help.append(max_arg_name - len, ' ');
        } else
            help.append(4 + max_arg_name, ' ');
        help += opt.help_msg;
        if(opt.have_arg && opt.def_val != nullptr) {
            help += ", default ";
            help += opt.def_val;
        }
        help += "\n";
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

Options::loop_val Options::parse_options(int l_argc, char *const argv[])
{
    int c;
    while((c = getopt_long(l_argc, argv, all_short_options.c_str(),
                    long_options_list.data(), nullptr)) != -1) {
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
                    return OPT_EMSG;
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
        else
            return OPT_ERR;
    }
    argc = l_argc;
    end_optind = optind;
    return OPT_DONE;
}
