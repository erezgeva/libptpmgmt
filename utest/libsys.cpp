/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Inplement library C calls for system depends unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 *
 */

#include <string>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <cerrno>
#include <climits>
#include <map>
#include <stdarg.h>
#include <time.h>
#include <pwd.h>
#include <dlfcn.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/un.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/timex.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <netinet/in.h>
#include <linux/filter.h>
#include <linux/if_packet.h>
#include <linux/sockios.h>
#include <linux/ptp_clock.h>
#include <linux/ethtool.h>
/*****************************************************************************/
static bool didInit = false;
static bool testMode = false;
static bool rootMode;
struct fdesc_info {
    int domain;
    clockid_t clkID;
};
std::map<int, fdesc_info> fdesc;
std::map<clockid_t, int> clkId2FD;
std::map<clockid_t, bool> clkId2Wr;
static time_t cur_sec;
void useTestMode(bool n)
{
    testMode = n;
    // Zero all
    rootMode = false;
    cur_sec = 0;
    fdesc.clear();
    clkId2FD.clear();
    clkId2Wr.clear();
}
void useRoot(bool n) {rootMode = n;}
/*****************************************************************************/
#define sysFuncDec(ret, name, ...)\
    ret (*_##name)(__VA_ARGS__);\
    extern "C" ret name(__VA_ARGS__)
#define sysFuncAgn(ret, name, ...)\
    _##name = (ret(*)(__VA_ARGS__))dlsym(RTLD_NEXT, #name);\
    if(_##name == nullptr) {\
        fprintf(stderr, "Fail allocating " #name "\n");\
        fail = true;}
/* Allocation can fail, calling need verify! */
#define sysFuncAgZ(ret, name, ...)\
    _##name = (ret(*)(__VA_ARGS__))dlsym(RTLD_NEXT, #name)
sysFuncDec(int, socket, int, int, int) throw();
sysFuncDec(int, close, int);
sysFuncDec(int, select, int, fd_set *, fd_set *, fd_set *, timeval *);
sysFuncDec(int, bind, int, const sockaddr *, socklen_t) throw();
sysFuncDec(int, setsockopt, int, int, int, const void *, socklen_t) throw();
sysFuncDec(ssize_t, recv, int, void *, size_t, int);
sysFuncDec(ssize_t, recvfrom, int, void *, size_t, int, sockaddr *,
    socklen_t *);
sysFuncDec(ssize_t, sendto, int, const void *buf, size_t len, int flags,
    const sockaddr *, socklen_t);
sysFuncDec(ssize_t, recvmsg, int, msghdr *, int);
sysFuncDec(ssize_t, sendmsg, int, const msghdr *, int);
sysFuncDec(uid_t, getuid, void) throw();
sysFuncDec(pid_t, getpid, void) throw();
sysFuncDec(int, unlink, const char *) throw();
sysFuncDec(passwd *, getpwuid, uid_t);
sysFuncDec(int, clock_gettime, clockid_t, timespec *) throw();
sysFuncDec(int, clock_settime, clockid_t, const timespec *) throw();
sysFuncDec(int, clock_adjtime, clockid_t, timex *) throw();
sysFuncDec(long, sysconf, int) throw();
sysFuncDec(int, open, const char *, int, ...);
sysFuncDec(int, __open_2, const char *, int);
sysFuncDec(ssize_t, read, int, void *, size_t);
// glibc stat fucntions
sysFuncDec(int, stat, const char *, struct stat *) throw();
sysFuncDec(int, stat64, const char *, struct stat64 *) throw();
sysFuncDec(int, __xstat, int, const char *, struct stat *);
sysFuncDec(int, __xstat64, int, const char *, struct stat64 *);
sysFuncDec(char *, realpath, const char *, char *) throw();
sysFuncDec(char *, __realpath_chk, const char *, char *, size_t) throw();
sysFuncDec(int, ioctl, int, unsigned long, ...) throw();
void initLibSys(void)
{
    if(didInit) {
        useTestMode(false);
        return;
    }
    bool fail = false;
    sysFuncAgn(int, socket, int, int, int);
    sysFuncAgn(int, close, int);
    sysFuncAgn(int, select, int, fd_set *, fd_set *, fd_set *, timeval *);
    sysFuncAgn(int, bind, int, const sockaddr *, socklen_t);
    sysFuncAgn(int, setsockopt, int, int, int, const void *, socklen_t);
    sysFuncAgn(ssize_t, recv, int, void *, size_t, int);
    sysFuncAgn(ssize_t, recvfrom, int, void *, size_t, int, sockaddr *,
        socklen_t *);
    sysFuncAgn(ssize_t, sendto, int, const void *buf, size_t len, int flags,
        const sockaddr *, socklen_t);
    sysFuncAgn(ssize_t, recvmsg, int, msghdr *, int);
    sysFuncAgn(ssize_t, sendmsg, int, const msghdr *, int);
    sysFuncAgn(uid_t, getuid, void);
    sysFuncAgn(pid_t, getpid, void);
    sysFuncAgn(int, unlink, const char *);
    sysFuncAgn(passwd *, getpwuid, uid_t);
    sysFuncAgn(int, clock_gettime, clockid_t, timespec *);
    sysFuncAgn(int, clock_settime, clockid_t, const timespec *);
    sysFuncAgn(int, clock_adjtime, clockid_t, timex *);
    sysFuncAgn(long, sysconf, int);
    sysFuncAgn(int, open, const char *, int, ...);
    sysFuncAgn(int, __open_2, const char *, int);
    sysFuncAgn(ssize_t, read, int, void *, size_t);
    sysFuncAgZ(int, stat, const char *, struct stat *);
    sysFuncAgZ(int, stat64, const char *, struct stat64 *);
    sysFuncAgn(int, __xstat, int, const char *, struct stat *);
    sysFuncAgn(int, __xstat64, int, const char *, struct stat64 *);
    sysFuncAgn(char *, realpath, const char *, char *);
    sysFuncAgn(char *, __realpath_chk, const char *, char *, size_t);
    sysFuncAgn(int, ioctl, int, unsigned long, ...);
    if(fail)
        fprintf(stderr, "Fail obtain address of functions\n");
    didInit = true;
    useTestMode(false);
}
// Make sure we initialize the library functions call back
__attribute__((constructor)) static void staticInit(void) { initLibSys(); }
/*****************************************************************************/
#define retSock(name, ...)\
    if(!testMode || fdesc.count(fd) == 0)\
        return _##name(fd, __VA_ARGS__)
#define retClk(name, ...)\
    if(!testMode || (clk_id != CLOCK_REALTIME && clkId2FD.count(clk_id) == 0))\
        return _##name(clk_id, __VA_ARGS__)
#define retTest0(name)\
    if(!testMode)\
        return _##name()
#define retTest(name, ...)\
    if(!testMode)\
        return _##name(__VA_ARGS__)
#define cmp_addr(n) if(addrlen == sizeof n && addr != nullptr &&\
    memcmp(addr, n, sizeof n) == 0){break;}
#define cmp_paddr(n) if(addrlen == sizeof n && memcmp(addr, n, sizeof n) == 0)\
        return 0
#define cmp_opt(n) if(optlen != sizeof n || optval == nullptr ||\
    memcmp(optval, n, sizeof n) != 0)\
    return retErr(EINVAL);break
#define cmp_int(v) if(optlen != sizeof(int) ||\
    *(int *)optval != v)\
    return retErr(EINVAL); break
#define STAT_RET(nm)\
    if(!testMode) {\
        if(_##nm != nullptr)\
            _##nm(name, sp);\
        else\
            ___x##nm(3, name, sp);\
    }
#define STAT_BODY\
    if(sp == nullptr)\
        return retErr(EINVAL);\
    if(strcmp("/dev/ptp0", name) != 0 &&\
        strcmp("/dev/ptp1", name) != 0)\
        return retErr(EINVAL);\
    sp->st_mode = S_IFCHR;\
    return 0
static inline int retErr(int err)
{
    errno = err;
    return -1;
}
static inline bool isDir(const char *name)
{
    DIR *dir = opendir(name);
    if(dir == nullptr)
        return false;
    closedir(dir);
    return true;
}
static passwd usePass = {
    .pw_name = const_cast<char *>("usr"),
    .pw_passwd = const_cast<char *>("pwd"),
    .pw_uid = 100,
    .pw_gid = 100,
    .pw_gecos = const_cast<char *>("geo"),
    .pw_dir = const_cast<char *>("/home/usr"),
    .pw_shell = const_cast<char *>("shell")
};
static passwd rootPass = {
    .pw_name = const_cast<char *>("root"),
    .pw_passwd = const_cast<char *>("pwd"),
    .pw_uid = 0,
    .pw_gid = 0,
    .pw_gecos = const_cast<char *>("geo"),
    .pw_dir = const_cast<char *>("/root"),
    .pw_shell = const_cast<char *>("shell")
};
static inline clockid_t fd_to_clockid(int fd)
{
    return (~(clockid_t)(fd) << 3) | 3;
}
static inline ssize_t recvFill(void *buf, size_t len, int flags)
{
    uint8_t *b = (uint8_t *)buf;
    if(flags & MSG_DONTWAIT)
        b[0] = 2;
    else
        b[0] = 1;
    size_t l = std::min((size_t)5, len);
    if(l > 1)
        memcpy(b + 1, "\x4\x5\x6\x7", l - 1);
    return l;
}
static inline int l_open(const char *name, int flags)
{
    if((strcmp("/dev/ptp0", name) != 0 || flags != O_RDWR) &&
        (strcmp("/dev/ptp1", name) != 0 || flags != O_RDONLY))
        return retErr(EINVAL);
    int fd = _socket(AF_INET, SOCK_DGRAM, 0);
    if(fd >= 0) {
        fdesc[fd].domain = AF_CAN; // Any which we do not use :-)
        clockid_t clkID = fd_to_clockid(fd);
        fdesc[fd].clkID = clkID;
        clkId2FD[clkID] = fd;
        clkId2Wr[clkID] = flags == O_RDWR;
    }
    return fd;
}
static inline char *l_realpath(const char *path, char *resolved)
{
    if(path == nullptr || resolved == nullptr || *path == 0)
        return nullptr;
    if(strchr(path, '/') == nullptr) {
        std::string ret = "/dev/";
        ret += path;
        strcpy(resolved, ret.c_str());
    } else
        strcpy(resolved, path);
    return resolved;
}
/*****************************************************************************/
const uint8_t ua_addr_b[110] = { 1, 0, 47, 109, 101 };
const uint8_t ua_addr_b0[110] = {1, 0, 47, 104, 111, 109, 101, 47, 117, 115,
        114, 47, 46, 112, 109, 99, 46, 49, 49, 49
    };
const uint8_t ip_addr_b[16] = { 2, 0, 1, 64 };
const uint8_t ip6_addr_b[28] = { 10, 0, 1, 64 };
const uint8_t raw_addr_b[20] = { 17, 0, 0, 3, 7 };
const uint8_t bpf_filter[4] = {40};
const uint8_t so_bindtodevice[4] = { 'e', 't', 'h', '0' };
const uint8_t ip_add_membership[12] = { 224, 0, 1, 129, 0, 0, 0, 0, 7 };
const uint8_t ip_multicast_if[12] = { 0, 0, 0, 0, 0, 0, 0, 0, 7 };
const uint8_t ipv6_add_membership[20] = { 255, 15, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 1, 129, 7
    };
const uint8_t ipv6_multicast_if[20] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 7
    };
const uint8_t packet_add_membership[16] = { 7, 0, 0, 0, 0, 0, 6,
        0, 1, 27, 23, 15, 12
    };
const uint8_t ua_addr_s[110] = { 1, 0, 47, 112, 101, 101, 114 };
const uint8_t ua_addr_s2[110] = { 1, 0, 0, 112, 101, 101, 114 };
const uint8_t ip_addr_s[16] = { 2, 0, 1, 64, 224, 0, 1, 129 };
const uint8_t ip6_addr_s[28] = { 10, 0, 1, 64, 0, 0, 0, 0, 255, 15, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 129
    };
const uint8_t msg_name[20] = { 17, 0, 0, 3, 7, 0, 0, 0, 0,
        0, 0, 6, 1, 27, 23, 15, 12
    };
const uint8_t msg_iov_0[14] = {1, 27, 23, 15, 12, 0, 1, 2, 3, 4, 5, 6, 136, 247 };
/*****************************************************************************/
int socket(int domain, int type, int protocol) throw()
{
    retTest(socket, domain, type, protocol);
    bool add = false;
    switch(domain) {
        case AF_UNIX:
        case AF_INET:
        case AF_INET6:
            add = type == SOCK_DGRAM && protocol == 0;
            break;
        case AF_PACKET:
            add = type == SOCK_RAW;
            break;
        default:
            break;
    }
    if(!add)
        return _socket(domain, type, protocol);
    int fd = _socket(AF_INET, SOCK_DGRAM, 0);
    if(fd >= 0) {
        fdesc[fd].domain = domain;
        fdesc[fd].clkID = 0;
    }
    return fd;
}
int close(int fd)
{
    if(testMode && fdesc.count(fd) > 0) {
        clockid_t clkID = fdesc[fd].clkID;
        if(clkID != 0) {
            clkId2FD.erase(clkID);
            clkId2Wr.erase(clkID);
        }
        fdesc.erase(fd);
    }
    return _close(fd);
}
int select(int nfds, fd_set *rfds, fd_set *wfds, fd_set *efds, timeval *to)
{
    retTest(select, nfds, rfds, wfds, efds, to);
    if(wfds == nullptr && efds == nullptr) {
        int fd = nfds - 1;
        if(fdesc.count(fd) > 0 && FD_ISSET(fd, rfds) > 0) {
            FD_ZERO(rfds);
            FD_SET(fd, rfds);
            if(to != nullptr && ((to->tv_usec != 0 && to->tv_usec != 500000) ||
                    to->tv_sec > 2 || to->tv_sec < 0))
                return retErr(EINVAL);
            return 1; // One FD!
        }
    }
    return _select(nfds, rfds, wfds, efds, to);
}
int bind(int fd, const sockaddr *addr, socklen_t addrlen) throw()
{
    retSock(bind, addr, addrlen);
    if(addr == nullptr || addrlen <= 0)
        return retErr(EINVAL);
    switch(fdesc[fd].domain) {
        case AF_UNIX:
            cmp_paddr(ua_addr_b);
            cmp_paddr(ua_addr_b0);
            break;
        case AF_INET:
            cmp_paddr(ip_addr_b);
            break;
        case AF_INET6:
            cmp_paddr(ip6_addr_b);
            break;
        case AF_PACKET:
            cmp_paddr(raw_addr_b);
            break;
        default:
            return retErr(ENOTSOCK);
    }
    return retErr(EINVAL);
}
int setsockopt(int fd, int level, int optname, const void *optval,
    socklen_t optlen) throw()
{
    retSock(setsockopt, level, optname, optval, optlen);
    if(optval == nullptr || optlen <= 0)
        return retErr(EINVAL);
    int *ival = (int *)optval;
    switch(level) {
        case SOL_SOCKET:
            switch(optname) {
                case SO_REUSEADDR:
                    cmp_int(1);
                case SO_PRIORITY:
                    cmp_int(7);
                case SO_ATTACH_FILTER:
                    if(optlen == sizeof(sock_fprog)) {
                        sock_fprog *bpf = (sock_fprog *)optval;
                        if(bpf->len == 4 && bpf->filter != nullptr &&
                            memcmp(bpf->filter, bpf_filter, 4) == 0)
                            break;
                    }
                    return retErr(EINVAL);
                case SO_BINDTODEVICE:
                    cmp_opt(so_bindtodevice);
                default:
                    return retErr(ENOPROTOOPT);
            }
            break;
        case IPPROTO_IP:
            switch(optname) {
                case IP_MULTICAST_TTL:
                    cmp_int(7);
                case IP_MULTICAST_LOOP:
                    cmp_int(0);
                case IP_ADD_MEMBERSHIP:
                    cmp_opt(ip_add_membership);
                case IP_MULTICAST_IF:
                    cmp_opt(ip_multicast_if);
                default:
                    return retErr(ENOPROTOOPT);
            }
            break;
        case IPPROTO_IPV6:
            switch(optname) {
                case IPV6_MULTICAST_HOPS:
                    cmp_int(7);
                case IPV6_MULTICAST_LOOP:
                    cmp_int(0);
                case IPV6_ADD_MEMBERSHIP:
                    cmp_opt(ipv6_add_membership);
                case IPV6_MULTICAST_IF:
                    cmp_opt(ipv6_multicast_if);
                default:
                    return retErr(ENOPROTOOPT);
            }
            break;
        case SOL_PACKET:
            switch(optname) {
                case PACKET_ADD_MEMBERSHIP:
                    cmp_opt(packet_add_membership);
                default:
                    return retErr(ENOPROTOOPT);
            }
            break;
        default:
            return retErr(ENOPROTOOPT);
    }
    return 0;
}
ssize_t recv(int fd, void *buf, size_t len, int flags)
{
    retSock(recv, buf, len, flags);
    if(buf == nullptr || len == 0)
        return retErr(ENOMEM);
    switch(fdesc[fd].domain) {
        case AF_INET:
        case AF_INET6:
            break;
        case AF_UNIX:
        case AF_PACKET:
            return retErr(EINVAL);
    }
    if(flags & ~MSG_DONTWAIT)
        return retErr(ECONNRESET);
    return recvFill(buf, len, flags);
}
ssize_t recvfrom(int fd, void *buf, size_t len, int flags, sockaddr *src_addr,
    socklen_t *addrlen)
{
    retSock(recvfrom, buf, len, flags, src_addr, addrlen);
    if(buf == nullptr || len == 0)
        return retErr(ENOMEM);
    if(flags & ~MSG_DONTWAIT)
        return retErr(ECONNRESET);
    switch(fdesc[fd].domain) {
        case AF_UNIX:
            if(addrlen == nullptr || *addrlen < sizeof(sockaddr_un) ||
                src_addr == nullptr)
                return retErr(EINVAL);
            *addrlen = sizeof(sockaddr_un);
            {
                sockaddr_un *ua = (sockaddr_un *)src_addr;
                ua->sun_family = AF_UNIX;
                strcpy(ua->sun_path, "/peer");
            }
            break;
        case AF_INET:
        case AF_INET6:
        case AF_PACKET:
            return retErr(EINVAL);
    }
    return recvFill(buf, len, flags);
}
ssize_t sendto(int fd, const void *buf, size_t len, int flags,
    const sockaddr *addr, socklen_t addrlen)
{
    retSock(sendto, buf, len, flags, addr, addrlen);
    if(buf == nullptr || len == 0)
        return retErr(ENOMEM);
    if(flags != 0)
        return retErr(ECONNRESET);
    switch(fdesc[fd].domain) {
        case AF_UNIX:
            cmp_addr(ua_addr_s)
            cmp_addr(ua_addr_s2)
            return retErr(EINVAL);
        case AF_INET:
            cmp_addr(ip_addr_s)
            return retErr(EINVAL);
        case AF_INET6:
            cmp_addr(ip6_addr_s)
            return retErr(EINVAL);
        case AF_PACKET:
            return retErr(EINVAL);
    }
    if(len != 5 && memcmp(buf, "\x1\x2\x3\x4\x5", 5) != 0)
        return retErr(ECONNRESET);
    return len;
}
ssize_t recvmsg(int fd, msghdr *msg, int flags)
{
    retSock(recvmsg, msg, flags);
    if(msg == nullptr || msg->msg_iov == nullptr || msg->msg_iovlen == 0)
        return retErr(ENOMEM);
    if(fdesc[fd].domain != AF_PACKET)
        return retErr(EINVAL);
    if(flags & ~MSG_DONTWAIT)
        return retErr(ECONNRESET);
    if(msg == nullptr || msg->msg_control != nullptr || msg->msg_controllen != 0 ||
        msg->msg_flags != 0 || msg->msg_iovlen != 2)
        return retErr(EINVAL);
    if(msg->msg_namelen != 0 || msg->msg_name != nullptr)
        return retErr(EINVAL);
    if(msg->msg_iov[0].iov_len != 14 || msg->msg_iov[0].iov_base == nullptr)
        return retErr(EINVAL);
    if(msg->msg_iov[1].iov_len == 0 || msg->msg_iov[1].iov_base == nullptr)
        return retErr(EINVAL);
    return recvFill(msg->msg_iov[1].iov_base, msg->msg_iov[1].iov_len, flags) + 14;
}
ssize_t sendmsg(int fd, const msghdr *msg, int flags)
{
    retSock(sendmsg, msg, flags);
    if(msg == nullptr || msg->msg_iov == nullptr || msg->msg_iovlen == 0)
        return retErr(ENOMEM);
    if(fdesc[fd].domain != AF_PACKET)
        return retErr(EINVAL);
    if(flags != 0)
        return retErr(ECONNRESET);
    if(msg == nullptr || msg->msg_control != nullptr || msg->msg_controllen != 0 ||
        msg->msg_flags != 0 || msg->msg_iovlen != 2)
        return retErr(EINVAL);
    if(msg->msg_namelen != sizeof msg_name || msg->msg_name == nullptr ||
        memcmp(msg->msg_name, msg_name, sizeof msg_name) != 0)
        return retErr(EINVAL);
    if(msg->msg_iov[0].iov_len != sizeof msg_iov_0 ||
        msg->msg_iov[0].iov_base == nullptr ||
        memcmp(msg->msg_iov[0].iov_base, msg_iov_0, sizeof msg_iov_0) != 0)
        return retErr(ECONNRESET);
    if(msg->msg_iov[1].iov_len != 5 || msg->msg_iov[1].iov_base == nullptr ||
        memcmp(msg->msg_iov[1].iov_base, "\x1\x2\x3\x4\x5", 5) != 0)
        return retErr(ECONNRESET);
    return 5 + sizeof msg_iov_0;
}
uid_t getuid(void) throw()
{
    retTest0(getuid);
    if(rootMode)
        return 0;
    uid_t cur = 100;
    std::string curDir;
    do {
        cur++;
        curDir = "/var/run/user/";
        curDir += std::to_string(cur);
    } while(isDir(curDir.c_str()));
    return cur;
}
pid_t getpid(void) throw()
{
    retTest0(getpid);
    return 111;
}
int unlink(const char *name) throw()
{
    retTest(unlink, name);
    if(*name == 0)
        return retErr(EFAULT);
    return 0;
}
passwd *getpwuid(uid_t uid)
{
    retTest(getpwuid, uid);
    if(uid == 0)
        return &rootPass;
    return &usePass;
}
int clock_gettime(clockid_t clk_id, timespec *tp) throw()
{
    retClk(clock_gettime, tp);
    if(clk_id == CLOCK_REALTIME) {
        // Every call passed 1 second since the last one :-)
        tp->tv_sec = ++cur_sec;
        tp->tv_nsec = 0;
    } else {
        tp->tv_sec = 17;
        tp->tv_nsec = 567;
    }
    return 0;
}
int clock_settime(clockid_t clk_id, const timespec *tp) throw()
{
    retClk(clock_settime, tp);
    if(clk_id == CLOCK_REALTIME) {
        if(tp->tv_sec == 12 && tp->tv_nsec == 147)
            return 0;
        if(tp->tv_sec == 17 && tp->tv_nsec == 567)
            return 0;
    } else {
        if(tp->tv_sec == 19 && tp->tv_nsec == 351)
            return 0;
        if(tp->tv_sec == 1 && tp->tv_nsec == 0)
            return 0;
    }
    return retErr(EINVAL);
}
int clock_adjtime(clockid_t clk_id, timex *tmx) throw()
{
    retClk(clock_adjtime, tmx);
    if(tmx->maxerror != 0 || tmx->esterror != 0 ||
        tmx->status != 0 || tmx->constant != 0 || tmx->precision != 0 ||
        tmx->tolerance != 0 || tmx->ppsfreq != 0 || tmx->jitter != 0 ||
        tmx->shift != 0 || tmx->stabil != 0 || tmx->jitcnt != 0 ||
        tmx->calcnt != 0 || tmx->errcnt != 0 || tmx->stbcnt != 0 || tmx->tai != 0)
        return retErr(EINVAL);
    if(clk_id == CLOCK_REALTIME) {
        switch(tmx->modes) {
            case ADJ_SETOFFSET | ADJ_NANO:
                if(tmx->tick != 0 || tmx->freq != 0 || tmx->offset != 0)
                    return retErr(EINVAL);
                if(tmx->time.tv_sec == 17 && tmx->time.tv_usec == 29)
                    break;
                if(tmx->time.tv_sec == -20 && tmx->time.tv_usec == 37)
                    break;
                return retErr(EINVAL);
            case ADJ_FREQUENCY | ADJ_TICK:
                if(tmx->tick != 14 || tmx->freq != -1572864000 ||
                    tmx->time.tv_sec != 0 || tmx->time.tv_usec != 0 ||
                    tmx->offset != 0)
                    return retErr(EINVAL);
                break;
            case ADJ_OFFSET | ADJ_NANO:
                if(tmx->tick != 0 || tmx->freq != 0 || tmx->time.tv_sec != 0 ||
                    tmx->time.tv_usec != 0 || tmx->offset != 218150012)
                    return retErr(EINVAL);
                break;
            case 0:
                if(tmx->tick != 0 || tmx->freq != 0 || tmx->time.tv_sec != 0 ||
                    tmx->time.tv_usec != 0 || tmx->offset != 0)
                    return retErr(EINVAL);
                tmx->tick = 786;
                tmx->freq = 254;
                break;
            default:
                return retErr(EINVAL);
        }
    } else {
        switch(tmx->modes) {
            case ADJ_SETOFFSET | ADJ_NANO:
                if(tmx->tick != 0 || tmx->freq != 0 || tmx->offset != 0)
                    return retErr(EINVAL);
                if(tmx->time.tv_sec == 93 && tmx->time.tv_usec == 571)
                    break;
                if(tmx->time.tv_sec == -20 && tmx->time.tv_usec == 37)
                    break;
                return retErr(EINVAL);
            case ADJ_FREQUENCY:
                if(tmx->tick != 0 || tmx->freq != 15386542 ||
                    tmx->time.tv_sec != 0 || tmx->time.tv_usec != 0 ||
                    tmx->offset != 0)
                    return retErr(EINVAL);
                break;
            case ADJ_OFFSET | ADJ_NANO:
                if(tmx->tick != 0 || tmx->freq != 0 || tmx->time.tv_sec != 0 ||
                    tmx->time.tv_usec != 0 || tmx->offset != 265963)
                    return retErr(EINVAL);
                break;
            case 0:
                if(tmx->tick != 0 || tmx->freq != 0 || tmx->time.tv_sec != 0 ||
                    tmx->time.tv_usec != 0 || tmx->offset != 0)
                    return retErr(EINVAL);
                tmx->tick = 934;
                tmx->freq = 654;
                break;
            default:
                return retErr(EINVAL);
        }
    }
    return TIME_OK;
}
long sysconf(int name) throw()
{
    if(!didInit) // Somehow this function may be called before the init
        initLibSys();
    if(testMode && name == _SC_CLK_TCK)
        return 100000;
    return _sysconf(name);
}
int open(const char *name, int flags, ...)
{
    if(!testMode) {
        if((flags & O_CREAT) == O_CREAT || (flags & O_TMPFILE) == O_TMPFILE) {
            va_list ap;
            va_start(ap, flags);
            mode_t mode = va_arg(ap, mode_t);
            va_end(ap);
            retTest(open, name, flags, mode);
        } else
            retTest(open, name, flags);
    }
    return l_open(name, flags);
}
int __open_2(const char *name, int flags)
{
    retTest(open, name, flags);
    return l_open(name, flags);
}
ssize_t read(int fd, void *buf, size_t count)
{
    retSock(read, buf, count);
    if(fdesc[fd].domain == AF_CAN) {
        size_t events = count / sizeof(ptp_extts_event);
        if(events == 1 || events == 10) {
            ptp_extts_event *ent = (ptp_extts_event *)buf;
            if(events == 1) {
                ent->index = 19;
                ent->t = { .sec = 123, .nsec = 712 };
            } else { // 3 events
                ent->index = 2;
                ent++->t = { .sec = 34, .nsec = 7856 };
                ent->index = 6;
                ent++->t = { .sec = 541, .nsec = 468 };
                ent->index = 3;
                ent->t = { .sec = 1587, .nsec = 12 };
                events = 3;
            }
            return events * sizeof(ptp_extts_event);
        }
    }
    return retErr(EINVAL);
}
// glibc stat fucntions
int stat(const char *name, struct stat *sp) throw()
{
    STAT_RET(stat);
    STAT_BODY;
}
int stat64(const char *name, struct stat64 *sp) throw()
{
    STAT_RET(stat64);
    STAT_BODY;
}
int __xstat(int ver, const char *name, struct stat *sp)
{
    retTest(__xstat, ver, name, sp);
    STAT_BODY;
}
int __xstat64(int ver, const char *name, struct stat64 *sp)
{
    retTest(__xstat64, ver, name, sp);
    STAT_BODY;
}
char *realpath(const char *path, char *resolved) throw()
{
    retTest(realpath, path, resolved);
    return l_realpath(path, resolved);
}
char *__realpath_chk(const char *path, char *resolved,
    size_t resolvedlen) throw()
{
    retTest(__realpath_chk, path, resolved, resolvedlen);
    return l_realpath(path, resolved);
}
int ioctl(int fd, unsigned long request, ...) throw()
{
    va_list ap;
    va_start(ap, request);
    void *arg = va_arg(ap, void *);
    va_end(ap);
    retSock(ioctl, request, arg);
    if(arg == nullptr)
        return retErr(EFAULT);
    ifreq *ifr = (ifreq *)arg;
    switch(request) {
        case SIOCGIFHWADDR:
            if(strcmp("eth0", ifr->ifr_name) != 0 || ifr->ifr_ifindex != 7)
                return retErr(EINVAL);
            memcpy(ifr->ifr_hwaddr.sa_data, "\x1\x2\x3\x4\x5\x6", 6);
            break;
        case SIOCETHTOOL:
            if(strcmp("eth0", ifr->ifr_name) != 0)
                return retErr(EINVAL);
            {
                ethtool_ts_info *info = (ethtool_ts_info *)ifr->ifr_data;
                if(info == nullptr || info->cmd != ETHTOOL_GET_TS_INFO)
                    return retErr(EINVAL);
                info->phc_index = 3;
            }
            break;
        case SIOCGIFINDEX:
            if(strcmp("eth0", ifr->ifr_name) != 0)
                return retErr(EINVAL);
            ifr->ifr_ifindex = 7;
            break;
        case SIOCGIFNAME:
            if(ifr->ifr_ifindex != 7)
                return retErr(EINVAL);
            strcpy(ifr->ifr_name, "eth0");
            break;
        case PTP_CLOCK_GETCAPS: {
            ptp_clock_caps *cps = (ptp_clock_caps *)arg;
            cps->max_adj = 17;
            cps->n_alarm = 32;
            cps->n_ext_ts = 48;
            cps->n_per_out = 75;
            cps->pps = 13;
            cps->n_pins = 12;
            cps->cross_timestamping = 1;
            #ifdef PTP_CLOCK_GETCAPS2
            cps->adjust_phase = 0;
            #endif
            break;
        }
        case PTP_PIN_GETFUNC: {
            ptp_pin_desc *desc = (ptp_pin_desc *)arg;
            if(desc->index != 1)
                return retErr(EINVAL);
            strcpy(desc->name, "pin desc");
            desc->chan = 19;
            desc->func = PTP_PF_PHYSYNC;
            break;
        }
        case PTP_PIN_SETFUNC: {
            ptp_pin_desc *desc = (ptp_pin_desc *)arg;
            if(desc->index != 2 || desc->chan != 23 || desc->func != PTP_PF_PEROUT)
                return retErr(EINVAL);
            break;
        }
        case PTP_ENABLE_PPS: {
            if(*(int *)arg != 1)
                return retErr(EINVAL);
            break;
        }
        case PTP_SYS_OFFSET: {
            ptp_sys_offset *req = (ptp_sys_offset *)arg;
            if(req->n_samples != 5)
                return retErr(EINVAL);
            req->n_samples = 2;
            req->ts[0] = { .sec = 11, .nsec = 33 }; // 0 sys
            req->ts[1] = { .sec = 22, .nsec = 44 }; // 0 phc
            req->ts[2] = { .sec = 71, .nsec = 63 }; // 1 sys
            req->ts[3] = { .sec = 62, .nsec = 84 }; // 1 phc
            break;
        }
        #ifdef PTP_SYS_OFFSET_EXTENDED
        case PTP_SYS_OFFSET_EXTENDED: {
            ptp_sys_offset_extended *req = (ptp_sys_offset_extended *)arg;
            if(req->n_samples != 7)
                return retErr(EINVAL);
            req->n_samples = 2;
            req->ts[0][0] = { .sec = 11, .nsec = 33 }; // 0 sys before
            req->ts[0][1] = { .sec = 22, .nsec = 44 }; // 0 phc
            req->ts[0][2] = { .sec = 84, .nsec = 91 }; // 1 sys after
            req->ts[1][0] = { .sec = 71, .nsec = 63 }; // 1 sys before
            req->ts[1][1] = { .sec = 62, .nsec = 84 }; // 1 phc
            req->ts[1][2] = { .sec = 45, .nsec = 753 }; // 1 sys after
            break;
        }
        #endif // PTP_SYS_OFFSET_EXTENDED
        case PTP_SYS_OFFSET_PRECISE: {
            ptp_sys_offset_precise *req = (ptp_sys_offset_precise *)arg;
            req->device = { .sec = 17, .nsec = 135 };
            req->sys_realtime = { .sec = 415, .nsec = 182 };
            req->sys_monoraw = { .sec = 9413, .nsec = 3654 };
            break;
        }
        #ifdef PTP_EXTTS_REQUEST2
        case PTP_EXTTS_REQUEST2:
            #endif
        case PTP_EXTTS_REQUEST: {
            ptp_extts_request *req = (ptp_extts_request *)arg;
            // Enable
            if(req->index == 7 &&
                req->flags == (PTP_ENABLE_FEATURE | PTP_RISING_EDGE))
                return 0;
            // Disable
            if(req->index == 9 && req->flags == 0)
                return 0;
            return retErr(EINVAL);
        }
        #ifdef PTP_PEROUT_REQUEST2
        case PTP_PEROUT_REQUEST2:
            #endif
        case PTP_PEROUT_REQUEST: {
            ptp_perout_request *req = (ptp_perout_request *)arg;
            if(req->index != 11 || req->start.nsec != 0 || req->period.sec != 76 ||
                req->period.nsec != 154)
                return retErr(EINVAL);
            if((req->flags & PTP_PEROUT_PHASE) && req->start.sec != 19)
                return retErr(EINVAL);
            break;
        }
        default:
            return retErr(EINVAL);
    }
    return 0;
}
