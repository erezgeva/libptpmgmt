/* SPDX-License-Identifier: LGPL-3.0-or-later */

/** @file
 * @brief Read network interface information and retrieve the PTP information
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include "msg.h"
#include "ptp.h"
// man netdevice

static const size_t max_ifName = sizeof(((ifreq *)nullptr)->ifr_name);

// From linux/posix-timers.h
#define CPUCLOCK_MAX      3
#define CLOCKFD           CPUCLOCK_MAX
#define FD_TO_CLOCKID(fd) ((~(clockid_t) (fd) << 3) | CLOCKFD)

//==========================================================================//
static inline clockid_t get_clockid_fd(int fd)
{
    return FD_TO_CLOCKID(fd);
}

bool ifInfo::initPtp(int fd, ifreq &ifr)
{
    /* retrieve corresponding MAC */
    if(ioctl(fd, SIOCGIFHWADDR, &ifr) == -1) {
        perror("SIOCGIFHWADDR");
        close(fd);
        return false;
    }
    m_mac.set(ifr.ifr_hwaddr.sa_data, EUI48);
    ethtool_ts_info info = {
        .cmd = ETHTOOL_GET_TS_INFO,
        .phc_index = -1,
    };
    ifr.ifr_data = (char *)&info;
    if(ioctl(fd, SIOCETHTOOL, &ifr) == -1) {
        perror("SIOCETHTOOL");
        close(fd);
        return false;
    }
    close(fd);
    m_ptpIndex = info.phc_index;
    m_isInit = true;
    return true;
}
bool ifInfo::initUsingName(const std::string ifName)
{
    if(m_isInit || ifName.empty() || ifName.length() > max_ifName)
        return false;
    int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if(fd < 0) {
        perror("socket");
        return false;
    }
    ifreq ifr = {0};
    strncpy(ifr.ifr_name, ifName.c_str(), max_ifName);
    if(ioctl(fd, SIOCGIFINDEX, &ifr) == -1) {
        perror("SIOCGIFINDEX");
        close(fd);
        return false;
    }
    m_ifName = ifName;
    m_ifIndex = ifr.ifr_ifindex;
    return initPtp(fd, ifr);
}
bool ifInfo::initUsingIndex(int ifIndex)
{
    if(m_isInit)
        return false;
    int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if(fd < 0) {
        perror("socket");
        return false;
    }
    ifreq ifr = {0};
    ifr.ifr_ifindex = ifIndex;
    if(ioctl(fd, SIOCGIFNAME, &ifr) == -1) {
        perror("SIOCGIFNAME");
        close(fd);
        return false;
    }
    m_ifName = ifr.ifr_name;
    m_ifIndex = ifIndex;
    return initPtp(fd, ifr);
}
ptpClock::ptpClock(int ptpIndex) : m_ptpIndex(ptpIndex), m_isInit(false)
{
    std::string dev = "/dev/ptp";
    dev += std::to_string(ptpIndex);
    m_fd = open(dev.c_str(), O_RDWR);
    if(m_fd < 0) {
        fprintf(stderr, "opening %s: %m\n", dev.c_str());
        m_clkId = -1;
        return;
    }
    m_clkId = get_clockid_fd(m_fd);
    if(m_clkId == -1) {
        fprintf(stderr, "failed to read clock id\n");
        return;
    }
    m_ptpDevice = dev;
    m_isInit = true;
}
ptpClock::~ptpClock()
{
    if(m_fd >= 0)
        close(m_fd);
}
