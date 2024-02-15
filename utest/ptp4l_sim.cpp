/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Simulate dummy PHC clock for ptp4l
 *        The purpose is to get Managment replies on UDS socket
 *         and run phc_ctl.
 *        Nothing beyond!
 *        If you wish to simulate a clocks network.
 *        You can use https://github.com/mlichvar/clknetsim/
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2023 Erez Geva
 *
 */

#include <string>
#include <cstring>
#include <dlfcn.h>
#include <stdarg.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/timex.h>
#include <sys/stat.h>
#include <net/if.h>
#include <linux/ptp_clock.h>
#include <linux/net_tstamp.h>
#include <linux/sockios.h>
#include <linux/ethtool.h>
/*****************************************************************************/
static bool did_init = false;
static const char ptp_dev[] = "/dev/ptp";
static timespec start_ts = {1600000000, 0};
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
#define orgFunc(name, ...) _##name(__VA_ARGS__)
sysFuncDec(int, clock_gettime, clockid_t, timespec *);
sysFuncDec(int, clock_settime, clockid_t, const timespec *);
sysFuncDec(int, clock_adjtime, clockid_t, timex *);
sysFuncDec(int, open, const char *, int, ...);
sysFuncDec(int, __open_2, const char *, int);
sysFuncDec(int, ioctl, int, unsigned long, ...) throw();
// glibc 'stat' fucntion
sysFuncDec(int, stat, const char *, struct stat *) throw();
sysFuncDec(int, stat64, const char *, struct stat64 *) throw();
sysFuncDec(int, __xstat, int, const char *, struct stat *);
sysFuncDec(int, __xstat64, int, const char *, struct stat64 *);
__attribute__((constructor))
static void initPtpSim(void)
{
    if(did_init)
        return;
    bool fail = false;
    sysFuncAgn(int, clock_gettime, clockid_t, timespec *);
    sysFuncAgn(int, clock_settime, clockid_t, const timespec *);
    sysFuncAgn(int, clock_adjtime, clockid_t, timex *);
    sysFuncAgn(int, open, const char *, int, ...);
    sysFuncAgn(int, __open_2, const char *, int);
    sysFuncAgn(int, ioctl, int, unsigned long, ...);
    sysFuncAgZ(int, stat, const char *, struct stat *);
    sysFuncAgZ(int, stat64, const char *, struct stat64 *);
    sysFuncAgn(int, __xstat, int, const char *, struct stat *);
    sysFuncAgn(int, __xstat64, int, const char *, struct stat64 *);
    if(fail)
        fprintf(stderr, "Fail obtain address of functions\n");
    did_init = true;
}
static inline bool isPhc(const char *name)
{
    return name != nullptr && strncmp(ptp_dev, name, sizeof(ptp_dev) - 1) == 0;
}
/*****************************************************************************/
int clock_gettime(clockid_t id, timespec *ts)
{
    if(id < 0) {
        ts->tv_sec = start_ts.tv_sec++;
        ts->tv_nsec = start_ts.tv_nsec;
        return 0;
    }
    return orgFunc(clock_gettime, id, ts);
}
int clock_settime(clockid_t id, const timespec *)
{
    if(id < 0)
        start_ts.tv_sec = 1;
    return 0;
}
int clock_adjtime(clockid_t, timex *)
{
    return TIME_OK;
}
int open(const char *name, int flags, ...)
{
    // Skip PHC clocks
    if(isPhc(name))
        return 0;
    mode_t mode = 0;
    if((flags & O_CREAT) == O_CREAT || (flags & O_TMPFILE) == O_TMPFILE) {
        va_list ap;
        va_start(ap, flags);
        mode = va_arg(ap, mode_t);
        va_end(ap);
    }
    return orgFunc(open, name, flags, mode);
}
int __open_2(const char *name, int flags)
{
    // Skip PHC clocks
    if(isPhc(name))
        return 0;
    return orgFunc(__open_2, name, flags);
}
int ioctl(int fd, unsigned long rq, ...) throw()
{
    va_list ap;
    va_start(ap, rq);
    void *arg = va_arg(ap, void *);
    va_end(ap);
    ifreq *ifr = (ifreq *)arg;
    switch(rq) {
        case SIOCGIFHWADDR:
            memcpy(ifr->ifr_hwaddr.sa_data, "\x1\x2\x3\x4\x5\x6", 6);
            break;
        case SIOCGIFINDEX:
            ifr->ifr_ifindex = 0;
            break;
        case SIOCETHTOOL: {
            ethtool_ts_info *info = (ethtool_ts_info *)ifr->ifr_data;
            info->phc_index = 0;
            info->so_timestamping = SOF_TIMESTAMPING_SOFTWARE |
                SOF_TIMESTAMPING_TX_SOFTWARE | SOF_TIMESTAMPING_RX_SOFTWARE |
                SOF_TIMESTAMPING_RAW_HARDWARE |
                SOF_TIMESTAMPING_TX_HARDWARE | SOF_TIMESTAMPING_RX_HARDWARE;
            info->tx_types = HWTSTAMP_TX_ON;
            info->rx_filters = 1 << HWTSTAMP_FILTER_NONE | 1 << HWTSTAMP_FILTER_ALL;
            break;
        }
        case PTP_CLOCK_GETCAPS: {
            ptp_clock_caps *cps = (ptp_clock_caps *)arg;
            memset(cps, 0, sizeof(ptp_clock_caps));
            cps->max_adj = 0xffffff;
            cps->n_pins = 1;
            break;
        }
        default:
            return orgFunc(ioctl, fd, rq, arg);
    }
    return 0;
}
/*****************************************************************************/
#define STAT_RET(nm)\
    if(_##nm != nullptr)\
        return _##nm(name, sp);\
    return ___x##nm(3, name, sp)
#define STAT_BODY\
    if(sp != nullptr && isPhc(name)) {sp->st_mode = S_IFCHR; return 0;}
int stat(const char *name, struct stat *sp) throw()
{
    STAT_BODY;
    STAT_RET(stat);
}
int stat64(const char *name, struct stat64 *sp) throw()
{
    STAT_BODY;
    STAT_RET(stat64);
}
int __xstat(int ver, const char *name, struct stat *sp)
{
    STAT_BODY;
    return orgFunc(__xstat, ver, name, sp);
}
int __xstat64(int ver, const char *name, struct stat64 *sp)
{
    STAT_BODY;
    return orgFunc(__xstat64, ver, name, sp);
}
