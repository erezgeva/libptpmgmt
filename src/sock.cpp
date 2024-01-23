/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Use unix socket to communicate with ptp4l
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 */

#include <pwd.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <linux/filter.h>
#include "comp.h"
#include "sock.h"
#include "timeCvrt.h"

__PTPMGMT_NAMESPACE_BEGIN

// Values from IEEE 1588 standard
const uint16_t udp_port = 320;
const char *ipv4_udp_mc = "224.0.1.129";
const char *ipv6_udp_mc = "ff0e::181";

const char *useDefstrPre = "/var/run/user/"; // System provide per user
const char *useDefstrPost = "/pmc.";
const char *useDefstr = "/.pmc."; // relative to home directory
const char *rootBasestr = "/var/run/pmc."; // Follow LinuxPTP
const size_t unix_path_max = sizeof(((sockaddr_un *)nullptr)->sun_path) - 1;

// Berkeley Packet Filter code
// The code run on network order (big endian).
// 0x30 Load bottom (2 last bytes of word)
#if 0
const uint16_t OP_LDB = BPF_LD  | BPF_B   | BPF_ABS;
#endif
// 0x28 Load high (2 first bytes)
const uint16_t OP_LDH = BPF_LD  | BPF_H   | BPF_ABS;
// 0x20 Load word (4 bytes)
#if 0
const uint16_t OP_LDW = BPF_LD  | BPF_W   | BPF_ABS;
#endif
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

static inline bool ensureDir(const char *name)
{
    // Verify name is a folder
    DIR *dir = opendir(name);
    if(dir == nullptr) {
        PTPMGMT_ERROR_P("opendir");
        return false;
    }
    closedir(dir);
    // Ensure we have full access rights
    if(access(name, R_OK | W_OK | X_OK) != 0) {
        PTPMGMT_ERROR_P("access");
        return false;
    }
    PTPMGMT_ERROR_CLR;
    return true;
}

void SockBase::closeBase()
{
    if(m_fd >= 0) {
        ::close(m_fd);
        m_fd = -1;
    }
    closeChild();
    m_isInit = false;
}
bool SockBase::sendReply(ssize_t cnt, size_t len) const
{
    if(cnt < 0) {
        PTPMGMT_ERROR_P("send");
        return false;
    }
    if(cnt != (ssize_t)len) {
        PTPMGMT_ERROR("send %zd instead of %zu", cnt, len);
        return false;
    }
    PTPMGMT_ERROR_CLR;
    return true;
}
bool SockBase::poll(uint64_t timeout_ms) const
{
    timeval to, *pto;
    if(timeout_ms > 0) {
        to = {
            .tv_sec = (long)(timeout_ms / MSEC_PER_SEC),
            .tv_usec = (long)(timeout_ms % MSEC_PER_SEC) * USEC_PER_MSEC
        };
        pto = &to;
    } else
        pto = nullptr;
    fd_set rs;
    FD_ZERO(&rs);
    FD_SET(m_fd, &rs);
    int ret = select(m_fd + 1, &rs, nullptr, nullptr, pto);
    if(ret > 0 && FD_ISSET(m_fd, &rs)) {
        PTPMGMT_ERROR_CLR;
        return true;
    }
    if(ret == -1)
        PTPMGMT_ERROR_P("selec");
    else
        PTPMGMT_ERROR("select fails with %d", ret);
    return false;
}
bool SockBase::tpoll(uint64_t &timeout_ms) const
{
    bool have_clock = timeout_ms > 0;
    timespec start;
    if(have_clock)
        have_clock = clock_gettime(CLOCK_REALTIME, &start) == 0;
    bool ret = poll(timeout_ms);
    if(have_clock) {
        timespec now;
        if(clock_gettime(CLOCK_REALTIME, &now) == 0) {
            Timestamp_t s(start), n(now);
            n -= s;
            uint64_t pass = n.toNanoseconds() / NSEC_PER_MSEC;
            if(timeout_ms > pass)
                timeout_ms -= pass;
            else
                timeout_ms = 0; // No time out!
        }
    }
    return ret;
}
static inline bool testUnix(const std::string &str, size_t extra = 0)
{
    size_t len = str.length();
    if(len == 0 || len > unix_path_max - extra) {
        PTPMGMT_ERROR("Wrong unix file name: %s", str.c_str());
        return false;
    }
    PTPMGMT_ERROR_CLR;
    return true;
}
void SockUnix::setUnixAddr(sockaddr_un &addr, const std::string &str)
{
    addr = {0};
    addr.sun_family = AF_UNIX;
    memcpy(addr.sun_path, str.c_str(), std::min(str.size(), unix_path_max));
}
void SockUnix::closeChild()
{
    if(m_isInit && !isSelfAddressAbstract())
        unlink(m_me.c_str());
}
bool SockUnix::initBase()
{
    if(m_isInit) {
        PTPMGMT_ERROR("Socket is already initialized");
        return false;
    }
    if(!testUnix(m_me))
        return false;
    closeBase();
    m_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if(m_fd < 0) {
        PTPMGMT_ERROR_P("socket");
        return false;
    }
    sockaddr_un addr;
    setUnixAddr(addr, m_me);
    if(bind(m_fd, (sockaddr *)&addr, sizeof addr) != 0) {
        PTPMGMT_ERROR_P("bind");
        return false;
    }
    m_isInit = true;
    PTPMGMT_ERROR_CLR;
    return true;
}
bool SockUnix::setPeerInternal(const std::string &str, bool useAbstract)
{
    if(!testUnix(str, useAbstract ? 1 : 0))
        return false;
    if(useAbstract)
        m_peer = std::string(1, '\0') + str;
    else
        m_peer = str;
    setUnixAddr(m_peerAddr, m_peer);
    PTPMGMT_ERROR_CLR;
    return true;
}
bool SockUnix::setSelfAddress(const std::string &str, bool useAbstract)
{
    if(m_isInit) {
        PTPMGMT_ERROR("self address can not be changed after initializing is done");
        return false;
    }
    if(!testUnix(str, useAbstract ? 1 : 0))
        return false;
    if(useAbstract)
        m_me = std::string(1, '\0') + str;
    else
        m_me = str;
    PTPMGMT_ERROR_CLR;
    return true;
}
bool SockUnix::setDefSelfAddress(const std::string &rootBase,
    const std::string &useDef)
{
    if(m_isInit) {
        PTPMGMT_ERROR("self address can not be changed after initializing is done");
        return false;
    }
    std::string new_me;
    uid_t uid = getuid();
    if(uid) {
        if(useDef.empty()) {
            new_me = useDefstrPre;
            new_me += std::to_string(uid);
            // Ensure system run folder per user exist
            if(!ensureDir(new_me.c_str())) {
                // If not use on home directory
                new_me = getHomeDir();
                new_me += useDefstr;
            } else
                new_me += useDefstrPost;
        } else {
            if(useDef[0] == '/') {
                // Absolete path
                new_me = useDef;
            } else {
                // relative to home directory
                new_me = getHomeDir();
                new_me += useDef;
            }
        }
    } else {
        if(rootBase.empty())
            new_me = rootBasestr;
        else
            new_me = rootBase;
    }
    new_me += std::to_string(getpid());
    return setSelfAddress(new_me);
}
const std::string &SockUnix::getHomeDir()
{
    passwd *pwd = getpwuid(getuid());
    if(pwd != nullptr)
        m_homeDir = pwd->pw_dir;
    return m_homeDir;
}
const char *SockUnix::getHomeDir_c()
{
    getHomeDir();
    return m_homeDir.c_str();
}
bool SockUnix::sendAny(const void *msg, size_t len,
    const sockaddr_un &addr) const
{
    ssize_t cnt = sendto(m_fd, msg, len, 0, (sockaddr *)&addr, sizeof addr);
    return sendReply(cnt, len);
}
bool SockUnix::sendBase(const void *msg, size_t len)
{
    if(!m_isInit) {
        PTPMGMT_ERROR("Socket is not initialized");
        return false;
    }
    if(!testUnix(m_peer))
        return false;
    return sendAny(msg, len, m_peerAddr);
}
bool SockUnix::sendTo(const void *msg, size_t len,
    const std::string &addrStr, bool useAbstract) const
{
    if(!m_isInit) {
        PTPMGMT_ERROR("Socket is not initialized");
        return false;
    }
    if(!testUnix(addrStr, useAbstract ? 1 : 0))
        return false;
    sockaddr_un addr;
    if(useAbstract)
        setUnixAddr(addr, std::string(1, '\0') + addrStr);
    else
        setUnixAddr(addr, addrStr);
    return sendAny(msg, len, addr);
}
ssize_t SockUnix::rcvBase(void *buf, size_t bufSize, bool block)
{
    if(!testUnix(m_peer))
        return -1;
    std::string from;
    ssize_t cnt = rcvFrom(buf, bufSize, from, block);
    if(cnt < 0)
        return -1;
    if(from != m_peer) {
        PTPMGMT_ERROR("Wrong peer");
        return -1;
    }
    if(cnt == 0) {
        PTPMGMT_ERROR("empty message");
        return -1;
    }
    return cnt;
}
ssize_t SockUnix::rcvFrom(void *buf, size_t bufSize, std::string &from,
    bool block) const
{
    if(!m_isInit) {
        PTPMGMT_ERROR("Socket is not initialized");
        return -1;
    }
    sockaddr_un addr;
    socklen_t len = sizeof addr;
    int flags = 0;
    if(!block)
        flags |= MSG_DONTWAIT;
    ssize_t cnt = recvfrom(m_fd, buf, bufSize, flags, (sockaddr *)&addr, &len);
    if(cnt < 0) {
        PTPMGMT_ERROR_P("recv");
        return -1;
    }
    if(cnt > (ssize_t)bufSize) {
        PTPMGMT_ERROR("rcv %zd more than buffer size %zu", cnt, bufSize);
        return -1;
    }
    addr.sun_path[unix_path_max] = 0; // Ensure string is null terminated
    from = addr.sun_path;
    PTPMGMT_ERROR_CLR;
    return cnt;
}
bool SockBaseIf::setInt(const IfInfo &ifObj)
{
    m_ifName = ifObj.ifName();
    m_ifIndex = ifObj.ifIndex();
    m_mac = ifObj.mac();
    m_have_if = true;
    PTPMGMT_ERROR_CLR;
    return true;
}
bool SockBaseIf::setIfUsingName(const std::string &ifName)
{
    if(m_isInit) {
        PTPMGMT_ERROR("Socket is already initialized");
        return false;
    }
    IfInfo ifObj;
    if(!ifObj.initUsingName(ifName))
        return false;
    return setInt(ifObj);
}
bool SockBaseIf::setIfUsingIndex(int ifIndex)
{
    if(m_isInit) {
        PTPMGMT_ERROR("Socket is already initialized");
        return false;
    }
    IfInfo ifObj;
    if(!ifObj.initUsingIndex(ifIndex))
        return false;
    return setInt(ifObj);
}
bool SockBaseIf::setIf(const IfInfo &ifObj)
{
    if(m_isInit) {
        PTPMGMT_ERROR("Socket is already initialized");
        return false;
    }
    if(!ifObj.isInit())
        return false;
    return setInt(ifObj);
}
SockIp::SockIp(int domain, const char *mcast, sockaddr *addr, size_t len) :
    m_domain(domain),
    m_udp_ttl(-1),
    m_addr(addr),
    m_addr_len(len),
    m_mcast_str(mcast)
{
}
bool SockIp::setUdpTtl(uint8_t udp_ttl)
{
    if(m_isInit) {
        PTPMGMT_ERROR("Can not set ttl after socket is initialized");
        return false;
    }
    m_udp_ttl = udp_ttl;
    PTPMGMT_ERROR_CLR;
    return true;
}
bool SockIp::setUdpTtl(const ConfigFile &cfg, const std::string &section)
{
    if(m_isInit) {
        PTPMGMT_ERROR("Can not set ttl after socket is initialized");
        return false;
    }
    m_udp_ttl = cfg.udp_ttl(section);
    PTPMGMT_ERROR_CLR;
    return true;
}
bool SockIp::sendBase(const void *msg, size_t len)
{
    if(!m_isInit) {
        PTPMGMT_ERROR("Socket is not initialized");
        return false;
    }
    ssize_t cnt = sendto(m_fd, msg, len, 0, m_addr, m_addr_len);
    return sendReply(cnt, len);
}
ssize_t SockIp::rcvBase(void *buf, size_t bufSize, bool block)
{
    if(!m_isInit) {
        PTPMGMT_ERROR("Socket is not initialized");
        return -1;
    }
    int flags = 0;
    if(!block)
        flags |= MSG_DONTWAIT;
    ssize_t cnt = recv(m_fd, buf, bufSize, flags);
    if(cnt < 0) {
        PTPMGMT_ERROR_P("recv");
        return -1;
    }
    if(cnt > (ssize_t)bufSize) {
        PTPMGMT_ERROR("rcv %zd more than buffer size %zu", cnt, bufSize);
        return -1;
    }
    PTPMGMT_ERROR_CLR;
    return cnt;
}
bool SockIp::initBase()
{
    if(m_isInit) {
        PTPMGMT_ERROR("Socket is already initialized");
        return false;
    }
    if(!m_have_if) {
        PTPMGMT_ERROR("Interface is missing");
        return false;
    }
    if(m_udp_ttl < 0) {
        PTPMGMT_ERROR("Wrong TTL value %d", m_udp_ttl);
        return false;
    }
    closeBase();
    m_fd = socket(m_domain, SOCK_DGRAM, 0/*IPPROTO_UDP*/);
    if(m_fd < 0) {
        PTPMGMT_ERROR_P("socket");
        return false;
    }
    int on = 1;
    if(setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on) != 0) {
        PTPMGMT_ERROR_P("setsockopt SO_REUSEADDR failed: %m");
        return false;
    }
    if(bind(m_fd, m_addr, m_addr_len) != 0) {
        PTPMGMT_ERROR_P("bind");
        return false;
    }
    if(setsockopt(m_fd, SOL_SOCKET, SO_BINDTODEVICE, m_ifName.c_str(),
            m_ifName.length()) != 0) {
        PTPMGMT_ERROR_P("BINDTODEVICE");
        return false;
    }
    if(!m_mcast.fromIp(m_mcast_str, m_domain)) {
        PTPMGMT_ERROR("multicast %s", m_mcast_str);
        return false;
    }
    if(!initIp())
        return false;
    m_isInit = true;
    PTPMGMT_ERROR_CLR;
    return true;
}
SockIp4::SockIp4() : SockIp(AF_INET, ipv4_udp_mc, (sockaddr *) & m_addr4,
        sizeof m_addr4)
{
    m_addr4 = {0};
    m_addr4.sin_family = m_domain;
    m_addr4.sin_addr.s_addr = cpu_to_net32(INADDR_ANY);
    m_addr4.sin_port = cpu_to_net16(udp_port);
}
bool SockIp4::initIp()
{
    if(setsockopt(m_fd, IPPROTO_IP, IP_MULTICAST_TTL, &m_udp_ttl,
            sizeof m_udp_ttl) != 0) {
        PTPMGMT_ERROR_P("IP_MULTICAST_TTL");
        return false;
    }
    ip_mreqn req;
    memset(&req, 0, sizeof req);
    req.imr_multiaddr = *(in_addr *)m_mcast.get();
    req.imr_ifindex = m_ifIndex;
    if(setsockopt(m_fd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &req, sizeof req) != 0) {
        PTPMGMT_ERROR_P("IP_ADD_MEMBERSHIP");
        return false;
    }
    int off = 0;
    if(setsockopt(m_fd, IPPROTO_IP, IP_MULTICAST_LOOP, &off, sizeof off) != 0) {
        PTPMGMT_ERROR_P("IP_MULTICAST_LOOP");
        return false;
    }
    memset(&req, 0, sizeof req);
    req.imr_ifindex = m_ifIndex;
    if(setsockopt(m_fd, IPPROTO_IP, IP_MULTICAST_IF, &req, sizeof req) != 0) {
        PTPMGMT_ERROR_P("IP_MULTICAST_IF");
        return false;
    }
    // For sending
    m_addr4.sin_addr = *(in_addr *)m_mcast.get();
    PTPMGMT_ERROR_CLR;
    return true;
}
bool SockIp4::setAllBase(const ConfigFile &cfg, const std::string &section)
{
    return setUdpTtl(cfg, section);
}
SockIp6::SockIp6() : SockIp(AF_INET6, ipv6_udp_mc, (sockaddr *) & m_addr6,
        sizeof m_addr6), m_udp6_scope(-1)
{
    m_addr6 = {0};
    m_addr6.sin6_family = m_domain;
    m_addr6.sin6_addr = IN6ADDR_ANY_INIT;
    m_addr6.sin6_port = cpu_to_net16(udp_port);
}
bool SockIp6::initIp()
{
    if(m_udp6_scope < 0) {
        PTPMGMT_ERROR("Wrong scope value %d", m_udp6_scope);
        return false;
    }
    if(setsockopt(m_fd, IPPROTO_IPV6, IPV6_MULTICAST_HOPS, &m_udp_ttl,
            sizeof m_udp_ttl) != 0) {
        PTPMGMT_ERROR_P("IPV6_MULTICAST_HOPS");
        return false;
    }
    m_mcast.setBin(1, m_udp6_scope);
    ipv6_mreq req;
    memset(&req, 0, sizeof req);
    req.ipv6mr_multiaddr = *(in6_addr *)m_mcast.get();
    req.ipv6mr_interface = m_ifIndex;
    if(setsockopt(m_fd, IPPROTO_IPV6, IPV6_ADD_MEMBERSHIP, &req, sizeof req) != 0) {
        PTPMGMT_ERROR_P("IPV6_ADD_MEMBERSHIP");
        return false;
    }
    int off = 0;
    if(setsockopt(m_fd, IPPROTO_IPV6, IPV6_MULTICAST_LOOP, &off, sizeof off) != 0) {
        PTPMGMT_ERROR_P("IPV6_MULTICAST_LOOP");
        return false;
    }
    memset(&req, 0, sizeof req);
    req.ipv6mr_interface = m_ifIndex;
    if(setsockopt(m_fd, IPPROTO_IPV6, IPV6_MULTICAST_IF, &req, sizeof req) != 0) {
        PTPMGMT_ERROR_P("IPV6_MULTICAST_IF");
        return false;
    }
    // For sending
    m_addr6.sin6_addr = *(in6_addr *)m_mcast.get();
    if(m_udp6_scope == 2) // Local link
        m_addr6.sin6_scope_id = m_ifIndex;
    PTPMGMT_ERROR_CLR;
    return true;
}
bool SockIp6::setScope(uint8_t udp6_scope)
{
    if(m_isInit) {
        PTPMGMT_ERROR("Socket is already initialized");
        return false;
    }
    if(udp6_scope < 0 || udp6_scope > 0xf) {
        PTPMGMT_ERROR("scope out of range 0x%x", udp6_scope);
        return false;
    }
    m_udp6_scope = udp6_scope;
    PTPMGMT_ERROR_CLR;
    return true;
}
bool SockIp6::setScope(const ConfigFile &cfg, const std::string &section)
{
    if(m_isInit) {
        PTPMGMT_ERROR("Socket is already initialized");
        return false;
    }
    m_udp6_scope = cfg.udp6_scope(section);
    PTPMGMT_ERROR_CLR;
    return true;
}
bool SockIp6::setAllBase(const ConfigFile &cfg, const std::string &section)
{
    return setUdpTtl(cfg, section) && setScope(cfg, section);
}
SockRaw::SockRaw() :
    m_socket_priority(-1),
    m_addr{0},
    m_msg_tx{0},
    m_msg_rx{0},
    m_hdr{{0}}
{
}
bool SockRaw::setPtpDstMacStr(const std::string &str)
{
    if(m_isInit) {
        PTPMGMT_ERROR("Socket is already initialized");
        return false;
    }
    if(str.empty()) {
        PTPMGMT_ERROR("Missing MAC address");
        return false;
    }
    Binary mac;
    if(!mac.fromMac(str)) {
        PTPMGMT_ERROR("Wrong MAC address '%s'", str.c_str());
        return false;
    }
    m_ptp_dst_mac = mac;
    PTPMGMT_ERROR_CLR;
    return true;
}
bool SockRaw::setPtpDstMac(const Binary &mac)
{
    if(m_isInit) {
        PTPMGMT_ERROR("Socket is already initialized");
        return false;
    }
    if(!mac.isMacLen()) {
        PTPMGMT_ERROR("Wrong MAC length %zu", mac.length());
        return false;
    }
    m_ptp_dst_mac = mac;
    PTPMGMT_ERROR_CLR;
    return true;
}
bool SockRaw::setPtpDstMac(const void *mac, size_t len)
{
    return setPtpDstMac(Binary(mac, len));
}
bool SockRaw::setPtpDstMac(const ConfigFile &cfg, const std::string &section)
{
    if(m_isInit) {
        PTPMGMT_ERROR("Socket is already initialized");
        return false;
    }
    m_ptp_dst_mac = cfg.ptp_dst_mac(section);
    PTPMGMT_ERROR_CLR;
    return true;
}
bool SockRaw::setSocketPriority(uint8_t socket_priority)
{
    if(m_isInit) {
        PTPMGMT_ERROR("Socket is already initialized");
        return false;
    }
    if(socket_priority < 0 || socket_priority > 15) {
        PTPMGMT_ERROR("Socket priority out of range 0x%x", socket_priority);
        return false;
    }
    m_socket_priority = socket_priority;
    PTPMGMT_ERROR_CLR;
    return true;
}
bool SockRaw::setSocketPriority(const ConfigFile &cfg,
    const std::string &section)
{
    if(m_isInit) {
        PTPMGMT_ERROR("Socket is already initialized");
        return false;
    }
    m_socket_priority = cfg.socket_priority(section);
    PTPMGMT_ERROR_CLR;
    return true;
}
bool SockRaw::initBase()
{
    if(m_isInit) {
        PTPMGMT_ERROR("Socket is already initialized");
        return false;
    }
    if(!m_have_if) {
        PTPMGMT_ERROR("Interface is missing");
        return false;
    }
    if(m_ptp_dst_mac.empty()) {
        PTPMGMT_ERROR("Mac address is missing");
        return false;
    }
    if(m_socket_priority < 0) {
        PTPMGMT_ERROR("Socket priority is missing");
        return false;
    }
    closeBase();
    uint16_t port_all = cpu_to_net16(ETH_P_ALL);
    uint16_t port_ptp = cpu_to_net16(ETH_P_1588);
    m_fd = socket(AF_PACKET, SOCK_RAW, port_all);
    if(m_fd < 0) {
        PTPMGMT_ERROR_P("socket");
        return false;
    }
    m_addr.sll_ifindex = m_ifIndex;
    m_addr.sll_family = AF_PACKET;
    m_addr.sll_protocol = port_all;
    if(bind(m_fd, (sockaddr *) &m_addr, sizeof m_addr)) {
        PTPMGMT_ERROR_P("bind");
        return false;
    }
    if(setsockopt(m_fd, SOL_SOCKET, SO_BINDTODEVICE, m_ifName.c_str(),
            m_ifName.length()) != 0) {
        PTPMGMT_ERROR_P("SO_BINDTODEVICE");
        return false;
    }
    if(setsockopt(m_fd, SOL_SOCKET, SO_PRIORITY, &m_socket_priority,
            sizeof m_socket_priority) != 0) {
        PTPMGMT_ERROR_P("SO_PRIORITY");
        return false;
    }
    if(setsockopt(m_fd, SOL_SOCKET, SO_ATTACH_FILTER, &bpf, sizeof bpf) != 0) {
        PTPMGMT_ERROR_P("SO_ATTACH_FILTER");
        return false;
    }
    packet_mreq mreq = {0};
    mreq.mr_ifindex = m_ifIndex;
    mreq.mr_type = PACKET_MR_MULTICAST;
    mreq.mr_alen = m_ptp_dst_mac.length();
    m_ptp_dst_mac.copy(mreq.mr_address);
    if(setsockopt(m_fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mreq,
            sizeof mreq) != 0) {
        PTPMGMT_ERROR_P("PACKET_ADD_MEMBERSHIP ptp_dst_mac");
        return false;
    }
    // TX
    m_addr.sll_halen = m_ptp_dst_mac.length();
    m_ptp_dst_mac.copy(m_addr.sll_addr);
    m_msg_tx.msg_name = &m_addr;
    m_msg_tx.msg_namelen = sizeof m_addr;
    m_msg_tx.msg_iov = m_iov_tx;
    m_msg_tx.msg_iovlen = sizeof(m_iov_tx) / sizeof(iovec);
    m_hdr.h_proto = port_ptp;
    m_ptp_dst_mac.copy(m_hdr.h_dest);
    m_mac.copy(m_hdr.h_source);
    m_iov_tx[0].iov_base = &m_hdr;
    m_iov_tx[0].iov_len = sizeof m_hdr;
    // RX
    m_msg_rx.msg_iov = m_iov_rx;
    m_msg_rx.msg_iovlen = sizeof(m_iov_rx) / sizeof(iovec);
    m_isInit = true;
    PTPMGMT_ERROR_CLR;
    return true;
}
bool SockRaw::sendBase(const void *msg, size_t len)
{
    if(!m_isInit) {
        PTPMGMT_ERROR("Socket is not initialized");
        return false;
    }
    m_iov_tx[1].iov_base = (void *)msg;
    m_iov_tx[1].iov_len = len;
    ssize_t cnt = sendmsg(m_fd, &m_msg_tx, 0);
    return sendReply(cnt, len + sizeof m_hdr);
}
ssize_t SockRaw::rcvBase(void *buf, size_t bufSize, bool block)
{
    if(!m_isInit) {
        PTPMGMT_ERROR("Socket is not initialized");
        return -1;
    }
    int flags = 0;
    if(!block)
        flags |= MSG_DONTWAIT;
    m_iov_rx[0].iov_base = m_rx_buf;
    m_iov_rx[0].iov_len = sizeof m_rx_buf;
    m_iov_rx[1].iov_base = buf;
    m_iov_rx[1].iov_len = bufSize;
    ssize_t cnt = recvmsg(m_fd, &m_msg_rx, flags);
    if(cnt < 0) {
        PTPMGMT_ERROR_P("recvmsg");
        return -1;
    }
    if(cnt < (ssize_t)(sizeof m_rx_buf)) {
        PTPMGMT_ERROR("rcv %zu less than Ethernet header", cnt);
        return -1;
    }
    if(cnt > (ssize_t)(bufSize + sizeof m_rx_buf)) {
        PTPMGMT_ERROR("rcv %zd more than buffer size %zu", cnt,
            bufSize + sizeof m_rx_buf);
        return -1;
    }
    PTPMGMT_ERROR_CLR;
    return cnt - sizeof m_rx_buf;
}
bool SockRaw::setAllBase(const ConfigFile &cfg, const std::string &section)
{
    return setPtpDstMac(cfg, section) && setSocketPriority(cfg, section);
}

__PTPMGMT_NAMESPACE_END

static const char ptpm_empty_str[] = "";
__PTPMGMT_NAMESPACE_USE;

extern "C" {

#include "c/sock.h"

#define C2CPP_valid(cls)\
    if(sk == nullptr || sk->_this == nullptr)\
        return nullptr;\
    return (cls *)(sk->_this)
    static inline SockBase *valid_csk(const_ptpmgmt_sk sk)
    {
        C2CPP_valid(SockBase);
    }
    static inline SockBase *valid_sk(ptpmgmt_sk sk)
    {
        C2CPP_valid(SockBase);
    }
    static inline SockUnix *valid_cusk(const_ptpmgmt_sk sk)
    {
        C2CPP_valid(SockUnix);
    }
    static inline SockUnix *valid_usk(ptpmgmt_sk sk)
    {
        C2CPP_valid(SockUnix);
    }
    static inline SockBaseIf *valid_isk(ptpmgmt_sk sk)
    {
        C2CPP_valid(SockBaseIf);
    }
    static inline SockIp *valid_ipsk(ptpmgmt_sk sk)
    {
        C2CPP_valid(SockIp);
    }
    static inline SockIp6 *valid_ip6sk(ptpmgmt_sk sk)
    {
        C2CPP_valid(SockIp6);
    }
    static inline SockRaw *valid_rsk(ptpmgmt_sk sk)
    {
        C2CPP_valid(SockRaw);
    }
    static void ptpmgmt_sk_free(ptpmgmt_sk sk)
    {
        SockBase *s = valid_sk(sk);
        if(s != nullptr) {
            delete s;
            free(sk);
        }
    }
    static void ptpmgmt_sk_free_wrap(ptpmgmt_sk sk)
    {
    }
    static void ptpmgmt_sk_close(ptpmgmt_sk sk)
    {
        SockBase *s = valid_sk(sk);
        if(s != nullptr)
            s->close();
    }
    static bool ptpmgmt_sk_init(ptpmgmt_sk sk)
    {
        SockBase *s = valid_sk(sk);
        if(s != nullptr)
            return s->init();
        return false;
    }
    static bool ptpmgmt_sk_send(ptpmgmt_sk sk, const void *msg, size_t len)
    {
        SockBase *s = valid_sk(sk);
        if(s != nullptr)
            return s->send(msg, len);
        return false;
    }
    static ssize_t ptpmgmt_sk_rcv(ptpmgmt_sk sk, void *buf, size_t bufSize,
        bool block)
    {
        SockBase *s = valid_sk(sk);
        if(s != nullptr)
            return s->rcv(buf, bufSize, block);
        return false;
    }
    static int ptpmgmt_sk_getFd(const_ptpmgmt_sk sk)
    {
        SockBase *s = valid_csk(sk);
        if(s != nullptr)
            return s->getFd();
        return -1;
    }
    static bool ptpmgmt_sk_poll(ptpmgmt_sk sk, uint64_t timeout_ms)
    {
        SockBase *s = valid_sk(sk);
        if(s != nullptr)
            return s->poll(timeout_ms);
        return false;
    }
    static bool ptpmgmt_sk_tpoll(ptpmgmt_sk sk, uint64_t *timeout_ms)
    {
        SockBase *s = valid_sk(sk);
        if(s != nullptr) {
            if(timeout_ms != nullptr)
                return s->tpoll(*timeout_ms);
            return s->poll(0);
        }
        return false;
    }
    static const char *non_ptpmgmt_sk_getPeerAddress(const_ptpmgmt_sk)
    {
        return nullptr;
    }
    static const char *ptpmgmt_sk_getPeerAddress(const_ptpmgmt_sk sk)
    {
        SockUnix *s = valid_cusk(sk);
        if(s != nullptr)
            return s->getPeerAddress_c();
        return nullptr;
    }
    static bool non_ptpmgmt_sk_isPeerAddressAbstract(const_ptpmgmt_sk)
    {
        return false;
    }
    static bool ptpmgmt_sk_isPeerAddressAbstract(const_ptpmgmt_sk sk)
    {
        SockUnix *s = valid_cusk(sk);
        if(s != nullptr)
            return s->isPeerAddressAbstract();
        return false;
    }
    static bool non_ptpmgmt_sk_setPeerAddress(ptpmgmt_sk, const char *)
    {
        return false;
    }
    static bool ptpmgmt_sk_setPeerAddress(ptpmgmt_sk sk, const char *str)
    {
        SockUnix *s = valid_usk(sk);
        if(s != nullptr && str != nullptr)
            return s->setPeerAddress(str);
        return false;
    }
    static bool non_ptpmgmt_sk_setPeerAddressA(ptpmgmt_sk, const char *)
    {
        return false;
    }
    static bool ptpmgmt_sk_setPeerAddressA(ptpmgmt_sk sk, const char *str)
    {
        SockUnix *s = valid_usk(sk);
        if(s != nullptr && str != nullptr)
            return s->setPeerAddress(str, true);
        return false;
    }
#define C2CPP_func(func)\
    if (s != nullptr && cfg != nullptr && cfg->_this != nullptr) {\
        if (section == nullptr)\
            return s->func(*(const ConfigFile *)cfg->_this);\
        return s->func(*(const ConfigFile *)cfg->_this, section);\
    }\
    return false
#define C2CPP_func2(func)\
    SockBaseIf *s = valid_isk(sk);\
    if (s != nullptr && ifObj != nullptr && ifObj->_this != nullptr\
        && cfg != nullptr && cfg->_this != nullptr) {\
        if (section == nullptr)\
            return s->func(*(const IfInfo*)ifObj->_this,\
                    *(const ConfigFile *)cfg->_this);\
        return s->func(*(const IfInfo*)ifObj->_this,\
                *(const ConfigFile *)cfg->_this, section);\
    }\
    return false
    static bool non_ptpmgmt_sk_setPeerAddressCfg(ptpmgmt_sk, const_ptpmgmt_cfg,
        const char *)
    {
        return false;
    }
    static bool ptpmgmt_sk_setPeerAddressCfg(ptpmgmt_sk sk, const_ptpmgmt_cfg cfg,
        const char *section)
    {
        SockUnix *s = valid_usk(sk);
        C2CPP_func(setPeerAddress);
    }
    static const char *non_ptpmgmt_sk_getSelfAddress(const_ptpmgmt_sk)
    {
        return nullptr;
    }
    static const char *ptpmgmt_sk_getSelfAddress(const_ptpmgmt_sk sk)
    {
        SockUnix *s = valid_cusk(sk);
        if(s != nullptr)
            return s->getSelfAddress_c();
        return nullptr;
    }
    static bool non_ptpmgmt_sk_isSelfAddressAbstract(const_ptpmgmt_sk)
    {
        return false;
    }
    static bool ptpmgmt_sk_isSelfAddressAbstract(const_ptpmgmt_sk sk)
    {
        SockUnix *s = valid_cusk(sk);
        if(s != nullptr)
            return s->isSelfAddressAbstract();
        return false;
    }
    static bool non_ptpmgmt_sk_setSelfAddress(ptpmgmt_sk, const char *)
    {
        return false;
    }
    static bool ptpmgmt_sk_setSelfAddress(ptpmgmt_sk sk, const char *str)
    {
        SockUnix *s = valid_usk(sk);
        if(s != nullptr && str != nullptr)
            return s->setSelfAddress(str);
        return false;
    }
    static bool non_ptpmgmt_sk_setSelfAddressA(ptpmgmt_sk, const char *)
    {
        return false;
    }
    static bool ptpmgmt_sk_setSelfAddressA(ptpmgmt_sk sk, const char *str)
    {
        SockUnix *s = valid_usk(sk);
        if(s != nullptr && str != nullptr)
            return s->setSelfAddress(str, true);
        return false;
    }
    static bool non_ptpmgmt_sk_setDefSelfAddress(ptpmgmt_sk, const char *,
        const char *)
    {
        return false;
    }
    static bool ptpmgmt_sk_setDefSelfAddress(ptpmgmt_sk sk, const char *rootBase,
        const char *useDef)
    {
        SockUnix *s = valid_usk(sk);
        if(s != nullptr) {
            const char *r = rootBase == nullptr ? ptpm_empty_str : rootBase;
            const char *u = useDef == nullptr ? ptpm_empty_str : useDef;
            return s->setDefSelfAddress(r, u);
        }
        return false;
    }
    static const char *non_ptpmgmt_sk_getHomeDir(ptpmgmt_sk)
    {
        return nullptr;
    }
    static const char *ptpmgmt_sk_getHomeDir(ptpmgmt_sk sk)
    {
        SockUnix *s = valid_usk(sk);
        if(s != nullptr)
            return s->getHomeDir_c();
        return nullptr;
    }
    static bool non_ptpmgmt_sk_sendTo(ptpmgmt_sk, const void *, size_t,
        const char *)
    {
        return false;
    }
    static bool ptpmgmt_sk_sendTo(ptpmgmt_sk sk, const void *msg, size_t len,
        const char *addrStr)
    {
        SockUnix *s = valid_usk(sk);
        if(s != nullptr && msg != nullptr && addrStr != nullptr && len > 0)
            return s->sendTo(msg, len, addrStr);
        return false;
    }
    static bool non_ptpmgmt_sk_sendToA(ptpmgmt_sk, const void *, size_t,
        const char *)
    {
        return false;
    }
    static bool ptpmgmt_sk_sendToA(ptpmgmt_sk sk, const void *msg, size_t len,
        const char *addrStr)
    {
        SockUnix *s = valid_usk(sk);
        if(s != nullptr && msg != nullptr && addrStr != nullptr && len > 0)
            return s->sendTo(msg, len, addrStr, true);
        return false;
    }
    static ssize_t non_ptpmgmt_sk_rcvFrom(ptpmgmt_sk, void *, size_t, char *,
        size_t *, bool)
    {
        return false;
    }
    static ssize_t ptpmgmt_sk_rcvFrom(ptpmgmt_sk sk, void *buf, size_t bufSize,
        char *from, size_t *fromSize, bool block)
    {
        SockUnix *s = valid_usk(sk);
        if(s != nullptr && buf != nullptr && bufSize > 0) {
            std::string f;
            ssize_t ret = s->rcvFrom(buf, bufSize, f, block);
            if(from != nullptr && fromSize != nullptr && *fromSize > 1 &&
                !f.empty()) {
                size_t c = std::min(f.length(), *fromSize - 1);
                memcpy(from, f.c_str(), c);
                from[c] = 0;
                *fromSize = c;
            }
            return ret;
        }
        return false;
    }
    static ssize_t non_ptpmgmt_sk_rcvFromA(ptpmgmt_sk, void *, size_t, bool)
    {
        return false;
    }
    static ssize_t ptpmgmt_sk_rcvFromA(ptpmgmt_sk sk, void *buf, size_t bufSize,
        bool block)
    {
        SockUnix *s = valid_usk(sk);
        if(s != nullptr && buf != nullptr && bufSize > 0)
            return s->rcvFrom(buf, bufSize, block);
        return false;
    }
    static const char *non_ptpmgmt_sk_getLastFrom(const_ptpmgmt_sk)
    {
        return nullptr;
    }
    static const char *ptpmgmt_sk_getLastFrom(const_ptpmgmt_sk sk)
    {
        SockUnix *s = valid_cusk(sk);
        if(s != nullptr)
            return s->getLastFrom_c();
        return nullptr;
    }
    static bool non_ptpmgmt_sk_isLastFromAbstract(const_ptpmgmt_sk)
    {
        return false;
    }
    static bool ptpmgmt_sk_isLastFromAbstract(const_ptpmgmt_sk sk)
    {
        SockUnix *s = valid_cusk(sk);
        if(s != nullptr)
            return s->isLastFromAbstract();
        return false;
    }
    static bool non_ptpmgmt_sk_setIfUsingName(ptpmgmt_sk, const char *)
    {
        return false;
    }
    static bool ptpmgmt_sk_setIfUsingName(ptpmgmt_sk sk, const char *ifName)
    {
        SockBaseIf *s = valid_isk(sk);
        if(s != nullptr && ifName != nullptr)
            return s->setIfUsingName(ifName);
        return false;
    }
    static bool non_ptpmgmt_sk_setIfUsingIndex(ptpmgmt_sk, int)
    {
        return false;
    }
    static bool ptpmgmt_sk_setIfUsingIndex(ptpmgmt_sk sk, int ifIndex)
    {
        SockBaseIf *s = valid_isk(sk);
        if(s != nullptr)
            return s->setIfUsingIndex(ifIndex);
        return false;
    }
    static bool non_ptpmgmt_sk_setIf(ptpmgmt_sk, const_ptpmgmt_ifInfo)
    {
        return false;
    }
    static bool ptpmgmt_sk_setIf(ptpmgmt_sk sk, const_ptpmgmt_ifInfo ifObj)
    {
        SockBaseIf *s = valid_isk(sk);
        if(s != nullptr && ifObj != nullptr && ifObj->_this != nullptr)
            return s->setIf(*(const IfInfo *)ifObj->_this);
        return false;
    }
    static bool non_ptpmgmt_sk_setAll(ptpmgmt_sk, const_ptpmgmt_ifInfo,
        const_ptpmgmt_cfg, const char *)
    {
        return false;
    }
    static bool ptpmgmt_sk_setAll(ptpmgmt_sk sk, const_ptpmgmt_ifInfo ifObj,
        const_ptpmgmt_cfg cfg, const char *section)
    {
        C2CPP_func2(setAll);
    }
    static bool non_ptpmgmt_sk_setAllInit(ptpmgmt_sk, const_ptpmgmt_ifInfo,
        const_ptpmgmt_cfg, const char *)
    {
        return false;
    }
    static bool ptpmgmt_sk_setAllInit(ptpmgmt_sk sk, const_ptpmgmt_ifInfo ifObj,
        const_ptpmgmt_cfg cfg, const char *section)
    {
        C2CPP_func2(setAllInit);
    }
    static bool non_ptpmgmt_sk_setUdpTtl(ptpmgmt_sk, uint8_t)
    {
        return false;
    }
    static bool ptpmgmt_sk_setUdpTtl(ptpmgmt_sk sk, uint8_t udp_ttl)
    {
        SockIp *s = valid_ipsk(sk);
        if(s != nullptr)
            return s->setUdpTtl(udp_ttl);
        return false;
    }
    static bool non_ptpmgmt_sk_setUdpTtlCfg(ptpmgmt_sk, const_ptpmgmt_cfg,
        const char *)
    {
        return false;
    }
    static bool ptpmgmt_sk_setUdpTtlCfg(ptpmgmt_sk sk, const_ptpmgmt_cfg cfg,
        const char *section)
    {
        SockIp *s = valid_ipsk(sk);
        C2CPP_func(setUdpTtl);
    }
    static bool non_ptpmgmt_sk_setScope(ptpmgmt_sk, uint8_t)
    {
        return false;
    }
    static bool ptpmgmt_sk_setScope(ptpmgmt_sk sk, uint8_t udp6_scope)
    {
        SockIp6 *s = valid_ip6sk(sk);
        if(s != nullptr)
            return s->setScope(udp6_scope);
        return false;
    }
    static bool non_ptpmgmt_sk_setScopeCfg(ptpmgmt_sk, const_ptpmgmt_cfg,
        const char *)
    {
        return false;
    }
    static bool ptpmgmt_sk_setScopeCfg(ptpmgmt_sk sk, const_ptpmgmt_cfg cfg,
        const char *section)
    {
        SockIp6 *s = valid_ip6sk(sk);
        C2CPP_func(setScope);
    }
    static bool non_ptpmgmt_sk_setPtpDstMacStr(ptpmgmt_sk, const char *)
    {
        return false;
    }
    static bool ptpmgmt_sk_setPtpDstMacStr(ptpmgmt_sk sk, const char *str)
    {
        SockRaw *s = valid_rsk(sk);
        if(s != nullptr && str != nullptr)
            return s->setPtpDstMacStr(str);
        return false;
    }
    static bool non_ptpmgmt_sk_setPtpDstMac(ptpmgmt_sk, const void *, size_t)
    {
        return false;
    }
    static bool ptpmgmt_sk_setPtpDstMac(ptpmgmt_sk sk, const void *ptp_dst_mac,
        size_t len)
    {
        SockRaw *s = valid_rsk(sk);
        if(s != nullptr && ptp_dst_mac != nullptr && len > 0)
            return s->setPtpDstMac(ptp_dst_mac, len);
        return false;
    }
    static bool non_ptpmgmt_sk_setPtpDstMacCfg(ptpmgmt_sk, const_ptpmgmt_cfg,
        const char *)
    {
        return false;
    }
    static bool ptpmgmt_sk_setPtpDstMacCfg(ptpmgmt_sk sk, const_ptpmgmt_cfg cfg,
        const char *section)
    {
        SockRaw *s = valid_rsk(sk);
        C2CPP_func(setPtpDstMac);
    }
    static bool non_ptpmgmt_sk_setSocketPriority(ptpmgmt_sk, uint8_t)
    {
        return false;
    }
    static bool ptpmgmt_sk_setSocketPriority(ptpmgmt_sk sk, uint8_t socket_priority)
    {
        SockRaw *s = valid_rsk(sk);
        if(s != nullptr)
            return s->setSocketPriority(socket_priority);
        return false;
    }
    static bool non_ptpmgmt_sk_setSocketPriorityCfg(ptpmgmt_sk, const_ptpmgmt_cfg,
        const char *)
    {
        return false;
    }
    static bool ptpmgmt_sk_setSocketPriorityCfg(ptpmgmt_sk sk,
        const_ptpmgmt_cfg cfg, const char *section)
    {
        SockRaw *s = valid_rsk(sk);
        C2CPP_func(setSocketPriority);
    }
    static ptpmgmt_sk ptpmgmt_sk_alloc_all(enum ptpmgmt_socket_class type,
        void *sko)
    {
        ptpmgmt_sk sk = (ptpmgmt_sk)malloc(sizeof(ptpmgmt_sk_t));
        if(sk == nullptr)
            return nullptr;
        memset(sk, 0, sizeof(ptpmgmt_sk_t));
        SockBase *s = nullptr;
        sk->getPeerAddress = non_ptpmgmt_sk_getPeerAddress;
        sk->isPeerAddressAbstract = non_ptpmgmt_sk_isPeerAddressAbstract;
        sk->setPeerAddress = non_ptpmgmt_sk_setPeerAddress;
        sk->setPeerAddressA = non_ptpmgmt_sk_setPeerAddressA;
        sk->setPeerAddressCfg = non_ptpmgmt_sk_setPeerAddressCfg;
        sk->getSelfAddress = non_ptpmgmt_sk_getSelfAddress;
        sk->isSelfAddressAbstract = non_ptpmgmt_sk_isSelfAddressAbstract;
        sk->setSelfAddress = non_ptpmgmt_sk_setSelfAddress;
        sk->setSelfAddressA = non_ptpmgmt_sk_setSelfAddressA;
        sk->setDefSelfAddress = non_ptpmgmt_sk_setDefSelfAddress;
        sk->getHomeDir = non_ptpmgmt_sk_getHomeDir;
        sk->sendTo = non_ptpmgmt_sk_sendTo;
        sk->sendToA = non_ptpmgmt_sk_sendToA;
        sk->rcvFrom = non_ptpmgmt_sk_rcvFrom;
        sk->rcvFromA = non_ptpmgmt_sk_rcvFromA;
        sk->getLastFrom = non_ptpmgmt_sk_getLastFrom;
        sk->isLastFromAbstract = non_ptpmgmt_sk_isLastFromAbstract;
        sk->setIfUsingName = non_ptpmgmt_sk_setIfUsingName;
        sk->setIfUsingIndex = non_ptpmgmt_sk_setIfUsingIndex;
        sk->setIf = non_ptpmgmt_sk_setIf;
        sk->setAll = non_ptpmgmt_sk_setAll;
        sk->setAllInit = non_ptpmgmt_sk_setAllInit;
        sk->setUdpTtl = non_ptpmgmt_sk_setUdpTtl;
        sk->setUdpTtlCfg = non_ptpmgmt_sk_setUdpTtlCfg;
        sk->setScope = non_ptpmgmt_sk_setScope;
        sk->setScopeCfg = non_ptpmgmt_sk_setScopeCfg;
        sk->setPtpDstMacStr = non_ptpmgmt_sk_setPtpDstMacStr;
        sk->setPtpDstMac = non_ptpmgmt_sk_setPtpDstMac;
        sk->setPtpDstMacCfg = non_ptpmgmt_sk_setPtpDstMacCfg;
        sk->setSocketPriority = non_ptpmgmt_sk_setSocketPriority;
        sk->setSocketPriorityCfg = non_ptpmgmt_sk_setSocketPriorityCfg;
        switch(type) {
            case ptpmgmt_SockUnix:
                if(sko == nullptr)
                    s = new SockUnix();
                sk->getPeerAddress = ptpmgmt_sk_getPeerAddress;
                sk->isPeerAddressAbstract = ptpmgmt_sk_isPeerAddressAbstract;
                sk->setPeerAddress = ptpmgmt_sk_setPeerAddress;
                sk->setPeerAddressA = ptpmgmt_sk_setPeerAddressA;
                sk->setPeerAddressCfg = ptpmgmt_sk_setPeerAddressCfg;
                sk->getSelfAddress = ptpmgmt_sk_getSelfAddress;
                sk->isSelfAddressAbstract = ptpmgmt_sk_isSelfAddressAbstract;
                sk->setSelfAddress = ptpmgmt_sk_setSelfAddress;
                sk->setSelfAddressA = ptpmgmt_sk_setSelfAddressA;
                sk->setDefSelfAddress = ptpmgmt_sk_setDefSelfAddress;
                sk->getHomeDir = ptpmgmt_sk_getHomeDir;
                sk->sendTo = ptpmgmt_sk_sendTo;
                sk->sendToA = ptpmgmt_sk_sendToA;
                sk->rcvFrom = ptpmgmt_sk_rcvFrom;
                sk->rcvFromA = ptpmgmt_sk_rcvFromA;
                sk->getLastFrom = ptpmgmt_sk_getLastFrom;
                sk->isLastFromAbstract = ptpmgmt_sk_isLastFromAbstract;
                break;
            case ptpmgmt_SockIp4:
                if(sko == nullptr)
                    s = new SockIp4();
                // Used by PTP network sockets
                sk->setIfUsingName = ptpmgmt_sk_setIfUsingName;
                sk->setIfUsingIndex = ptpmgmt_sk_setIfUsingIndex;
                sk->setIf = ptpmgmt_sk_setIf;
                sk->setAll = ptpmgmt_sk_setAll;
                sk->setAllInit = ptpmgmt_sk_setAllInit;
                // IP sockets
                sk->setUdpTtl = ptpmgmt_sk_setUdpTtl;
                sk->setUdpTtlCfg = ptpmgmt_sk_setUdpTtlCfg;
                break;
            case ptpmgmt_SockIp6:
                if(sko == nullptr)
                    s = new SockIp6();
                // Used by PTP network sockets
                sk->setIfUsingName = ptpmgmt_sk_setIfUsingName;
                sk->setIfUsingIndex = ptpmgmt_sk_setIfUsingIndex;
                sk->setIf = ptpmgmt_sk_setIf;
                sk->setAll = ptpmgmt_sk_setAll;
                sk->setAllInit = ptpmgmt_sk_setAllInit;
                // IP sockets
                sk->setUdpTtl = ptpmgmt_sk_setUdpTtl;
                sk->setUdpTtlCfg = ptpmgmt_sk_setUdpTtlCfg;
                // IPv6
                sk->setScope = ptpmgmt_sk_setScope;
                sk->setScopeCfg = ptpmgmt_sk_setScopeCfg;
                break;
            case ptpmgmt_SockRaw:
                if(sko == nullptr)
                    s = new SockRaw();
                // Raw socket interfaces
                sk->setPtpDstMacStr = ptpmgmt_sk_setPtpDstMacStr;
                sk->setPtpDstMac = ptpmgmt_sk_setPtpDstMac;
                sk->setPtpDstMacCfg = ptpmgmt_sk_setPtpDstMacCfg;
                sk->setSocketPriority = ptpmgmt_sk_setSocketPriority;
                sk->setSocketPriorityCfg = ptpmgmt_sk_setSocketPriorityCfg;
                // Used by PTP network sockets
                sk->setIfUsingName = ptpmgmt_sk_setIfUsingName;
                sk->setIfUsingIndex = ptpmgmt_sk_setIfUsingIndex;
                sk->setIf = ptpmgmt_sk_setIf;
                sk->setAll = ptpmgmt_sk_setAll;
                sk->setAllInit = ptpmgmt_sk_setAllInit;
                break;
            default:
                free(sk);
                return nullptr;
        }
        if(sko != nullptr) {
            sk->_this = sko;
            sk->free = ptpmgmt_sk_free_wrap;
        } else if(s != nullptr) {
            sk->_this = (void *)s;
            sk->free = ptpmgmt_sk_free;
        } else {
            free(sk);
            return nullptr;
        }
        // Used by all sockets
        sk->close = ptpmgmt_sk_close;
        sk->init = ptpmgmt_sk_init;
        sk->send = ptpmgmt_sk_send;
        sk->rcv = ptpmgmt_sk_rcv;
        sk->getFd = ptpmgmt_sk_getFd;
        sk->fileno = ptpmgmt_sk_getFd;
        sk->poll = ptpmgmt_sk_poll;
        sk->tpoll = ptpmgmt_sk_tpoll;
        return sk;
    }
    ptpmgmt_sk ptpmgmt_sk_alloc(enum ptpmgmt_socket_class type)
    {
        return ptpmgmt_sk_alloc_all(type, nullptr);
    }
    ptpmgmt_sk ptpmgmt_sk_alloc_wrap(enum ptpmgmt_socket_class type, void *sko)
    {
        if(sko == nullptr)
            return nullptr;
        return ptpmgmt_sk_alloc_all(type, sko);
    }
}
