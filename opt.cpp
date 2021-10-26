/* SPDX-License-Identifier: LGPL-3.0-or-later */

/** @file
 * @brief handle command line options
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 * @copyright GNU Lesser General Public License 3.0 or later
 *
 */

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

Options::loop_val Options::parse_options(int l_argc, char *const argv[])
{
    int c;
    opterr = 0; // Prevent printing the error
    while((c = getopt_long(l_argc, argv, all_short_options.c_str(),
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
    argc = l_argc;
    end_optind = optind;
    return OPT_DONE;
}
