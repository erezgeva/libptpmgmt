/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* sock.cpp use unix socket to communicate with ptp4l
 *
 * Authors: Erez Geva <ErezGeva2@gmail.com>
 *
 */

#include "sock.h"
#include <pwd.h>
#include <unistd.h>
#include <sys/socket.h>

const char *useDefstr = "/.pmc.";
const char *rootBasestr = "/var/run/pmc.";
const size_t unix_path_max = sizeof(((struct sockaddr_un *)nullptr)
                             ->sun_path) - 1;

static inline bool testUnix(const std::string &str)
{
    size_t len = str.length();
    if (len < 2 || len > unix_path_max || str.substr(0,1) != "/")
        return false;
    return true;
}
static inline bool testUnix(const char *str)
{
    if (str == nullptr)
        return false;
    size_t len = strlen(str);
    if (len < 2 || len > unix_path_max || *str != '/')
        return false;
    return true;
}
static inline void setAddr(struct sockaddr_un &addr, std::string &str)
{
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, str.c_str(), unix_path_max);
}
static inline void setAddr(struct sockaddr_un &addr, const char *str)
{
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, str, unix_path_max);
}
sockUnix::sockUnix() : m_fd(-1), m_isInit(false)
{
    setAddr(m_peerAddr, m_peer);
}
sockUnix::~sockUnix()
{
    this->close();
}
void sockUnix::close()
{
    if (m_fd >= 0) {
        ::close(m_fd);
        unlink(m_me.c_str());
        m_fd = -1;
        m_isInit = false;
    }

}
bool sockUnix::init()
{
    if (m_isInit || !testUnix(m_me))
        return false;
    m_fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if (m_fd < 0) {
        perror("socket");
        return false;
    }
    struct sockaddr_un addr;
    setAddr(addr, m_me);
    auto ret = bind(m_fd, (struct sockaddr*)&addr, sizeof(addr));
    if (ret < 0) {
        perror("bind");
        ::close(m_fd);
        m_fd = -1;
        return false;
    }
    m_isInit = true;
    return true;
}
int sockUnix::getFd()
{
    return m_fd;
}
bool sockUnix::setPeerInternal(const std::string &str)
{
    if (!testUnix(str))
        return false;
    m_peer = str;
    setAddr(m_peerAddr, m_peer);
    return true;
}
bool sockUnix::setPeerInternal(const char *str)
{
    if (!testUnix(str))
        return false;
    m_peer = str;
    setAddr(m_peerAddr, m_peer);
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
bool sockUnix::setPeerAddress(configFile &cfg, std::string &section)
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
    if (m_isInit || !testUnix(str))
        return false;
    m_me = str;
    return true;
}
bool sockUnix::setSelfAddress(const char *str)
{
    // self address can not be changed after init is done
    if (m_isInit || !testUnix(str))
        return false;
    m_me = str;
    return true;
}
bool sockUnix::setDefSelfAddress(std::string &rootBase, std::string &useDef)
{
    // self address can not be changed after init is done
    if (m_isInit)
        return false;
    std::string new_me;
    auto uid = getuid();
    if (uid)
    {
        new_me = getHomeDir();
        if (useDef.empty())
            useDef = useDefstr;
        new_me += useDef;
    }
    else
    {
        if (rootBase.empty())
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
    if (rootBase != nullptr)
        s1 = rootBase;
    if (useDef != nullptr)
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
bool sockUnix::sendAny(const void *msg, size_t len, struct sockaddr_un &addr)
{
    ssize_t cnt = sendto(m_fd, msg, len, 0, (struct sockaddr*)&addr,
                      sizeof(addr));
    if (cnt < 0) {
        perror("send");
        return false;
    }
    if (cnt != (ssize_t)len) {
        fprintf(stderr, "send %zd instead of %zu\n", cnt, len);
        return false;
    }
    return true;
}
bool sockUnix::send(const void *msg, size_t len)
{
    if (!m_isInit || !testUnix(m_peer))
        return false;
    return sendAny(msg, len, m_peerAddr);
}
bool sockUnix::sendTo(const void *msg, size_t len, const char *addrStr)
{
    if (!m_isInit || !testUnix(addrStr))
        return false;
    struct sockaddr_un addr;
    setAddr(addr, addrStr);
    return sendAny(msg, len, addr);
}
bool sockUnix::sendTo(const void *msg, size_t len, std::string &addrStr)
{
    if (!m_isInit || !testUnix(addrStr))
        return false;
    struct sockaddr_un addr;
    setAddr(addr, addrStr);
    return sendAny(msg, len, addr);
}
ssize_t sockUnix::rcv(void *buf, size_t bufSize, bool block)
{
    if (!testUnix(m_peer))
        return -1;
    std::string from;
    ssize_t ret = rcvFrom(buf, bufSize, from, block);
    if (ret > 0 && from == m_peer)
        return ret;
    return -1;
}
ssize_t sockUnix::rcvFrom(void *buf, size_t bufSize, std::string &from,
                          bool block)
{
    if (!m_isInit)
        return -1;
    struct sockaddr_un addr;
    socklen_t len = sizeof(addr);
    int flags = 0;
    if (!block)
        flags |= MSG_DONTWAIT;
    auto cnt = recvfrom(m_fd, buf, bufSize, flags, (struct sockaddr*)&addr,
                        &len);
    if (cnt < 0) {
        perror("send");
        return -1;
    }
    if (cnt > (ssize_t)bufSize) {
        fprintf(stderr, "rcv %zd more then buffer size %zu\n", cnt, bufSize);
        return -1;
    }
    addr.sun_path[unix_path_max] = 0; // Ensure string is null terminated
    from = addr.sun_path;
    return cnt;
}
bool sockUnix::poll(uint64_t timeout_ms)
{
   struct timeval *pto, to;
   if (timeout_ms > 0) {
        to = { .tv_sec = (long)(timeout_ms / 1000),
               .tv_usec = (long)(timeout_ms % 1000) * 1000
             };
        pto = &to;
   } else
        pto = nullptr;
   fd_set rs;
   FD_ZERO(&rs);
   FD_SET(m_fd, &rs);
   int ret = select(m_fd + 1, &rs, nullptr, nullptr, pto);
   if (ret > 0 && FD_ISSET(m_fd, &rs))
       return true;
   return false;
}
