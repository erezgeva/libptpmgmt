/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/** @file
 * @brief Read network interface information and retrieve the PTP information
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#include <cmath>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <linux/ptp_clock.h>
#include "err.h"
#include "ptp.h"

namespace ptpmgmt
{

#ifdef __GNUC__
#define PURE __attribute__((pure))
#else
#define PURE
#endif
static inline clockid_t get_clockid_fd(int fd) PURE;

const char ptp_dev[] = "/dev/ptp";

#ifndef NSEC_PER_SEC
#define NSEC_PER_SEC 1000000000L
#endif

// man netdevice
// From linux/posix-timers.h
#define CPUCLOCK_MAX      3
#define CLOCKFD           CPUCLOCK_MAX
#define FD_TO_CLOCKID(fd) ((~(clockid_t) (fd) << 3) | CLOCKFD)

//==========================================================================//
static inline clockid_t get_clockid_fd(int fd)
{
    return FD_TO_CLOCKID(fd);
}

bool IfInfo::initPtp(int fd, ifreq &ifr)
{
    /* retrieve corresponding MAC */
    if(ioctl(fd, SIOCGIFHWADDR, &ifr) == -1) {
        PTPMGMT_PERROR("SIOCGIFHWADDR");
        close(fd);
        return false;
    }
    m_mac.setBin(ifr.ifr_hwaddr.sa_data, EUI48);
    ethtool_ts_info info = {0};
    info.cmd = ETHTOOL_GET_TS_INFO;
    info.phc_index = NO_SUCH_PTP;
    ifr.ifr_data = (char *)&info;
    if(ioctl(fd, SIOCETHTOOL, &ifr) == -1) {
        PTPMGMT_PERROR("SIOCETHTOOL");
        close(fd);
        return false;
    }
    close(fd);
    m_ptpIndex = info.phc_index;
    m_isInit = true;
    return true;
}
bool IfInfo::initUsingName(const std::string &ifName)
{
    if(m_isInit || ifName.empty() || ifName.length() >= IFNAMSIZ)
        return false;
    int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if(fd < 0) {
        PTPMGMT_PERROR("socket");
        return false;
    }
    ifreq ifr = {0};
    // ifName is shorter than IFNAMSIZ
    strcpy(ifr.ifr_name, ifName.c_str());
    if(ioctl(fd, SIOCGIFINDEX, &ifr) == -1) {
        PTPMGMT_PERROR("SIOCGIFINDEX");
        close(fd);
        return false;
    }
    m_ifName = ifName;
    m_ifIndex = ifr.ifr_ifindex;
    return initPtp(fd, ifr);
}
bool IfInfo::initUsingIndex(int ifIndex)
{
    if(m_isInit)
        return false;
    int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if(fd < 0) {
        PTPMGMT_PERROR("socket");
        return false;
    }
    ifreq ifr = {0};
    ifr.ifr_ifindex = ifIndex;
    if(ioctl(fd, SIOCGIFNAME, &ifr) == -1) {
        PTPMGMT_PERROR("SIOCGIFNAME");
        close(fd);
        return false;
    }
    m_ifName = ifr.ifr_name;
    m_ifIndex = ifIndex;
    return initPtp(fd, ifr);
}
void ClockTime::formFloat(long double _seconds)
{
    seconds = floorl(_seconds);
    nanoseconds = (_seconds - seconds) * NSEC_PER_SEC;
}
long double ClockTime::toFloat() const
{
    return (long double)nanoseconds / NSEC_PER_SEC + seconds;
}
int64_t ClockTime::toNanoseconds() const
{
    return seconds + NSEC_PER_SEC * nanoseconds;
}
void ClockTime::fromNanoseconds(int64_t nanoseconds)
{
    auto d = div((long long)nanoseconds, (long long)NSEC_PER_SEC);
    seconds = d.quot;
    nanoseconds = d.rem;
}
ClockTime &ClockTime::add(const ClockTime &ts)
{
    seconds += ts.seconds;
    nanoseconds += ts.nanoseconds;
    // Should loop once at most
    while(nanoseconds >= NSEC_PER_SEC) {
        nanoseconds -= NSEC_PER_SEC;
        seconds++;
    }
    return *this;
}
ClockTime &ClockTime::subt(const ClockTime &ts)
{
    seconds -= ts.seconds;
    // Should loop once at most
    while(nanoseconds < ts.nanoseconds) {
        nanoseconds += NSEC_PER_SEC;
        seconds--;
    }
    nanoseconds -= ts.nanoseconds;
    // Should not happen
    while(nanoseconds >= NSEC_PER_SEC) {
        nanoseconds -= NSEC_PER_SEC;
        seconds++;
    }
    return *this;
}
bool PtpClock::isChar(const std::string &file)
{
    struct stat sb;
    return stat(file.c_str(), &sb) == 0 && (sb.st_mode & S_IFMT) == S_IFCHR;
}
bool PtpClock::init(const char *device, bool readonly)
{
    int flags = readonly ? O_RDONLY : O_RDWR;
    int fd = open(device, flags);
    if(fd < 0) {
        PTPMGMT_ERRORA("opening %s: %m", device);
        return false;
    }
    clockid_t cid = get_clockid_fd(fd);
    if(cid == CLOCK_INVALID) {
        PTPMGMT_ERROR("failed to read clock id");
        close(fd);
        return false;
    }
    struct timespec ts;
    if(clock_gettime(cid, &ts) != 0) {
        close(fd);
        return false;
    }
    m_fd = fd;
    m_clkId = cid;
    m_isInit = true;
    return true;
}
bool PtpClock::initSysClock()
{
    if(m_isInit)
        return false;
    m_clkId = CLOCK_REALTIME;
    m_isInit = true;
    return true;
}
bool PtpClock::initUsingDevice(const std::string &device, bool readonly)
{
    if(m_isInit)
        return false;
    char file[PATH_MAX];
    if(!isChar(device)) {
        std::string dev;
        if(device.find('/') != std::string::npos)
            return false;
        // A bare file name, lets try under '/dev/'
        dev = "/dev/";
        dev += device;
        // File can be symbolic link in the /dev folder
        if(!isChar(dev) || realpath(dev.c_str(), file) == nullptr)
            return false;
    } else if(realpath(device.c_str(), file) == nullptr)
        return false;
    if(init(file, readonly)) {
        m_device = file; // Store the realpath
        // Does this device have a ptp index?
        char *num; // Store number location
        if(strncmp(file, ptp_dev, sizeof(ptp_dev)) == 0 &&
            *(num = file + sizeof(ptp_dev)) != 0) {
            char *endptr;
            long ret = strtol(num, &endptr, 10);
            if(ret >= 0 && *endptr == 0 && ret < LONG_MAX)
                m_ptpIndex = ret;
        }
        return true;
    }
    return false;
}
bool PtpClock::initUsingIndex(int ptpIndex, bool readonly)
{
    if(m_isInit)
        return false;
    std::string dev = ptp_dev;
    dev += std::to_string(ptpIndex);
    if(isChar(dev) && init(dev.c_str(), readonly)) {
        m_ptpIndex = ptpIndex;
        m_device = dev;
        return true;
    }
    return false;
}
PtpClock::~PtpClock()
{
    if(m_fd >= 0)
        close(m_fd);
}

ClockTime PtpClock::getTime() const
{
    ClockTime ts;
    if(m_isInit) {
        struct timespec ts1;
        if(clock_gettime(m_clkId, &ts1) == 0)
            ts.fromTimespec(ts1);
    }
    return ts;
}
bool PtpClock::setTime(const ClockTime &ts) const
{
    if(m_isInit) {
        struct timespec ts1;
        ts.toTimespec(ts1);
        return clock_gettime(m_clkId, &ts1) == 0;
    }
    return false;
}
bool PtpClock::fetchCaps(PtPCaps &caps) const
{
    if(!m_isInit || m_fd < 0)
        return false;
    struct ptp_clock_caps c;
    if(ioctl(m_fd, PTP_CLOCK_GETCAPS, &c) == -1)
        return false;
    caps.max_ppb = c.max_adj;
    caps.num_alarm = c.n_alarm;
    caps.num_external_channels = c.n_ext_ts;
    caps.num_periodic_sig = c.n_per_out;
    caps.pps = c.pps > 0;
    caps.num_pins = c.n_pins;
    caps.cross_timestamping = c.cross_timestamping > 0;
    caps.adjust_phase = c.adjust_phase > 0;
    return true;
}

}; /* namespace ptpmgmt */
