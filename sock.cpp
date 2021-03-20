/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* sock.cpp use unix socket to communicate with ptp4l
 *
 * Authors: Erez Geva <ErezGeva2@gmail.com>
 *
 */

#include <pwd.h>
#include <unistd.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <linux/filter.h>
#include "end.h"
#include "sock.h"

const uint16_t udp_port = 320;
const char *ipv4_udp_mc = "224.0.1.129";
const char *ipv6_udp_mc = "ff0e::181";

const char *useDefstr = "/.pmc.";
const char *rootBasestr = "/var/run/pmc.";
const size_t unix_path_max = sizeof(((sockaddr_un *)nullptr)->sun_path) - 1;

// Berkeley Packet Filter code
// The code run on network order (big endian).
// 0x30 Load bottom (2 last bytes of word)
const uint16_t OP_LDB = BPF_LD  | BPF_B   | BPF_ABS;
// 0x28 Load high (2 first bytes)
const uint16_t OP_LDH = BPF_LD  | BPF_H   | BPF_ABS;
// 0x20 Load word (4 bytes)
const uint16_t OP_LDW = BPF_LD  | BPF_W   | BPF_ABS;
// 0x15 Jump Equal
const uint16_t OP_JEQ = BPF_JMP | BPF_JEQ | BPF_K;
//  0x6 Return with pass or drop
const uint16_t OP_RET = BPF_RET | BPF_K;

/*
 * Filter to receive PTP frames with ethernet protocol 1558
 * From: sudo tcpdump -d  ether proto 0x88F7
 *       sudo tcpdump -dd ether proto 0x88F7
 * See: 'man 7 pcap-filter' for filter syntax
 */
const sock_filter bpf_code[] = {
    // opcode  Jump true  Jump false  field (32 bits)
    { OP_LDH,  0,         0,          12 },
    { OP_JEQ,  0,         1,          ETH_P_1588 },
    { OP_RET,  0,         0,          0x40000 }, // pass
    { OP_RET,  0,         0,          0 },       // toss
};
const sock_fprog bpf = {
    // Number of code lines
    .len = sizeof(bpf_code) / sizeof(sock_filter),
    .filter = (sock_filter *)bpf_code,
};

sockBase::sockBase() : m_fd(-1), m_isInit(false) {}
sockBase::~sockBase()
{
    this->close();
}
void sockBase::close()
{
    if(m_fd >= 0) {
        ::close(m_fd);
        m_fd = -1;
    }
}
bool sockBase::sendReply(ssize_t cnt, size_t len)
{
    if(cnt < 0) {
        perror("send");
        return false;
    }
    if(cnt != (ssize_t)len) {
        fprintf(stderr, "send %zd instead of %zu\n", cnt, len);
        return false;
    }
    return true;
}
bool sockBase::send(message &msg)
{
    size_t size = msg.getSendBufSize();
    const void *buf = msg.getSendBuf();
    if(size > 0 && buf != nullptr)
        return send(buf, size);
    return false;
}
bool sockBase::poll(uint64_t timeout_ms)
{
    timeval to, *pto;
    if(timeout_ms > 0) {
        to = {
            .tv_sec = (long)(timeout_ms / 1000),
            .tv_usec = (long)(timeout_ms % 1000) * 1000
        };
        pto = &to;
    } else
        pto = nullptr;
    fd_set rs;
    FD_ZERO(&rs);
    FD_SET(m_fd, &rs);
    int ret = select(m_fd + 1, &rs, nullptr, nullptr, pto);
    if(ret > 0 && FD_ISSET(m_fd, &rs))
        return true;
    return false;
}
bool sockBase::tpoll(uint64_t &timeout_ms)
{
    timeval start;
    if(timeout_ms > 0)
        gettimeofday(&start, NULL);
    bool ret = poll(timeout_ms);
    if(timeout_ms > 0) {
        timeval now, left;
        gettimeofday(&now, NULL);
        timersub(&now, &start, &left);
        uint64_t pass = left.tv_sec * 1000 + left.tv_usec / 1000;
        if(timeout_ms > pass)
            timeout_ms -= pass;
        else
            timeout_ms = 0; // No time out!
    }
    return ret;
}
static inline bool testUnix(const std::string &str)
{
    size_t len = str.length();
    if(len < 2 || len > unix_path_max || str.substr(0, 1) != "/")
        return false;
    return true;
}
static inline bool testUnix(const char *str)
{
    if(str == nullptr)
        return false;
    size_t len = strlen(str);
    if(len < 2 || len > unix_path_max || *str != '/')
        return false;
    return true;
}
static inline void setUnixAddr(sockaddr_un &addr, std::string &str)
{
    addr = {0};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, str.c_str(), unix_path_max);
}
static inline void setUnixAddr(sockaddr_un &addr, const char *str)
{
    addr = {0};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, str, unix_path_max);
}
sockUnix::sockUnix()
{
    setUnixAddr(m_peerAddr, m_peer);
}
void sockUnix::close()
{
    sockBase::close();
    if(m_isInit) {
        unlink(m_me.c_str());
        m_isInit = false;
    }
}
bool sockUnix::init()
{
    if(m_isInit || !testUnix(m_me))
        return false;
    sockBase::close();
    m_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if(m_fd < 0) {
        perror("socket");
        return false;
    }
    sockaddr_un addr;
    setUnixAddr(addr, m_me);
    if(bind(m_fd, (sockaddr *)&addr, sizeof(addr)) != 0) {
        perror("bind");
        return false;
    }
    m_isInit = true;
    return true;
}
bool sockUnix::setPeerInternal(const std::string &str)
{
    if(!testUnix(str))
        return false;
    m_peer = str;
    setUnixAddr(m_peerAddr, m_peer);
    return true;
}
bool sockUnix::setPeerInternal(const char *str)
{
    if(!testUnix(str))
        return false;
    m_peer = str;
    setUnixAddr(m_peerAddr, m_peer);
    return true;
}
const std::string &sockUnix::getPeerAddress()
{
    return m_peer;
}
const char *sockUnix::getPeerAddress_c()
{
    return m_peer.c_str();
}
bool sockUnix::setPeerAddress(const std::string &str)
{
    return setPeerInternal(str);
}
bool sockUnix::setPeerAddress(const char *str)
{
    return setPeerInternal(str);
}
bool sockUnix::setPeerAddress(configFile &cfg, const char *section)
{
    return setPeerInternal(cfg.uds_address(section));
}
bool sockUnix::setPeerAddress(configFile &cfg, const std::string &section)
{
    return setPeerInternal(cfg.uds_address(section));
}
const std::string &sockUnix::getSelfAddress()
{
    return m_me;
}
const char *sockUnix::getSelfAddress_c()
{
    return m_me.c_str();
}
bool sockUnix::setSelfAddress(const std::string &str)
{
    // self address can not be changed after init is done
    if(m_isInit || !testUnix(str))
        return false;
    m_me = str;
    return true;
}
bool sockUnix::setSelfAddress(const char *str)
{
    // self address can not be changed after init is done
    if(m_isInit || !testUnix(str))
        return false;
    m_me = str;
    return true;
}
bool sockUnix::setDefSelfAddress(std::string &rootBase, std::string &useDef)
{
    // self address can not be changed after init is done
    if(m_isInit)
        return false;
    std::string new_me;
    auto uid = getuid();
    if(uid) {
        new_me = getHomeDir();
        if(useDef.empty())
            useDef = useDefstr;
        new_me += useDef;
    } else {
        if(rootBase.empty())
            rootBase = rootBasestr;
        new_me = rootBase;
    }
    new_me += std::to_string(getpid());
    return setSelfAddress(new_me);
}
bool sockUnix::setDefSelfAddress(std::string &rootBase)
{
    std::string s1 = rootBase, s2;
    return setDefSelfAddress(s1, s2);
}
bool sockUnix::setDefSelfAddress(const char *rootBase, const char *useDef)
{
    std::string s1, s2;
    if(rootBase != nullptr)
        s1 = rootBase;
    if(useDef != nullptr)
        s2 = useDef;
    return setDefSelfAddress(s1, s2);
}
const std::string &sockUnix::getHomeDir()
{
    auto uid = getuid();
    auto *pwd = getpwuid(uid);
    m_homeDir = pwd->pw_dir;
    return m_homeDir;
}
const char *sockUnix::getHomeDir_c()
{
    return getHomeDir().c_str();
}
bool sockUnix::sendAny(const void *msg, size_t len, sockaddr_un &addr)
{
    ssize_t cnt = sendto(m_fd, msg, len, 0, (sockaddr *)&addr, sizeof(addr));
    return sendReply(cnt, len);
}
bool sockUnix::send(const void *msg, size_t len)
{
    if(!m_isInit || !testUnix(m_peer))
        return false;
    return sendAny(msg, len, m_peerAddr);
}
bool sockUnix::sendTo(const void *msg, size_t len, const char *addrStr)
{
    if(!m_isInit || !testUnix(addrStr))
        return false;
    sockaddr_un addr;
    setUnixAddr(addr, addrStr);
    return sendAny(msg, len, addr);
}
bool sockUnix::sendTo(const void *msg, size_t len, std::string &addrStr)
{
    if(!m_isInit || !testUnix(addrStr))
        return false;
    sockaddr_un addr;
    setUnixAddr(addr, addrStr);
    return sendAny(msg, len, addr);
}
ssize_t sockUnix::rcv(void *buf, size_t bufSize, bool block)
{
    if(!testUnix(m_peer))
        return -1;
    std::string from;
    ssize_t cnt = rcvFrom(buf, bufSize, from, block);
    if(cnt > 0 && from == m_peer)
        return cnt;
    return -1;
}
ssize_t sockUnix::rcvFrom(void *buf, size_t bufSize, std::string &from,
    bool block)
{
    if(!m_isInit)
        return -1;
    sockaddr_un addr;
    socklen_t len = sizeof(addr);
    int flags = 0;
    if(!block)
        flags |= MSG_DONTWAIT;
    ssize_t cnt = recvfrom(m_fd, buf, bufSize, flags, (sockaddr *)&addr, &len);
    if(cnt < 0) {
        perror("recv");
        return -1;
    }
    if(cnt > (ssize_t)bufSize) {
        fprintf(stderr, "rcv %zd more then buffer size %zu\n", cnt, bufSize);
        return -1;
    }
    addr.sun_path[unix_path_max] = 0; // Ensure string is null terminated
    from = addr.sun_path;
    return cnt;
}
bool sockBaseIf::set(ptpIf &ifObj)
{
    m_ifName = ifObj.ifName();
    m_ifIndex = ifObj.ifIndex();
    m_mac = ifObj.mac();
    m_have_if = true;
    return true;
}
bool sockBaseIf::setIf(const std::string &ifName)
{
    if(m_isInit)
        return false;
    ptpIf ifObj;
    if(ifObj.init(ifName))
        return set(ifObj);
    return false;
}
bool sockBaseIf::setIf(const char *ifName)
{
    if(m_isInit)
        return false;
    ptpIf ifObj;
    if(ifObj.init(ifName))
        return set(ifObj);
    return false;
}
bool sockBaseIf::setIf(int ifIndex)
{
    if(m_isInit)
        return false;
    ptpIf ifObj;
    if(ifObj.init(ifIndex))
        return set(ifObj);
    return false;
}
bool sockBaseIf::setIf(ptpIf &ifObj)
{
    if(m_isInit)
        return false;
    if(ifObj.isInit())
        return set(ifObj);
    return false;
}
bool sockBaseIf::setAll(ptpIf &ifObj, configFile &cfg, const char *section)
{
    return setIf(ifObj) && setAllBase(cfg, section) && init();
}
bool sockBaseIf::setAll(ptpIf &ifObj, configFile &cfg,
    const std::string &section)
{
    return setIf(ifObj) && setAllBase(cfg, section) && init();
}
sockIp::sockIp(int domain, const char *mcast, sockaddr *addr, size_t len) :
    m_domain(domain),
    m_udp_ttl(-1),
    m_addr(addr),
    m_addr_len(len),
    m_mcast_str(mcast)
{
}
bool sockIp::setUdpTtl(uint8_t udp_ttl)
{
    if(m_isInit)
        return false;
    m_udp_ttl = udp_ttl;
    return true;
}
bool sockIp::setUdpTtl(configFile &cfg, const char *section)
{
    if(m_isInit)
        return false;
    m_udp_ttl = cfg.udp_ttl(section);
    return true;
}
bool sockIp::setUdpTtl(configFile &cfg, const std::string &section)
{
    if(m_isInit)
        return false;
    m_udp_ttl = cfg.udp_ttl(section);
    return true;
}
bool sockIp::send(const void *msg, size_t len)
{
    if(!m_isInit)
        return false;
    ssize_t cnt = sendto(m_fd, msg, len, 0, m_addr, m_addr_len);
    return sendReply(cnt, len);
}
ssize_t sockIp::rcv(void *buf, size_t bufSize, bool block)
{
    if(!m_isInit)
        return -1;
    int flags = 0;
    if(!block)
        flags |= MSG_DONTWAIT;
    ssize_t cnt = recv(m_fd, buf, bufSize, flags);
    if(cnt < 0) {
        perror("recv");
        return -1;
    }
    if(cnt > (ssize_t)bufSize) {
        fprintf(stderr, "rcv %zd more then buffer size %zu\n", cnt, bufSize);
        return -1;
    }
    return cnt;
}
bool sockIp::init()
{
    if(m_isInit || !m_have_if || m_udp_ttl < 0)
        return false;
    sockBase::close();
    m_fd = socket(m_domain, SOCK_DGRAM, 0/*IPPROTO_UDP*/);
    if(m_fd < 0) {
        perror("socket");
        return false;
    }
    int on = 1;
    if(setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) != 0) {
        perror("setsockopt SO_REUSEADDR failed: %m");
        return false;
    }
    if(bind(m_fd, m_addr, m_addr_len) != 0) {
        perror("bind");
        return false;
    }
    if(setsockopt(m_fd, SOL_SOCKET, SO_BINDTODEVICE, m_ifName.c_str(),
            m_ifName.length()) != 0) {
        perror("BINDTODEVICE");
        return false;
    }
    int ret = inet_pton(m_domain, m_mcast_str, m_mcast);
    if(ret != 1) {
        fprintf(stderr, "inet_pton %d %m\n", ret);
        return false;
    }
    if(!init2())
        return false;
    m_isInit = true;
    return true;
}
sockIp4::sockIp4() : sockIp(AF_INET, ipv4_udp_mc, (sockaddr *) & m_addr4,
        sizeof(m_addr4))
{
    m_addr4 = {0};
    m_addr4.sin_family = m_domain;
    m_addr4.sin_addr.s_addr = hton32(INADDR_ANY);
    m_addr4.sin_port = hton16(udp_port);
}
bool sockIp4::init2()
{
    if(setsockopt(m_fd, IPPROTO_IP, IP_MULTICAST_TTL, &m_udp_ttl,
            sizeof(m_udp_ttl)) != 0) {
        perror("IP_MULTICAST_TTL");
        return false;
    }
    ip_mreqn req = {0};
    req.imr_multiaddr = *(in_addr *)m_mcast;
    req.imr_ifindex = m_ifIndex;
    if(setsockopt(m_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &req, sizeof(req)) != 0) {
        perror("IP_ADD_MEMBERSHIP");
        return false;
    }
    int off = 0;
    if(setsockopt(m_fd, IPPROTO_IP, IP_MULTICAST_LOOP, &off, sizeof(off)) != 0) {
        perror("IP_MULTICAST_LOOP");
        return false;
    }
    req = {0};
    req.imr_ifindex = m_ifIndex;
    if(setsockopt(m_fd, IPPROTO_IP, IP_MULTICAST_IF, &req, sizeof(req)) != 0) {
        perror("IP_MULTICAST_IF");
        return false;
    }
    /* For sending */
    m_addr4.sin_addr = *(in_addr *)m_mcast;
    return true;
}
bool sockIp4::setAllBase(configFile &cfg, const char *section)
{
    return setUdpTtl(cfg, section);
}
bool sockIp4::setAllBase(configFile &cfg, const std::string &section)
{
    return setUdpTtl(cfg, section);
}
sockIp6::sockIp6() : sockIp(AF_INET6, ipv6_udp_mc, (sockaddr *) & m_addr6,
        sizeof(m_addr6)),
    m_udp6_scope(-1)
{
    m_addr6 = {0};
    m_addr6.sin6_family = m_domain;
    m_addr6.sin6_addr = IN6ADDR_ANY_INIT;
    m_addr6.sin6_port = hton16(udp_port);
}
bool sockIp6::init2()
{
    if(m_udp6_scope < 0)
        return false;
    if(setsockopt(m_fd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &m_udp_ttl,
            sizeof(m_udp_ttl)) != 0) {
        perror("IPV6_MULTICAST_HOPS");
        return false;
    }
    m_mcast[1] = m_udp6_scope;
    ipv6_mreq req = {0};
    req.ipv6mr_multiaddr = *(in6_addr *)m_mcast;
    req.ipv6mr_interface = m_ifIndex;
    if(setsockopt(m_fd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &req,
            sizeof(req)) != 0) {
        perror("IPV6_ADD_MEMBERSHIP");
        return false;
    }
    int off = 0;
    if(setsockopt(m_fd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &off,
            sizeof(off)) != 0) {
        perror("IPV6_MULTICAST_LOOP");
        return false;
    }
    req = {0};
    req.ipv6mr_interface = m_ifIndex;
    if(setsockopt(m_fd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &req, sizeof(req)) != 0) {
        perror("IPV6_MULTICAST_IF");
        return false;
    }
    /* For sending */
    m_addr6.sin6_addr = *(in6_addr *)m_mcast;
    if(m_udp6_scope == 2)  // Local link
        m_addr6.sin6_scope_id = m_ifIndex;
    return true;
}
bool sockIp6::setScope(uint8_t udp6_scope)
{
    if(m_isInit || udp6_scope < 0 || udp6_scope > 0xf)
        return false;
    m_udp6_scope = udp6_scope;
    return true;
}
bool sockIp6::setScope(configFile &cfg, const char *section)
{
    if(m_isInit)
        return false;
    m_udp6_scope = cfg.udp6_scope(section);
    return true;
}
bool sockIp6::setScope(configFile &cfg, const std::string &section)
{
    if(m_isInit)
        return false;
    m_udp6_scope = cfg.udp6_scope(section);
    return true;
}
bool sockIp6::setAllBase(configFile &cfg, const char *section)
{
    return setUdpTtl(cfg, section) && setScope(cfg, section);
}
bool sockIp6::setAllBase(configFile &cfg, const std::string &section)
{
    return setUdpTtl(cfg, section) && setScope(cfg, section);
}
sockRaw::sockRaw() :
    m_socket_priority(-1),
    m_addr{0},
    m_msg_tx{0},
    m_msg_rx{0},
    m_hdr{0}
{
}
bool sockRaw::setPtpDstMacStr(const std::string &str)
{
    if(m_isInit || str.empty())
        return false;
    auto mac = ptpIf::str2mac(str);
    if(mac.empty())
        return false;
    m_ptp_dst_mac = mac;
    return true;
}
bool sockRaw::setPtpDstMacStr(const char *str)
{
    if(m_isInit || str == nullptr || *str == 0)
        return false;
    auto mac = ptpIf::str2mac(str);
    if(mac.empty())
        return false;
    m_ptp_dst_mac = mac;
    return true;
}
bool sockRaw::setPtpDstMac(const std::string &ptp_dst_mac)
{
    size_t len = ptp_dst_mac.length();
    if(m_isInit || (len != EUI48 && len != EUI64))
        return false;
    m_ptp_dst_mac = ptp_dst_mac;
    return true;
}
bool sockRaw::setPtpDstMac(const uint8_t *ptp_dst_mac, size_t len)
{
    if(m_isInit || (len != EUI48 && len != EUI64))
        return false;
    m_ptp_dst_mac = std::string((const char *)ptp_dst_mac, len);
    return true;
}
bool sockRaw::setPtpDstMac(configFile &cfg, const char *section)
{
    if(m_isInit)
        return false;
    m_ptp_dst_mac = cfg.ptp_dst_mac(section);
    return true;
}
bool sockRaw::setPtpDstMac(configFile &cfg, const std::string &section)
{
    if(m_isInit)
        return false;
    m_ptp_dst_mac = cfg.ptp_dst_mac(section);
    return true;
}
bool sockRaw::setSocketPriority(uint8_t socket_priority)
{
    if(m_isInit || socket_priority < 0 || socket_priority > 15)
        return false;
    m_socket_priority = socket_priority;
    return true;
}
bool sockRaw::setSocketPriority(configFile &cfg, const char *section)
{
    if(m_isInit)
        return false;
    m_socket_priority = cfg.socket_priority(section);
    return true;
}
bool sockRaw::setSocketPriority(configFile &cfg, const std::string &section)
{
    if(m_isInit)
        return false;
    m_socket_priority = cfg.socket_priority(section);
    return true;
}
bool sockRaw::init()
{
    if(m_isInit || !m_have_if || m_ptp_dst_mac.empty() || m_socket_priority < 0)
        return false;
    uint16_t port_all = hton16(ETH_P_ALL);
    uint16_t port_ptp = hton16(ETH_P_1588);
    m_fd = socket(AF_PACKET, SOCK_RAW, port_all);
    if(m_fd < 0) {
        perror("socket");
        return false;
    }
    m_addr.sll_ifindex = m_ifIndex;
    m_addr.sll_family = AF_PACKET;
    m_addr.sll_protocol = port_all;
    if(bind(m_fd, (sockaddr *) &m_addr, sizeof(m_addr))) {
        perror("bind");
        return false;
    }
    if(setsockopt(m_fd, SOL_SOCKET, SO_BINDTODEVICE, m_ifName.c_str(),
            m_ifName.length()) != 0) {
        perror("SO_BINDTODEVICE");
        return false;
    }
    if(setsockopt(m_fd, SOL_SOCKET, SO_PRIORITY, &m_socket_priority,
            sizeof(m_socket_priority)) != 0) {
        perror("SO_PRIORITY");
        return false;
    }
    if(setsockopt(m_fd, SOL_SOCKET, SO_ATTACH_FILTER, &bpf, sizeof(bpf)) != 0) {
        perror("SO_ATTACH_FILTER");
        return false;
    }
    packet_mreq mreq = {0};
    mreq.mr_ifindex = m_ifIndex;
    mreq.mr_type = PACKET_MR_MULTICAST;
    mreq.mr_alen = m_ptp_dst_mac.length();
    memcpy(mreq.mr_address, m_ptp_dst_mac.c_str(), m_ptp_dst_mac.length());
    if(setsockopt(m_fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mreq,
            sizeof(mreq)) != 0) {
        perror("PACKET_ADD_MEMBERSHIP ptp_dst_mac");
        return false;
    }
    // TX
    m_addr.sll_halen = m_ptp_dst_mac.length();
    memcpy(m_addr.sll_addr, m_ptp_dst_mac.c_str(), m_ptp_dst_mac.length());
    m_msg_tx.msg_name = &m_addr;
    m_msg_tx.msg_namelen = sizeof(m_addr);
    m_msg_tx.msg_iov = m_iov_tx;
    m_msg_tx.msg_iovlen = sizeof(m_iov_tx) / sizeof(iovec);
    m_hdr.h_proto = port_ptp;
    memcpy(m_hdr.h_dest, m_ptp_dst_mac.c_str(), m_ptp_dst_mac.length());
    memcpy(m_hdr.h_source, m_mac.c_str(), m_mac.length());
    m_iov_tx[0].iov_base = &m_hdr;
    m_iov_tx[0].iov_len = sizeof(m_hdr);
    // RX
    m_msg_rx.msg_iov = m_iov_rx;
    m_msg_rx.msg_iovlen = sizeof(m_iov_rx) / sizeof(iovec);
    m_isInit = true;
    return true;
}
bool sockRaw::send(const void *msg, size_t len)
{
    if(!m_isInit)
        return false;
    m_iov_tx[1].iov_base = (void *)msg;
    m_iov_tx[1].iov_len = len;
    ssize_t cnt = sendmsg(m_fd, &m_msg_tx, 0);
    return sendReply(cnt, len + sizeof(m_hdr));
}
ssize_t sockRaw::rcv(void *buf, size_t bufSize, bool block)
{
    if(!m_isInit)
        return -1;
    int flags = 0;
    if(!block)
        flags |= MSG_DONTWAIT;
    m_iov_rx[0].iov_base = m_rx_buf;
    m_iov_rx[0].iov_len = sizeof(m_rx_buf);
    m_iov_rx[1].iov_base = buf;
    m_iov_rx[1].iov_len = bufSize;
    ssize_t cnt = recvmsg(m_fd, &m_msg_rx, flags);
    if(cnt < 0) {
        perror("recvmsg");
        return -1;
    }
    if(cnt > (ssize_t)(bufSize + sizeof(m_rx_buf))) {
        fprintf(stderr, "rcv %zd more then buffer size %zu\n", cnt,
            bufSize + sizeof(m_rx_buf));
        return -1;
    }
    return cnt;
}
bool sockRaw::setAllBase(configFile &cfg, const char *section)
{
    return setPtpDstMac(cfg, section) && setSocketPriority(cfg, section);
}
bool sockRaw::setAllBase(configFile &cfg, const std::string &section)
{
    return setPtpDstMac(cfg, section) && setSocketPriority(cfg, section);
}
