/* SPDX-License-Identifier: LGPL-3.0-or-later */

/** @file
 * @brief Read network interface information and retrieve the PTP information
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include "err.h"
#include "ptp.h"

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
        PMC_PERROR("SIOCGIFHWADDR");
        close(fd);
        return false;
    }
    m_mac.setBin(ifr.ifr_hwaddr.sa_data, EUI48);
    ethtool_ts_info info = {0};
    info.cmd = ETHTOOL_GET_TS_INFO;
    info.phc_index = -1;
    ifr.ifr_data = (char *)&info;
    if(ioctl(fd, SIOCETHTOOL, &ifr) == -1) {
        PMC_PERROR("SIOCETHTOOL");
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
        PMC_PERROR("socket");
        return false;
    }
    ifreq ifr = {0};
    // ifName is shorter than IFNAMSIZ
    strcpy(ifr.ifr_name, ifName.c_str());
    if(ioctl(fd, SIOCGIFINDEX, &ifr) == -1) {
        PMC_PERROR("SIOCGIFINDEX");
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
        PMC_PERROR("socket");
        return false;
    }
    ifreq ifr = {0};
    ifr.ifr_ifindex = ifIndex;
    if(ioctl(fd, SIOCGIFNAME, &ifr) == -1) {
        PMC_PERROR("SIOCGIFNAME");
        close(fd);
        return false;
    }
    m_ifName = ifr.ifr_name;
    m_ifIndex = ifIndex;
    return initPtp(fd, ifr);
}
PtpClock::PtpClock(int ptpIndex) : m_ptpIndex(ptpIndex), m_isInit(false)
{
    std::string dev = "/dev/ptp";
    dev += std::to_string(ptpIndex);
    m_fd = open(dev.c_str(), O_RDWR);
    if(m_fd < 0) {
        PMC_ERRORA("opening %s: %m", dev.c_str());
        m_clkId = -1;
        return;
    }
    m_clkId = get_clockid_fd(m_fd);
    if(m_clkId == -1) {
        PMC_ERROR("failed to read clock id");
        return;
    }
    m_ptpDevice = dev;
    m_isInit = true;
}
PtpClock::~PtpClock()
{
    if(m_fd >= 0)
        close(m_fd);
}
