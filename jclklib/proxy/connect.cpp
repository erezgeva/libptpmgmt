#include <proxy/connect.hpp>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <poll.h>
#include <libgen.h>
#include <string>
#include <map>
#include <ptpmgmt/init.h>
#include <sys/epoll.h>

//TODO: subsription part
//#include "thread.hpp"
//#include "ptp4l_connect.hpp"

using namespace std;
using namespace JClkLibProxy;
using namespace ptpmgmt;

const int SUBSCRIBE_DURATION = 180; /* 3 minutes */

static const size_t bufSize = 2000;
static char buf[bufSize];
static Init obj;
static Message &msg = obj.msg();
static Message msgu; // TODO: to be removed
static SockBase *sk;
static unsigned int threshold;

static std::string designated_iface;
static std::unique_ptr<SockBase> m_sk;

struct port_info {
    PortIdentity_t portid;
    portState_e portState;
    std::string interface;
    int64_t master_offset;
    bool local;
};

std::map<PortIdentity_t, port_info> ports;

int epd;
struct epoll_event epd_event;
SUBSCRIBE_EVENTS_NP_t d;

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
        return;
    }
    for(const auto &p : ports) {
        const auto &port = p.second;
        if(port.portState == TIME_RECEIVER) {
            if(llabs(port.master_offset) > threshold) {
                printf("port %s (%s) offset %10jd under threshold %d",
                    port.portid.string().c_str(), port.interface.c_str(),
                    port.master_offset, threshold);
                return;
            }
        }
    }
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
    if(!local)
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

//TODO: subscription part
//bool send_subscription(struct jcl_handle *handle)
bool send_subscription()
{
    memset(d.bitmask, 0, sizeof d.bitmask);
    d.duration = SUBSCRIBE_DURATION;
    d.setEvent(NOTIFY_PORT_STATE);
    d.setEvent(NOTIFY_TIME_SYNC); 

    //TODO: subscription part
/*   struct jcl_subscription subscription;
    subscription.event = jcl_offset_threshold_flag | jcl_peer_present_flag;
	subscription.threshold.offset = 100;
    handle->subscription = subscription;
*/
    if(!msg.setAction(SET, SUBSCRIBE_EVENTS_NP, &d)) {
        fprintf(stderr, "Fail set SUBSCRIBE_EVENTS_NP\n");
        return false;
    }
    bool ret = sendAction(false);
    msg.clearData(); // Remove referance to local SUBSCRIBE_EVENTS_NP_t
    return ret;
}

void *loop( void *arg)
//static void loop(void)
{
    const uint64_t timeout_ms = 1000; // 1 second
	for(;;) {
        if(!sendGet(true, PORT_PROPERTIES_NP))
                break;
        ssize_t cnt;

        sk->poll(timeout_ms);
        cnt = sk->rcv(buf, bufSize);

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

    while (1) {
        if (epoll_wait( epd, &epd_event, 1, 100) != -1) {
        //if ( sk->poll(timeout_ms)) {
            const auto cnt = sk->rcv(buf, bufSize);
            MNG_PARSE_ERROR_e err = msg.parse(buf, cnt);
            if(err == MNG_PARSE_ERROR_OK)
                handle(false);
        }
    }
}

int Connect::connect()
{
    // Ensure we recieve reply from local ptp daemon
    MsgParams prms = msg.getParams();
    prms.boundaryHops = 0;
    
    std::string interface;
    std::unique_ptr<SockBase> m_sk;
    std::string uds_address;

    //TODO: hard-coded the interface
    interface = "enp1s0";   
    SockUnix *sku = new SockUnix;
    if(sku == nullptr) {
        return -1;
    }
    m_sk.reset(sku);
    
    //TODO: hard-coded uds_socket
    uds_address = "/var/run/ptp4l";
    if(!sku->setDefSelfAddress() || !sku->init() ||
            !sku->setPeerAddress(uds_address))
        return -1;
    pid_t pid = getpid();
    prms.self_id.clockIdentity.v[6] = (pid >> 24) && 0xff;
    prms.self_id.clockIdentity.v[7] = (pid >> 16) && 0xff;
    prms.self_id.portNumber = pid & 0xffff;

    sk = m_sk.get();

    msgu.updateParams(prms);

    int ret;
    epd = epoll_create1( 0);
    if( epd == -1) {
		ret = -errno;
        printf("epoll create failed");
	}

	epd_event.data.fd = sk->fileno();
	epd_event.events  = ( EPOLLIN | EPOLLERR);
	if( epoll_ctl( epd, EPOLL_CTL_ADD, sk->fileno(), &epd_event) == 1) {
		ret = -errno;
        printf("epoll ctl failed");
	}
   
    //TODO: subscription part
    //jcl_handle_connect( NULL, epd_event);

    //TODO: to be removed, for testing purpose
    loop(NULL);
    return 0;
}
