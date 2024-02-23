/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Impleament linuxptp pmc tool using the libptpmgmt library
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 * Created following "IEEE Std 1588-2008", PTP version 2
 */

#include "pmc.h"

#ifndef INFTIM
#define INFTIM -1
#endif

/* Receive constants */
static const int wait = 500; // milli
// buffer for send and receive
static const size_t bufSize = 2000;
static uint8_t buf[bufSize];
static Init obj;
static Message &msg = obj.msg();
static SockBase *sk;
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
bool sendAction()
{
    static int seq = 0;
    MNG_PARSE_ERROR_e err = msg.build(buf, bufSize, seq);
    if(err != MNG_PARSE_ERROR_OK) {
        DUMPS("build error %s\n", msg.err2str_c(err));
        return false;
    }
    if(!sk->send(buf, msg.getMsgLen())) {
        PMCLERR;
        return false;
    }
    seq++;
    return true;
}
static inline int rcv()
{
    const ssize_t cnt = sk->rcv(buf, bufSize);
    if(cnt < 0) {
        PMCLERR;
        return -1;
    }
    MNG_PARSE_ERROR_e err = msg.parse(buf, cnt);
    switch(err) {
        case MNG_PARSE_ERROR_MSG:
            dump_err();
            break;
        case MNG_PARSE_ERROR_OK:
            dump_head(msg.getReplyAction());
            call_dump(msg);
            return 0;
        case MNG_PARSE_ERROR_SIG:
            dump_sig();
            msg.traversSigTlvs(call_dumpSig);
            return 1; // Do not count signalling messages
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
    do {
        if(!sk->tpoll(timeout)) {
            PMCLERR;
            break;
        }
    }while(rcv() == 1 && timeout > 0);
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
    if(!msg.findMngID(cur, id, false))
        return false;
    return call_data(msg, action, id, save);
}
void help(const std::string &app, const char *hmsg)
{
    fprintf(stderr, "\nusage: %s [options] [commands]\n\n%s\n",
        app.c_str(), hmsg);
}
[[noreturn]] static void handle_sig(int)
{
    obj.close();
    exit(0);
}
[[noreturn]] static void handle_sig_ctrl(int)
{
    obj.close();
    DUMPNL;
    exit(0);
}
int main(int argc, char *const argv[])
{
    Options opt;
    std::string app = basename(argv[0]);
    switch(opt.parse_options(argc, argv)) {
        case Options::OPT_ERR:
            fprintf(stderr, "%s: %s\n", app.c_str(), opt.get_msg_c());
            help(app, opt.get_help());
            return -1;
        case Options::OPT_MSG:
            DUMPS("%s\n", opt.get_msg_c());
            return 0;
        case Options::OPT_HELP:
            help(app, opt.get_help());
            return 0;
        case Options::OPT_DONE:
            break;
    }
    int ret = obj.process(opt);
    if(ret) {
        PMCLERR;
        return ret;
    }
    sk = obj.sk();
    use_uds = obj.use_uds();
    // allowed signalling TLV
    MsgParams prms = msg.getParams();
    prms.allowSigTlv(SLAVE_RX_SYNC_TIMING_DATA);
    prms.allowSigTlv(SLAVE_DELAY_TIMING_DATA_NP);
    bool batch = opt.have_more();
    // if we use real network layer and run mode, allow signalling
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
        for(int index = opt.process_next(); index < argc; index++) {
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
            fds[1].fd = sk->fileno();
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
    obj.close();
    return 0;
}
