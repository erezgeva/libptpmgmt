/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/** @file
 * @brief Read network interface information and retrieve the PTP information
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/sockios.h>
#include <linux/ptp_clock.h>
#include "err.h"
#include "ptp.h"
#include "comp.h"

namespace ptpmgmt
{

static inline clockid_t get_clockid_fd(int fd) PURE;

const char ptp_dev[] = "/dev/ptp";

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
    ethtool_ts_info info = { .cmd = ETHTOOL_GET_TS_INFO };
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
SysClock::SysClock() : BaseClock(CLOCK_REALTIME, true) {}
Timestamp_t BaseClock::getTime() const
{
    if(m_isInit) {
        timespec ts1;
        if(clock_gettime(m_clkId, &ts1) == 0)
            return ts1;
    }
    return 0;
}
bool BaseClock::setTime(const Timestamp_t &ts) const
{
    if(m_isInit) {
        timespec ts1 = ts;
        return clock_gettime(m_clkId, &ts1) == 0;
    }
    return false;
}
bool PtpClock::isCharFile(const std::string &file)
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
    timespec ts;
    if(clock_gettime(cid, &ts) != 0) {
        close(fd);
        return false;
    }
    m_fd = fd;
    m_clkId = cid;
    m_isInit = true;
    return true;
}
bool PtpClock::initUsingDevice(const std::string &device, bool readonly)
{
    if(m_isInit)
        return false;
    char file[PATH_MAX];
    if(!isCharFile(device)) {
        std::string dev;
        if(device.find('/') != std::string::npos)
            return false;
        // A bare file name, lets try under '/dev/'
        dev = "/dev/";
        dev += device;
        // File can be symbolic link in the /dev folder
        if(!isCharFile(dev) || realpath(dev.c_str(), file) == nullptr)
            return false;
    } else if(realpath(device.c_str(), file) == nullptr)
        return false;
    if(init(file, readonly)) {
        m_device = file; // Store the realpath
        // Does this device have a ptp index?
        char *num; // Store number location
        if(strncmp(file, ptp_dev, sizeof ptp_dev) == 0 &&
            *(num = file + sizeof ptp_dev) != 0) {
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
    if(isCharFile(dev) && init(dev.c_str(), readonly)) {
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
bool PtpClock::fetchCaps(PtpCaps_t &caps) const
{
    if(!m_isInit || m_fd < 0)
        return false;
    ptp_clock_caps cps = {0};
    if(ioctl(m_fd, PTP_CLOCK_GETCAPS, &cps) == -1) {
        PTPMGMT_PERROR("PTP_CLOCK_GETCAPS");
        return false;
    }
    caps.max_ppb = cps.max_adj;
    caps.num_alarm = cps.n_alarm;
    caps.num_external_channels = cps.n_ext_ts;
    caps.num_periodic_sig = cps.n_per_out;
    caps.pps = cps.pps > 0;
    caps.num_pins = cps.n_pins;
    caps.cross_timestamping = cps.cross_timestamping > 0;
    #ifdef PTP_CLOCK_GETCAPS2
    caps.adjust_phase = cps.adjust_phase > 0;
    #else
    caps.adjust_phase = false;
    #endif
    return true;
}
bool PtpClock::readPin(unsigned int index, PtpPin_t &pin) const
{
    if(!m_isInit || m_fd < 0)
        return false;
    ptp_pin_desc desc = {0};
    desc.index = index;
    if(ioctl(m_fd, PTP_PIN_GETFUNC, &desc) == -1) {
        PTPMGMT_PERROR("PTP_PIN_GETFUNC");
        return false;
    }
    pin.index = index;
    pin.description = desc.name;
    pin.channel = desc.chan;
    switch(desc.func) {
        case PTP_PF_NONE:
            pin.functional = PTP_PIN_UNUSED;
            break;
        case PTP_PF_EXTTS:
            pin.functional = PTP_PIN_EXTERNAL_TS;
            break;
        case PTP_PF_PEROUT:
            pin.functional = PTP_PIN_PERIODIC_OUT;
            break;
        case PTP_PF_PHYSYNC:
            pin.functional = PTP_PIN_PHY_SYNC;
            break;
        default:
            PTPMGMT_ERRORA("readPin unknown functional %d", desc.func);
            return false;
    };
    return true;
}
bool PtpClock::writePin(PtpPin_t &pin) const
{
    if(!m_isInit || m_fd < 0)
        return false;
    ptp_pin_desc desc;
    switch(pin.functional) {
        case PTP_PIN_UNUSED:
            desc.func = PTP_PF_NONE;
            break;
        case PTP_PIN_EXTERNAL_TS:
            desc.func = PTP_PF_EXTTS;
            break;
        case PTP_PIN_PERIODIC_OUT:
            desc.func = PTP_PF_PEROUT;
            break;
        case PTP_PIN_PHY_SYNC:
            desc.func = PTP_PF_PHYSYNC;
            break;
        default:
            PTPMGMT_ERRORA("writePin wrong functional %d", pin.functional);
            return false;
    };
    desc.index = pin.index;
    desc.chan = pin.channel;
    if(ioctl(m_fd, PTP_PIN_SETFUNC, &desc) == -1) {
        PTPMGMT_PERROR("PTP_PIN_SETFUNC");
        return false;
    }
    return true;
}
bool PtpClock::ExternTSEbable(unsigned int index, uint8_t flags) const
{
    if(!m_isInit || m_fd < 0)
        return false;
    ptp_extts_request req = { .index = index };
    req.flags = flags | PTP_ENABLE_FEATURE;
    if(ioctl(m_fd, PTP_EXTTS_REQUEST, &req) == -1) {
        PTPMGMT_PERROR("PTP_EXTTS_REQUEST");
        return false;
    }
    return true;
}
bool PtpClock::ExternTSDisable(unsigned int index) const
{
    if(!m_isInit || m_fd < 0)
        return false;
    ptp_extts_request req = { .index = index };
    if(ioctl(m_fd, PTP_EXTTS_REQUEST, &req) == -1) {
        PTPMGMT_PERROR("PTP_EXTTS_REQUEST");
        return false;
    }
    return true;
}
bool PtpClock::readEvent(PtpEvent_t &event) const
{
    if(!m_isInit || m_fd < 0)
        return false;
    ptp_extts_event ent;
    if(read(m_fd, &ent, sizeof ent) != sizeof ent)
        return false;
    event = { ent.index, { ent.t.sec, ent.t.nsec } };
    return true;
}
int PtpClock::readEvents(std::vector<PtpEvent_t> &events, int max) const
{
    if(!m_isInit || m_fd < 0)
        return -1;
    if(max < 0 || max > 30)
        max = 30;
    else if(max == 0)
        return 0;
    ptp_extts_event ents[max];
    int cnt = read(m_fd, ents, sizeof ents);
    if(cnt < 0)
        return -1;
    else if(cnt == 0)
        return 0;
    auto d = div(cnt, sizeof(ptp_extts_event));
    if(d.rem > 0)
        return -1;
    for(int i = 0; i < d.quot; i++)
        events.push_back({ents[i].index, { ents[i].t.sec, ents[i].t.nsec }});
    return d.quot;
}

}; /* namespace ptpmgmt */
