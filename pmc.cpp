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
#include "pmc.h"
#include "opt.h"

#ifndef INFTIM
#define INFTIM -1
#endif

/* Receive constants */
static const int wait = 500; // milli
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
    if(end == port || *end != 0 || val < 0 || val > UINT16_MAX)
        return false;
    prms.target.portNumber = val & 0xffff;
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
    const auto cnt = sk->rcv(buf, bufSize);
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
        if(!msg.setAction(action, id, data))
            return false;
    }
    if(!sendAction())
        return false;
    // Finish with data, free it
    DUMPS("sending: %s %s\n", msg.act2str_c(msg.getSendAction()),
        msg.mng2str_c(id));
    if(data != nullptr) {
        msg.clearData();
        delete data;
    }
    return true;
}
void help(const std::string &app, const char *hmsg)
{
    fprintf(stderr, "\nusage: %s [options] [commands]\n\n%s\n",
        app.c_str(), hmsg);
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
    Options opt;
    std::string app = basename(argv[0]);
    switch(opt.parse_options(argc, argv)) {
        case Options::OPT_EMSG:
            fprintf(stderr, "%s: %s\n", app.c_str(), opt.get_msg());
        case Options::OPT_ERR:
            help(app, opt.get_help());
            return -1;
        case Options::OPT_MSG:
            DUMPS("%s\n", opt.get_msg());
            return 0;
        case Options::OPT_HELP:
            help(app, opt.get_help());
            return 0;
        case Options::OPT_DONE:
            break;
    }
    char net_select = opt.get_net_transport();
    ConfigFile cfg;
    /* handle configuration file */
    if(opt.have('f') && !cfg.read_cfg(opt.val('f')))
        return -1;
    if(net_select == 0)
        net_select = cfg.network_transport();
    std::string interface;
    if(opt.have('i') && !opt.val('i').empty())
        interface = opt.val('i');
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
    if(opt.have('b'))
        prms.boundaryHops = opt.val_i('b');
    else
        prms.boundaryHops = 1;
    if(opt.have('d'))
        prms.domainNumber = opt.val_i('d');
    else
        prms.domainNumber = cfg.domainNumber(interface);
    if(opt.have('t'))
        prms.transportSpecific = strtol(opt.val_c('t'), nullptr, 16);
    else
        prms.transportSpecific = cfg.transportSpecific(interface);
    prms.useZeroGet = opt.val('z') == "1";
    switch(net_select) {
        case 'u': {
            SockUnix *sku = new SockUnix;
            if(sku == nullptr) {
                PMCERR("failed to allocate SockUnix");
                return -1;
            }
            sk.reset(sku);
            std::string uds_address;
            if(opt.have('s'))
                uds_address = opt.val('s');
            else
                uds_address = cfg.uds_address(interface);
            if(!sku->setDefSelfAddress() || !sku->init() ||
                !sku->setPeerAddress(uds_address)) {
                PMCERR("failed to create transport");
                return -1;
            }
            prms.self_id.portNumber = getpid() & 0xffff;
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
            if(opt.have('T') && !sk4->setUdpTtl(opt.val_i('T'))) {
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
            if(opt.have('T') && !sk6->setUdpTtl(opt.val_i('T'))) {
                PMCERR("failed to set udp_ttl");
                return -1;
            }
            if(opt.have('S') && !sk6->setScope(opt.val_i('S'))) {
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
            if(opt.have('P') &&
                !skr->setSocketPriority(opt.val_i('P'))) {
                PMCERR("failed to set socket_priority");
                return -1;
            }
            Binary mac;
            if(opt.have('M') && (!mac.fromMac(opt.val('M')) ||
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
    bool batch = opt.have_more();
    // if we use real network layer and run mode, allow signaling
    if(!batch && !use_uds)
        prms.rcvSignaling = true;
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
        for(int index = opt.procces_next(); index < argc; index++) {
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
