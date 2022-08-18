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
#include "comp.h"
#include "timeCvrt.h"

__PTPMGMT_NAMESPACE_BEGIN

// These are system/hardware based parameters
static bool fetchClockTicks = false;
static long clockTicks;
static long userTicks;

static inline const Timestamp_t toTs(const ptp_clock_time &pct) PURE;
static inline clockid_t get_clockid_fd(int fd) PURE;

const char ptp_dev[] = "/dev/ptp";
// parts per billion (ppb) = 10^9
// to scale parts per billion (ppm) = 10^6 * 2^16
const long double PPB_TO_SCALE_PPM = 65.536; // (2^16 / 1000)

// man netdevice
// From linux/posix-timers.h
#define CPUCLOCK_MAX      3
#define CLOCKFD           CPUCLOCK_MAX
#define FD_TO_CLOCKID(fd) ((~(clockid_t) (fd) << 3) | CLOCKFD)

//==========================================================================//
static inline const Timestamp_t toTs(const ptp_clock_time &pct)
{
    return { pct.sec, pct.nsec };
}
static inline void fromTs(ptp_clock_time &pct, const Timestamp_t &ts)
{
    pct.sec = ts.secondsField;
    pct.nsec = ts.nanosecondsField;
}
static inline clockid_t get_clockid_fd(int fd)
{
    return FD_TO_CLOCKID(fd);
}
static void calcTicks()
{
    if(!fetchClockTicks) {
        clockTicks = sysconf(_SC_CLK_TCK);
        if(clockTicks > 0)
            userTicks = (1000000 + clockTicks / 2) / clockTicks;
        else
            userTicks = 0;
        fetchClockTicks = true;
    }
}
static bool setOff(clockid_t clkId, timeval time)
{
    timex tmx = {0};
    // ADJ_NANO: Use nanoseconds instead of microseconds!
    tmx.modes = ADJ_SETOFFSET | ADJ_NANO;
    tmx.time = time;
    if(clock_adjtime(clkId, &tmx) != -1)
        return true;
    PTPMGMT_PERROR("ADJ_SETOFFSET");
    return false;
}
static bool setTimeFromTime(bool isInit, clockid_t from, clockid_t to)
{
    timespec ts;
    if(isInit && clock_gettime(from, &ts) == 0)
        return clock_settime(to, &ts) == 0;
    return false;
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
        return clock_settime(m_clkId, &ts1) == 0;
    }
    return false;
}
bool BaseClock::offsetClock(long double offset) const
{
    if(m_isInit) {
        time_t secs = floorl(offset);
        suseconds_t nsecs = (offset - secs) * NSEC_PER_SEC;
        return setOff(m_clkId, {secs, nsecs});
    }
    return false;
}
bool BaseClock::offsetClockNsec(int64_t offset) const
{
    if(m_isInit) {
        auto d = div((long long)offset, (long long)NSEC_PER_SEC);
        while(d.rem < 0) {
            d.quot--;
            d.rem += NSEC_PER_SEC;
        };
        return setOff(m_clkId, {d.quot, d.rem});
    }
    return false;
}
long double BaseClock::getFreq() const
{
    if(m_isInit) {
        timex tmx = {0};
        if(clock_adjtime(m_clkId, &tmx) != -1)
            return (long double)tmx.freq / PPB_TO_SCALE_PPM +
                freqAddTicks(tmx.tick);
        PTPMGMT_PERROR("clock_adjtime");
    }
    return 0;
}
bool BaseClock::setFreq(long double freq) const
{
    if(m_isInit) {
        timex tmx = {0};
        tmx.modes = ADJ_FREQUENCY;
        freqModeTicks(freq, tmx);
        tmx.freq = (long)(freq * PPB_TO_SCALE_PPM);
        if(clock_adjtime(m_clkId, &tmx) != -1)
            return true;
        PTPMGMT_PERROR("ADJ_FREQUENCY");
    }
    return false;
}
SysClock::SysClock() : BaseClock(CLOCK_REALTIME) {}
long double SysClock::freqAddTicks(long long tick) const
{
    calcTicks();
    if(tick == 0 || userTicks == 0)
        return 0;
    return 1e3 * clockTicks * (tick - userTicks);
}
void SysClock::freqModeTicks(long double &freq, timex &tmx) const
{
    calcTicks();
    if(userTicks != 0) {
        tmx.modes |= ADJ_TICK;
        tmx.tick = round(freq / 1e3 / clockTicks) + userTicks;
        freq -= 1e3 * clockTicks * (tmx.tick - userTicks);
    }
}
PtpClock::~PtpClock()
{
    if(m_fd >= 0)
        close(m_fd);
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
        PTPMGMT_PERROR("clock_gettime");
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
bool PtpClock::setTimeFromSys() const
{
    return setTimeFromTime(m_isInit, CLOCK_REALTIME, m_clkId);
}
bool PtpClock::setTimeToSys() const
{
    return setTimeFromTime(m_isInit, m_clkId, CLOCK_REALTIME);
}
bool PtpClock::fetchCaps(PtpCaps_t &caps) const
{
    if(!m_isInit)
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
    if(!m_isInit)
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
    if(!m_isInit)
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
    if(!m_isInit)
        return false;
    unsigned long rid;
    #ifdef PTP_EXTTS_REQUEST2
    rid = PTP_EXTTS_REQUEST2;
    #else
    if((PTP_EXTERN_TS_STRICT & flags) > 0) {
        PTPMGMT_ERROR("Old kernel, PTP_EXTERN_TS_STRICT flag is not supported");
        return false;
    }
    rid = PTP_EXTTS_REQUEST;
    #endif
    ptp_extts_request req = { .index = index };
    req.flags = flags | PTP_ENABLE_FEATURE;
    if(ioctl(m_fd, rid, &req) == -1) {
        PTPMGMT_PERROR("PTP_EXTTS_REQUEST");
        return false;
    }
    return true;
}
bool PtpClock::ExternTSDisable(unsigned int index) const
{
    if(!m_isInit)
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
    if(!m_isInit)
        return false;
    ptp_extts_event ent;
    if(read(m_fd, &ent, sizeof ent) != sizeof ent)
        return false;
    event = { ent.index, toTs(ent.t) };
    return true;
}
const size_t PTP_BUF_TIMESTAMPS = 30; // From Linux kernel ptp_private.h
int PtpClock::readEvents(std::vector<PtpEvent_t> &events, size_t max) const
{
    if(!m_isInit)
        return -1;
    if(max == 0)
        max = PTP_BUF_TIMESTAMPS;
    else
        max = std::min(max, PTP_BUF_TIMESTAMPS);
    ptp_extts_event ents[max];
    ssize_t cnt = read(m_fd, ents, sizeof ents);
    if(cnt < 0)
        return -1;
    else if(cnt == 0)
        return 0;
    auto d = div(cnt, sizeof(ptp_extts_event));
    if(d.rem != 0)
        return -1;
    ptp_extts_event *ent = ents;
    for(int i = 0; i < d.quot; i++, ent++)
        events.push_back({ent->index, toTs(ent->t)});
    return d.quot;
}
bool PtpClock::setPinPeriod(unsigned int index, PtpPinPeriodDef_t times,
    uint8_t flags) const
{
    if(!m_isInit)
        return false;
    unsigned long rid;
    struct ptp_perout_request req = {0};
    #ifndef PTP_PEROUT_REQUEST2
    if(flags != 0) {
        PTPMGMT_ERROR("Old kernel, flags are not supported");
        return false;
    }
    rid = PTP_PEROUT_REQUEST;
    req.flags = 0;
    #else
    rid = PTP_PEROUT_REQUEST2;
    req.flags = flags;
    if((PTP_PERIOD_WIDTH & flags) > 0)
        fromTs(req.on, times.width);
    if((PTP_PERIOD_PHASE & flags) > 0)
        fromTs(req.phase, times.phase);
    else
    #endif
    {
        timespec ts;
        if(clock_gettime(m_clkId, &ts)) {
            PTPMGMT_PERROR("clock_gettime");
            return false;
        }
        req.start.sec = ts.tv_sec + 2;
        req.start.nsec = 0;
    }
    req.index = index;
    fromTs(req.period, times.period);
    if(ioctl(m_fd, rid, &req) == -1) {
        PTPMGMT_PERROR("PTP_PEROUT_REQUEST");
        return false;
    }
    return true;
}
bool PtpClock::setPtpPpsEvent(bool enable) const
{
    if(!m_isInit)
        return false;
    int req = enable ? 1 : 0;
    if(ioctl(m_fd, PTP_ENABLE_PPS, &req) == -1) {
        PTPMGMT_PERROR("PTP_ENABLE_PPS");
        return false;
    }
    return true;
}
bool PtpClock::samplePtpSys(size_t count,
    std::vector<PtpSample_t> &samples) const
{
    if(!m_isInit)
        return false;
    if(count == 0 || count > PTP_MAX_SAMPLES)
        return false;
    ptp_sys_offset req = {0};
    req.n_samples = count;
    if(ioctl(m_fd, PTP_SYS_OFFSET, &req) == -1) {
        PTPMGMT_PERROR("PTP_SYS_OFFSET");
        return false;
    }
    ptp_clock_time *pct = req.ts;
    for(unsigned i = 0; i < req.n_samples; i++)
        samples.push_back({
        toTs(*pct++), // System clock
        toTs(*pct++)}); // PHP clock
    return true;
}
bool PtpClock::extSamplePtpSys(size_t count,
    std::vector<PtpSampleExt_t> &samples) const
{
    #ifdef PTP_SYS_OFFSET_EXTENDED
    if(!m_isInit)
        return false;
    if(count == 0 || count > PTP_MAX_SAMPLES)
        return false;
    ptp_sys_offset_extended req = {0};
    req.n_samples = count;
    if(ioctl(m_fd, PTP_SYS_OFFSET_EXTENDED, &req) == -1)
        return false;
    for(unsigned i = 0; i < req.n_samples; i++) {
        ptp_clock_time *pct = req.ts[i];
        samples.push_back({
            toTs(*pct++), // System clock before
            toTs(*pct++), // PHP clock
            toTs(*pct++)}); // System clock after
    }
    return true;
    #else
    PTPMGMT_ERROR("Old kernel, PTP_SYS_OFFSET_EXTENDED ioctl is not supported");
    return false;
    #endif
}
bool PtpClock::preciseSamplePtpSys(PtpSamplePrecise_t &sample) const
{
    if(!m_isInit)
        return false;
    ptp_sys_offset_precise req = {0};
    if(ioctl(m_fd, PTP_SYS_OFFSET_PRECISE, &req) == -1)
        return false;
    sample.phcClk = toTs(req.device);
    sample.sysClk = toTs(req.sys_realtime);
    sample.monoClk = toTs(req.sys_monoraw);
    return true;
}

__PTPMGMT_NAMESPACE_END
