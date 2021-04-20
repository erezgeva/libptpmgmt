/* SPDX-License-Identifier: GPL-3.0-or-later */

/** @file
 * @brief Impleament linuxptp pmc tool using the libpmc library
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 * @copyright GNU General Public License 3.0 or later
 *
 * Created following "IEEE Std 1588-2008", PTP version 2
 */

#include <signal.h>
#include <unistd.h>
#include <poll.h>
#include <getopt.h>
#include <libgen.h>
#include "msg.h"
#include "ptp.h"
#include "sock.h"
#include "bin.h"
#include "ver.h"

#ifndef INFTIM
#define INFTIM -1
#endif
#define PMCERR(msg) fprintf(stderr, msg "\n")
#define DUMPS(format, ...) printf(format, __VA_ARGS__)
#define DUMPNL printf("\n");
/* from pmc_dump.cpp */
extern void call_dump(Message &msg, BaseMngTlv *data);
extern bool call_dumpSig(const Message &msg, tlvType_e tlvType,
    BaseSigTlv *tlv);
extern BaseMngTlv *call_data(Message &msg, mng_vals_e id, char *save);

/* Receive constants */
static const int wait = 500; // milli

static const char toksep[] = " \t\n\r"; // while spaces
// buffer for send and receive
static const size_t bufSize = 2000;
static char buf[bufSize];
static Message msg;
static std::unique_ptr<SockBase> sk;
static Binary clockIdentity;
static bool use_uds;
static uint64_t timeout;

static inline void dump_head(actionField_e action)
{
    DUMPS("\t%s seq %u %s MANAGEMENT %s ",
        msg.getPeer().string().c_str(),
        msg.getSequence(),
        msg.act2str_c(action),
        msg.mng2str_c(msg.getTlvId()));
}
static inline void dump_sig()
{
    DUMPS("\t%s seq %u SIGNALING\n",
        msg.getPeer().string().c_str(),
        msg.getSequence());
}
static inline void dump_err()
{
    DUMPS("\t%s seq %u %s MANAGEMENT_ERROR_STATUS %s\n"
        "\tERROR: %s(%u)\n"
        "\tERROR DISPLAY: %s\n\n",
        msg.getPeer().string().c_str(),
        msg.getSequence(),
        msg.act2str_c(msg.getReplyAction()),
        msg.mng2str_c(msg.getTlvId()),
        msg.errId2str_c(msg.getErrId()),
        msg.getErrId(),
        msg.getErrDisplay_c());
}
static inline bool updatePortIdentity(MsgParams &prms, char *str)
{
    char *port = strchr(str, '-');
    if(port == nullptr)
        return false;
    *port = 0;
    Binary nc;
    if(!nc.fromHex(str) || nc.length() != ClockIdentity_t::size())
        return false;
    port++;
    char *end;
    long val = strtol(port, &end, 0);
    if(*end != 0 || val < 0 || val > UINT16_MAX)
        return false;
    prms.target.portNumber = val;
    nc.copy(prms.target.clockIdentity.v);
    return true;
}
static inline bool sendAction()
{
    static int seq = 0;
    MNG_PARSE_ERROR_e err = msg.build(buf, bufSize, seq);
    if(err != MNG_PARSE_ERROR_OK) {
        DUMPS("build error %s\n", msg.err2str_c(err));
        return false;
    }
    if(!sk->send(buf, msg.getMsgLen()))
        return false;
    seq++;
    return true;
}
static inline int rcv()
{
    auto cnt = sk->rcv(buf, bufSize);
    if(cnt < 0)
        return -1;
    MNG_PARSE_ERROR_e err = msg.parse(buf, cnt);
    switch(err) {
        case MNG_PARSE_ERROR_MSG:
            dump_err();
            break;
        case MNG_PARSE_ERROR_OK:
            dump_head(msg.getReplyAction());
            call_dump(msg, nullptr);
            return 0;
        case MNG_PARSE_ERROR_SIG:
            dump_sig();
            msg.traversSigTlvs(call_dumpSig);
            return 1; // Do not count signaling messages
        case MNG_PARSE_ERROR_ACTION: // Not management, or another clock id
        case MNG_PARSE_ERROR_HEADER: // Not reply
            if(!use_uds)
                // We got the wrong message, wait for the next one
                return 1;
        default:
            DUMPS("Parse error %s\n", msg.err2str_c(err));
            break;
    }
    return -1;
}
static inline void rcv_timeout()
{
    timeout = wait;
    while(sk->tpoll(timeout) && rcv() == 1 && timeout > 0);
}
static inline bool findId(mng_vals_e &id, char *str)
{
    size_t len = strlen(str);
    if(len == 0)
        return false;
    // Make string uppercase as all commands are uppercase
    for(char *cur = str; *cur; cur++)
        *cur = toupper(*cur);
    if(strstr(str, "NULL") != nullptr) {
        id = NULL_PTP_MANAGEMENT;
        return true;
    }
    int find = 0;
    for(int i = FIRST_MNG_ID; i <= LAST_MNG_ID; i++) {
        const char *sid = Message::mng2str_c((mng_vals_e)i);
        if(strcmp(sid, str) == 0) {
            id = (mng_vals_e)i;
            return true; // Exact match!
        }
        if(find < 2 && strncmp(sid, str, len) == 0) {
            id = (mng_vals_e)i;
            find++;
        }
    }
    // 1 match
    return find == 1;
}
static bool run_line(char *line)
{
    char *save;
    char *cur = strtok_r(line, toksep, &save);
    if(cur == nullptr)
        return false;
    actionField_e action;
    if(strcasecmp(cur, "get") == 0)
        action = GET;
    else if(strcasecmp(cur, "set") == 0)
        action = SET;
    else if(strcasecmp(cur, "cmd") == 0 || strcasecmp(cur, "command") == 0)
        action = COMMAND;
    else if(strcasecmp(cur, "target") == 0) {
        cur = strtok_r(nullptr, toksep, &save);
        if(cur == nullptr || *cur == 0)
            return false;
        if(*cur == '*')
            msg.setAllClocks();
        else {
            MsgParams prms = msg.getParams();
            if(updatePortIdentity(prms, cur))
                msg.updateParams(prms);
            else
                PMCERR("Wrong clock ID");
        }
        return false;
    } else
        return false;
    cur = strtok_r(nullptr, toksep, &save);
    if(cur == nullptr)
        return false;
    mng_vals_e id;
    if(!findId(id, cur))
        return false;
    BaseMngTlv *data;
    if(action == GET || msg.isEmpty(id)) {
        // No data is needed
        if(!msg.setAction(action, id))
            return false;
        data = nullptr;
    } else {
        data = call_data(msg, id, save);
        // No point to send without data
        if(data == nullptr)
            return false;
        if(!msg.setAction(action, id, *data))
            return false;
    }
    if(!sendAction())
        return false;
    // Finish with data, free it
    DUMPS("sending: %s %s\n", msg.act2str_c(msg.getSendAction()),
        msg.mng2str_c(id));
    if(data != nullptr)
        delete data;
    return true;
}
void help(char *app)
{
    fprintf(stderr, "\n"
        "usage: %s [options] [commands]\n\n"
        " Network Transport\n\n"
        " -2        IEEE 802.3\n"
        " -4        UDP IPV4 (default)\n"
        " -6        UDP IPV6\n"
        " -u        UDS local\n\n"
        " Other Options\n\n"
        " -b [num]  boundary hops, default 1\n"
        " -d [num]  domain number, default 0\n"
        " -f [file] read configuration from 'file'\n"
        " -h        prints this message and exits\n"
        " -i [dev]  interface device to use, default 'eth0'\n"
        "           for network and '/var/run/pmc.$pid' for UDS.\n"
        " -s [path] server address for UDS, default '/var/run/ptp4l'.\n"
        " -t [hex]  transport specific field, default 0x0\n"
        " -v        prints the software version and exits\n"
        " -z        send zero length TLV values with the GET actions\n\n",
        basename(app));
}
static void handle_sig(int)
{
    sk->close();
    exit(0);
}
static void handle_sig_ctrl(int)
{
    sk->close();
    DUMPNL;
    exit(0);
}
int main(int argc, char *const argv[])
{
    int c;
    std::map<int, std::string> options;
    const char *with_options = "fbdsti";
    const char *no_options = "zvh";
    const char *net_options = "u246";
    const char *l_options = "MSTP";
    const option long_options[] = {
        // .name               .has_arg           .flag    .val
        // Long option for short option
        { "domainNumber",      required_argument, nullptr, 'd' },
        { "uds_address",       required_argument, nullptr, 's' },
        { "transportSpecific", required_argument, nullptr, 't' },
        // Long only options
        { "network_transport", required_argument, nullptr, 'n' },
        { "ptp_dst_mac",       required_argument, nullptr, 'M' },
        { "udp6_scope",        required_argument, nullptr, 'S' },
        { "udp_ttl",           required_argument, nullptr, 'T' },
        { "socket_priority",   required_argument, nullptr, 'P' },
        // termination
        { nullptr,             0,                 nullptr,  0  }
    };
    std::string opts = net_options;
    opts += no_options;
    for(size_t i = 0; i < strlen(with_options); i++) {
        opts += with_options[i];
        opts += ':';
    }
    char net_select = 0;
    bool zero_get = false;
    while((c = getopt_long(argc, argv, opts.c_str(), long_options,
                    nullptr)) != -1) {
        switch(c) {
            case ':':
                PMCERR("Wrong option ':'");
                return -1;
            case 'v':
                DUMPS("%s\n", getVersion());
                return 0;
            case 'h':
                help(argv[0]);
                return 0;
            case 'z':
                /* See Interpretation Response #29 in
                 * IEEE Standards Interpretations for IEEE Std 1588-2008
                 * https://standards.ieee.org/content/dam/ieee-standards/
                 * standards/web/documents/interpretations/1588-2008_interp.pdf */
                zero_get = true;
                continue;
            case 'n':
                if(strcasecmp(optarg, "UDPv4") == 0)
                    net_select = '4';
                else if(strcasecmp(optarg, "UDPv6") == 0)
                    net_select = '6';
                else if(strcasecmp(optarg, "L2") == 0)
                    net_select = '2';
                else {
                    PMCERR("Wrong network_transport");
                    return -1;
                }
                continue;
            default:
                break;
        }
        if(strrchr(with_options, c) != nullptr || strrchr(l_options, c) != nullptr)
            options[c] = optarg;
        else if(strrchr(net_options, c) != nullptr)
            net_select = c;
        else {
            help(argv[0]);
            return -1;
        }
    }
    ConfigFile cfg;
    /* handle configuration file */
    if(options.count('f') && !cfg.read_cfg(options['f']))
        return -1;
    if(net_select == 0)
        net_select = cfg.network_transport();
    std::string interface;
    if(options.count('i') && !options['i'].empty())
        interface = options['i'];
    IfInfo ifObj;
    MsgParams prms = msg.getParams();
    if(net_select != 'u') {
        if(interface.empty()) {
            PMCERR("missing interface");
            return -1;
        }
        if(!ifObj.initUsingName(interface))
            return -1;
        clockIdentity = ifObj.mac();
        clockIdentity.eui48ToEui64();
        clockIdentity.copy(prms.self_id.clockIdentity.v);
        prms.self_id.portNumber = 1;
        use_uds = false;
    }
    if(options.count('b'))
        prms.boundaryHops = atoi(options['b'].c_str());
    else
        prms.boundaryHops = 1;
    if(options.count('d'))
        prms.domainNumber = atoi(options['b'].c_str());
    else
        prms.domainNumber = cfg.domainNumber(interface);
    if(options.count('t'))
        prms.transportSpecific = strtol(options['t'].c_str(), nullptr, 16);
    else
        prms.transportSpecific = cfg.transportSpecific(interface);
    switch(net_select) {
        case 'u': {
            SockUnix *sku = new SockUnix;
            if(sku == nullptr) {
                PMCERR("failed to allocate SockUnix");
                return -1;
            }
            sk.reset(sku);
            std::string uds_address;
            if(options.count('s'))
                uds_address = options['s'];
            else
                uds_address = cfg.uds_address(interface);
            if(!sku->setDefSelfAddress() || !sku->init() ||
                !sku->setPeerAddress(uds_address)) {
                PMCERR("failed to create transport");
                return -1;
            }
            prms.self_id.portNumber = getpid();
            use_uds = true;
            break;
        }
        default:
        case '4': {
            SockIp4 *sk4 = new SockIp4;
            if(sk4 == nullptr) {
                PMCERR("failed to allocate SockIp4");
                return -1;
            }
            sk.reset(sk4);
            if(!sk4->setAll(ifObj, cfg, interface)) {
                PMCERR("failed to set transport");
                return -1;
            }
            if(options.count('T') && !sk4->setUdpTtl(atoi(options['T'].c_str()))) {
                PMCERR("failed to set udp_ttl");
                return -1;
            }
            if(!sk4->init()) {
                PMCERR("failed to init transport");
                return -1;
            }
            break;
        }
        case '6': {
            SockIp6 *sk6 = new SockIp6;
            if(sk6 == nullptr) {
                PMCERR("failed to allocate SockIp6");
                return -1;
            }
            sk.reset(sk6);
            if(!sk6->setAll(ifObj, cfg, interface)) {
                PMCERR("failed to set transport");
                return -1;
            }
            if(options.count('T') && !sk6->setUdpTtl(atoi(options['T'].c_str()))) {
                PMCERR("failed to set udp_ttl");
                return -1;
            }
            if(options.count('S') && !sk6->setScope(atoi(options['S'].c_str()))) {
                PMCERR("failed to set udp6_scope");
                return -1;
            }
            if(!sk6->init()) {
                PMCERR("failed to init transport");
                return -1;
            }
            break;
        }
        case '2': {
            SockRaw *skr = new SockRaw;
            if(skr == nullptr) {
                PMCERR("failed to allocate SockRaw");
                return -1;
            }
            sk.reset(skr);
            if(!skr->setAll(ifObj, cfg, interface)) {
                PMCERR("failed to set transport");
                return -1;
            }
            if(options.count('P') &&
                !skr->setSocketPriority(atoi(options['P'].c_str()))) {
                PMCERR("failed to set socket_priority");
                return -1;
            }
            Binary mac;
            if(options.count('M') && (!mac.fromMac(options['M']) ||
                    !skr->setPtpDstMac(mac))) {
                PMCERR("failed to set ptp_dst_mac");
                return -1;
            }
            if(!skr->init()) {
                PMCERR("failed to init transport");
                return -1;
            }
            break;
        }
    }
    // allowed signaling TLV
    prms.allowSigTlvs[SLAVE_RX_SYNC_TIMING_DATA] = true;
    prms.allowSigTlvs[SLAVE_DELAY_TIMING_DATA_NP] = true;
    bool batch = optind < argc;
    // if we use real network layer and run mode, allow signaling
    if(!batch && !use_uds)
        prms.rcvSignaling = true;
    prms.useZeroGet = zero_get;
    msg.updateParams(prms);
    // Normal Termination (by kill)
    if(signal(SIGTERM, handle_sig) == SIG_ERR)
        PMCERR("sig term fails %m");
    // Control C, interrupt from keyboard
    if(signal(SIGINT, handle_sig_ctrl) == SIG_ERR)
        PMCERR("sig init fails %m");
    // quit from keyboard
    if(signal(SIGQUIT, handle_sig) == SIG_ERR)
        PMCERR("sig quit fails %m");
    // Hangup detected
    if(signal(SIGHUP, handle_sig) == SIG_ERR)
        PMCERR("sig hup fails %m");
    if(batch) {
        // batch mode
        int pkts = 0;
        // First we send all the commands, then we receive them all
        for(int index = optind; index < argc; index++) {
            if(run_line(argv[index]))
                pkts++;
        }
        for(; pkts > 0; pkts--)
            rcv_timeout();
    } else {
        char lineBuf[bufSize];
        if(use_uds) {
            // We only receive after sending a command
            while(fgets(lineBuf, bufSize, stdin) != nullptr)
                if(run_line(lineBuf))
                    rcv_timeout();
        } else {
            pollfd fds[2];
            // standard input
            fds[0].fd = STDIN_FILENO;
            fds[0].events = POLLIN;
            // network socket
            fds[1].fd = sk->getFd();
            fds[1].events = POLLIN;
            // We receive except when we type a command
            for(;;) {
                if(poll(fds, 2, INFTIM) > 0) {
                    if(fds[1].revents & POLLIN)
                        rcv();
                    else if(fds[0].revents & POLLIN) {
                        if(fgets(lineBuf, bufSize, stdin) == nullptr)
                            break; // End Of File, Control D
                        run_line(lineBuf);
                    }
                }
            }
        }
    }
    sk->close();
    return 0;
}
