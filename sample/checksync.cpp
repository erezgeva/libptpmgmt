/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Utility program to wait until a PTP is synchronized
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 * @note Convert Vladimir Oltean <olteanv@gmail.com> application
 *  This is a sample code, not a product! You should use it as a reference.
 *  You can compile it with: g++ -Wall checksync.cpp -o checksync -lptpmgmt
 *
 */

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <poll.h>
#include <libgen.h>
#include <string>
#include <map>
#include <ptpmgmt/init.h>

using namespace ptpmgmt;

const uint64_t NS_PER_SEC = 1000000000LL;
const uint64_t PMC_UPDATE_INTERVAL = 60 * NS_PER_SEC;
const int PMC_SUBSCRIBE_DURATION = 180; /* 3 minutes */

static const size_t bufSize = 2000;
static char buf[bufSize];
static Init obj;
static Message &msg = obj.msg();
static Message msgu; // For local UDS
static SockBase *sk;
static SockUnix sku; // A local UDS socket, used if other socket is NOT UDS.
static bool use_uds;  // Is sk a UDS socket?
static unsigned int threshold;
static bool need_sync = true; // flag indicate all PTP are synchronized
static std::string designated_iface;

struct port_info {
    PortIdentity_t portid;
    portState_e portState;
    std::string interface;
    int64_t master_offset;
    bool local;
};

std::map<PortIdentity_t, port_info> ports;

static inline portState_e normalize_state(portState_e state)
{
    switch(state) {
        case PRE_TIME_TRANSMITTER:
        case TIME_TRANSMITTER:
        case UNCALIBRATED:
        case TIME_RECEIVER:
            return state;
        default:
            return DISABLED;
    }
};

const port_info *designated_port_get()
{
    if(designated_iface.empty())
        return nullptr;
    for(const auto &p : ports) {
        const auto &port = p.second;
        if(port.local && port.interface == designated_iface &&
            port.portState == TIME_RECEIVER)
            return &p.second;
    }
    for(const auto &p : ports) {
        const auto &port = p.second;
        if(port.local && port.portState == TIME_RECEIVER)
            return &p.second;
    }
    return nullptr;
}

void handle(bool local)
{
    const BaseMngTlv *data = msg.getData();
    PORT_PROPERTIES_NP_t *pp;
    PORT_DATA_SET_t *pd;
    portState_e portState;
    switch(msg.getTlvId()) {
        case PORT_PROPERTIES_NP:
            pp = (PORT_PROPERTIES_NP_t *)data;
            if(local || TS_SOFTWARE != pp->timestamping) {
                // create or update
                port_info &pi = ports[pp->portIdentity];
                pi.portid = pp->portIdentity;
                pi.portState = normalize_state(pp->portState);
                pi.interface = pp->interface.textField;
                pi.local = local;
            }
            return;
        case PORT_DATA_SET:
            pd = (PORT_DATA_SET_t *)data;
            portState = normalize_state(pd->portState);
            if(ports.count(pd->portIdentity) > 0) {
                port_info &pi = ports[pd->portIdentity];
                if(pi.portState == portState)
                    return;
                pi.portState = portState;
                printf("port %s (%s) changed state\n",
                    pd->portIdentity.string().c_str(), pi.interface.c_str());
            } else
                return;
            break;
        case TIME_STATUS_NP:
            if(ports.count(msg.getPeer()) > 0) {
                port_info &pi = ports[msg.getPeer()];
                if(pi.portState != TIME_RECEIVER)
                    return;
                pi.master_offset = ((TIME_STATUS_NP_t *)data)->master_offset;
            } else {
                printf("received time sync data for unknown port %s\n",
                    msg.getPeer().string().c_str());
                return;
            }
            break;
        default:
            return;
    }
    const port_info *port = designated_port_get();
    if(port != nullptr) {
        if(llabs(port->master_offset) > threshold) {
            printf("port %s (%s) offset %10jd under threshold %d",
                port->portid.string().c_str(), port->interface.c_str(),
                port->master_offset, threshold);
            return;
        }
        need_sync = false;
        return;
    }
    bool have_time_receivers = false;
    for(const auto &p : ports) {
        const auto &port = p.second;
        if(port.portState == TIME_RECEIVER) {
            if(llabs(port.master_offset) > threshold) {
                printf("port %s (%s) offset %10jd under threshold %d",
                    port.portid.string().c_str(), port.interface.c_str(),
                    port.master_offset, threshold);
                return;
            }
            have_time_receivers = true;
        }
    }
    if(have_time_receivers)
        need_sync = false;
}

static inline bool sendAction(bool local)
{
    static int seq = 0;
    Message *m;
    if(local)
        m = &msgu;
    else
        m = &msg;
    MNG_PARSE_ERROR_e err = m->build(buf, bufSize, seq);
    if(err != MNG_PARSE_ERROR_OK) {
        fprintf(stderr, "build error %s\n", msgu.err2str_c(err));
        return false;
    }
    bool ret;
    if(local && !use_uds)
        ret = sku.send(buf, m->getMsgLen());
    else
        ret = sk->send(buf, m->getMsgLen());
    if(!ret) {
        fprintf(stderr, "send failed\n");
        return false;
    }
    seq++;
    return true;
}
static inline bool sendGet(bool local, mng_vals_e id)
{
    bool ret;
    if(local)
        ret = msgu.setAction(GET, id);
    else
        ret = msg.setAction(GET, id);
    if(!ret) {
        fprintf(stderr, "Fail get %s\n", msg.mng2str_c(id));
        return false;
    }
    return sendAction(local);
}

bool send_subscription()
{
    SUBSCRIBE_EVENTS_NP_t d;
    memset(d.bitmask, 0, sizeof d.bitmask);
    d.duration = PMC_SUBSCRIBE_DURATION;
    d.setEvent(NOTIFY_PORT_STATE);
    if(!msg.setAction(SET, SUBSCRIBE_EVENTS_NP, &d)) {
        fprintf(stderr, "Fail set SUBSCRIBE_EVENTS_NP\n");
        return false;
    }
    bool ret = sendAction(false);
    msg.clearData(); // Remove referance to local SUBSCRIBE_EVENTS_NP_t
    return ret;
}

static void loop(void)
{
    for(;;) {
        if(!sendGet(true, PORT_PROPERTIES_NP))
            return;
        ssize_t cnt;
        const uint64_t timeout_ms = 1000; // 1 second
        if(use_uds) {
            sk->poll(timeout_ms);
            cnt = sk->rcv(buf, bufSize);
        } else {
            sku.poll(timeout_ms);
            cnt = sku.rcv(buf, bufSize);
        }
        if(cnt > 0) {
            MNG_PARSE_ERROR_e err = msg.parse(buf, cnt);
            if(err == MNG_PARSE_ERROR_OK) {
                handle(true);
                break;
            }
        }
    }
    // Send to all
    sendGet(false, PORT_PROPERTIES_NP);
    send_subscription();
    pollfd pd[2];
    nfds_t nfds;
    pd[0].fd = sk->fileno();
    pd[0].events = POLLIN;
    if(use_uds)
        nfds = 1;
    else {
        nfds = 2;
        pd[1].fd = sku.fileno();
        pd[1].events = POLLIN;
    }
    printf("start loop\n");
    while(need_sync) {
        const auto cnt = poll(pd, nfds, 0);
        if(cnt > 0) {
            if(pd[0].revents == POLLIN) {
                const auto cnt = sk->rcv(buf, bufSize);
                MNG_PARSE_ERROR_e err = msg.parse(buf, cnt);
                if(err == MNG_PARSE_ERROR_OK)
                    handle(false);
            } else if(!use_uds && pd[1].revents == POLLIN) {
                const auto cnt = sku.rcv(buf, bufSize);
                MNG_PARSE_ERROR_e err = msg.parse(buf, cnt);
                if(err == MNG_PARSE_ERROR_OK)
                    handle(true);
            }
        }
    }
}

static void handle_sig(int)
{
    obj.close();
    sku.close();
    exit(0);
}

static void handle_sig_ctrl(int)
{
    obj.close();
    sku.close();
    printf("\n");
    exit(0);
}

static inline void help(const std::string &app, const char *hmsg)
{
    fprintf(stderr, "\nusage: %s [options] [commands]\n\n%s\n",
        app.c_str(), hmsg);
}

static int init(Options &opt)
{
    int ret = obj.process(opt);
    if(ret)
        return ret;
    sk = obj.sk();
    threshold = opt.have('x') ? atoi(opt.val_c('x')) : 0;
    if(opt.have('i'))
        designated_iface = opt.val('i');
    // Ensure we recieve reply from local ptp daemon
    MsgParams prms = msg.getParams();
    prms.boundaryHops = 0;
    use_uds = obj.use_uds();
    if(!use_uds) {
        memset(prms.self_id.clockIdentity.v, 0, ClockIdentity_t::size());
        prms.self_id.portNumber = getpid() & 0xffff;
        std::string uds_address;
        if(opt.have('s'))
            uds_address = opt.val('s');
        else {
            std::string interface;
            if(opt.have('i') && !opt.val('i').empty())
                interface = opt.val('i');
            if(interface.empty()) {
                fprintf(stderr, "missing interface\n");
                return -1;
            }
            uds_address = obj.cfg().uds_address(interface);
        }
        if(uds_address.empty()) {
            fprintf(stderr, "missing UDS address\n");
            return -1;
        }
        if(!sku.setDefSelfAddress() || !sku.init() ||
            !sku.setPeerAddress(uds_address)) {
            fprintf(stderr, "failed to create uds transport\n");
            return -1;
        }
    }
    msgu.updateParams(prms);
    if(signal(SIGTERM, handle_sig) == SIG_ERR)
        fprintf(stderr, "sig term fails %m\n");
    // Control C, interrupt from keyboard
    if(signal(SIGINT, handle_sig_ctrl) == SIG_ERR)
        fprintf(stderr, "sig init fails %m\n");
    // quit from keyboard
    if(signal(SIGQUIT, handle_sig) == SIG_ERR)
        fprintf(stderr, "sig quit fails %m\n");
    // Hangup detected
    if(signal(SIGHUP, handle_sig) == SIG_ERR)
        fprintf(stderr, "sig hup fails %m\n");
    loop();
    return 0;
}

int main(int argc, char *argv[])
{
    Options opt;
    Pmc_option opt_x = {
        .short_name = 'x',
        .have_arg = true,
        .long_only = false,
        .help_msg = "set the sync offset threshold",
        .arg_help = "num",
        .def_val = "0",
    };
    opt.insert(opt_x);
    std::string app = basename(argv[0]);
    switch(opt.parse_options(argc, argv)) {
        case Options::OPT_ERR:
            fprintf(stderr, "%s: %s\n", app.c_str(), opt.get_msg_c());
            help(app, opt.get_help());
            return -1;
        case Options::OPT_MSG:
            printf("%s\n", opt.get_msg_c());
            return 0;
        case Options::OPT_HELP:
            help(app, opt.get_help());
            return 0;
        case Options::OPT_DONE:
            break;
    }
    int ret = init(opt);
    obj.close();
    sku.close();
    return ret;
}
