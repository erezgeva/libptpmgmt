/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Read network interface information and retrieve the PTP information
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
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
#include <linux/version.h>
#include "ptp.h"
#include "comp.h"
#include "timeCvrt.h"

__PTPMGMT_NAMESPACE_BEGIN

// These are system/hardware based parameters
static bool fetchClockTicks = false;
static long clockTicks;
static long userTicks;

static inline const Timestamp_t toTs(const ptp_clock_time &pct) PURE;
static inline clockid_t make_process_cpuclock(unsigned int pid,
    clockid_t clock) PURE;
static inline clockid_t fd_to_clockid(int fd) PURE;
#if 0
static inline int clockid_to_fd(clockid_t clk) PURE;
#endif

const char ptp_dev[] = "/dev/ptp";
// parts per billion (ppb) = 10^9
// to scale parts per billion (ppm) = 10^6 * 2^16
const float_freq PPB_TO_SCALE_PPM = 65.536; // (2^16 / 1000)

// From kernel include/linux/posix-timers.h
const clockid_t CLOCKFD = 3;
static inline clockid_t make_process_cpuclock(unsigned int pid, clockid_t clock)
{
    return ((~pid) << 3) | clock;
}
static inline clockid_t fd_to_clockid(int fd)
{
    return make_process_cpuclock((unsigned int) fd, CLOCKFD);
}
#if 0
static inline int clockid_to_fd(clockid_t clk)
{
    return ~(clk >> 3);
}
#endif

// From Kernel include/uapi/linux/timex.h
#ifndef ADJ_OFFSET
const int ADJ_OFFSET    = 0x0001;
#endif
#ifndef ADJ_FREQUENCY
const int ADJ_FREQUENCY = 0x0002;
#endif
#ifndef ADJ_SETOFFSET
const int ADJ_SETOFFSET = 0x0100;
#endif
#ifndef ADJ_NANO
const int ADJ_NANO      = 0x2000;
#endif
#ifndef ADJ_TICK
const int ADJ_TICK      = 0x4000;
#endif

// From kernel include/uapi/linux/ptp_clock.h
#ifndef PTP_MAX_SAMPLES
#define PTP_MAX_SAMPLES 25
#endif
#ifndef PTP_SYS_OFFSET_EXTENDED
#define PTP_SYS_OFFSET_EXTENDED _IOWR(PTP_CLK_MAGIC, 9, ptp_sys_offset_extended)
extern "C" {
    struct ptp_sys_offset_extended {
        unsigned int n_samples;
        unsigned int rsv[3];
        ptp_clock_time ts[PTP_MAX_SAMPLES][3];
    };
}
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(6,5,0)
extern "C" {
    struct next_ptp_clock_caps {
        int max_adj;
        int n_alarm;
        int n_ext_ts;
        int n_per_out;
        int pps;
        int n_pins;
        int cross_timestamping;
        int adjust_phase;
        int max_phase_adj;
        int rsv[11];
    };
}
#define ptp_clock_caps next_ptp_clock_caps
#endif
#ifndef PTP_EXTTS_REQUEST2
#define PTP_EXTTS_REQUEST2  _IOW(PTP_CLK_MAGIC, 11, ptp_extts_request)
#endif
#ifndef PTP_PEROUT_REQUEST2
#define PTP_PEROUT_REQUEST2 _IOW(PTP_CLK_MAGIC, 12, ptp_perout_request)
#endif
#if LINUX_VERSION_CODE < KERNEL_VERSION(5,9,0)
extern "C" {
    struct next_ptp_perout_request {
        union {
            ptp_clock_time start;
            ptp_clock_time phase;
        };
        ptp_clock_time period;
        unsigned int index;
        unsigned int flags;
        union {
            ptp_clock_time on;
            unsigned int rsv[4];
        };
    };
}
#define ptp_perout_request next_ptp_perout_request
#endif

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
static bool setTimeFromTime(bool isInit, clockid_t from, clockid_t to)
{
    timespec ts;
    if(!isInit) {
        PTPMGMT_ERROR("not initialized yet");
        return false;
    }
    if(clock_gettime(from, &ts) != 0) {
        PTPMGMT_ERROR_P("clock_gettime");
        return false;
    }
    if(clock_settime(to, &ts) != 0) {
        PTPMGMT_ERROR_P("clock_settime");
        return false;
    }
    PTPMGMT_ERROR_CLR;
    return true;
}
bool IfInfo::initPtp(int fd, ifreq &ifr)
{
    /* retrieve corresponding MAC */
    if(ioctl(fd, SIOCGIFHWADDR, &ifr) == -1) {
        PTPMGMT_ERROR_P("SIOCGIFHWADDR");
        close(fd);
        return false;
    }
    m_mac.setBin(ifr.ifr_hwaddr.sa_data, EUI48);
    ethtool_ts_info info = { .cmd = ETHTOOL_GET_TS_INFO };
    info.phc_index = NO_SUCH_PTP;
    ifr.ifr_data = (char *)&info;
    if(ioctl(fd, SIOCETHTOOL, &ifr) == -1) {
        PTPMGMT_ERROR_P("SIOCETHTOOL");
        close(fd);
        return false;
    }
    close(fd);
    m_ptpIndex = info.phc_index;
    m_isInit = true;
    PTPMGMT_ERROR_CLR;
    return true;
}
bool IfInfo::initUsingName(const std::string &ifName)
{
    if(m_isInit) {
        PTPMGMT_ERROR("Alreay initialized");
        return false;
    }
    if(ifName.empty() || ifName.length() >= IFNAMSIZ) {
        PTPMGMT_ERROR("missing interface");
        return false;
    }
    int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if(fd < 0) {
        PTPMGMT_ERROR_P("socket");
        return false;
    }
    ifreq ifr;
    memset(&ifr, 0, sizeof ifr);
    // ifName is shorter than IFNAMSIZ
    strcpy(ifr.ifr_name, ifName.c_str());
    if(ioctl(fd, SIOCGIFINDEX, &ifr) == -1) {
        PTPMGMT_ERROR_P("SIOCGIFINDEX");
        close(fd);
        return false;
    }
    m_ifName = ifName;
    m_ifIndex = ifr.ifr_ifindex;
    return initPtp(fd, ifr);
}
bool IfInfo::initUsingIndex(int ifIndex)
{
    if(m_isInit) {
        PTPMGMT_ERROR("Alreay initialized");
        return false;
    }
    int fd = socket(AF_UNIX, SOCK_DGRAM, 0);
    if(fd < 0) {
        PTPMGMT_ERROR_P("socket");
        return false;
    }
    ifreq ifr;
    memset(&ifr, 0, sizeof ifr);
    ifr.ifr_ifindex = ifIndex;
    if(ioctl(fd, SIOCGIFNAME, &ifr) == -1) {
        PTPMGMT_ERROR_P("SIOCGIFNAME");
        close(fd);
        return false;
    }
    m_ifName = ifr.ifr_name;
    m_ifIndex = ifIndex;
    return initPtp(fd, ifr);
}
Timestamp_t BaseClock::getTime() const
{
    if(!m_isInit) {
        PTPMGMT_ERROR("not initialized yet");
        return 0;
    }
    timespec ts1;
    if(clock_gettime(m_clkId, &ts1) != 0) {
        PTPMGMT_ERROR_P("clock_gettime");
        return 0;
    }
    PTPMGMT_ERROR_CLR;
    return ts1;
}
bool BaseClock::setTime(const Timestamp_t &ts) const
{
    if(!m_isInit) {
        PTPMGMT_ERROR("not initialized yet");
        return false;
    }
    timespec ts1 = ts;
    if(clock_settime(m_clkId, &ts1) != 0) {
        PTPMGMT_ERROR_P("clock_settime");
        return false;
    }
    PTPMGMT_ERROR_CLR;
    return true;
}
bool BaseClock::offsetClock(int64_t offset) const
{
    if(!m_isInit) {
        PTPMGMT_ERROR("not initialized yet");
        return false;
    }
    lldiv_t d = lldiv((long long)offset, (long long)NSEC_PER_SEC);
    while(d.rem < 0) {
        d.quot--;
        d.rem += NSEC_PER_SEC;
    };
    timex tmx = {0};
    // ADJ_NANO: Use nanoseconds instead of microseconds!
    tmx.modes = ADJ_SETOFFSET | ADJ_NANO;
    tmx.time = {d.quot, d.rem};
    if(clock_adjtime(m_clkId, &tmx) != 0) {
        PTPMGMT_ERROR_P("ADJ_SETOFFSET");
        return false;
    }
    PTPMGMT_ERROR_CLR;
    return true;
}
float_freq BaseClock::getFreq() const
{
    if(!m_isInit) {
        PTPMGMT_ERROR("not initialized yet");
        return 0;
    }
    timex tmx = {0};
    if(clock_adjtime(m_clkId, &tmx) != 0) {
        PTPMGMT_ERROR_P("clock_adjtime");
        return 0;
    }
    float_freq add = 0;
    if(m_freq) {
        calcTicks();
        if(userTicks != 0 && tmx.tick != 0)
            add = 1e3 * clockTicks * (tmx.tick - userTicks);
    }
    PTPMGMT_ERROR_CLR;
    return (float_freq)tmx.freq / PPB_TO_SCALE_PPM + add;
}
bool BaseClock::setFreq(float_freq freq) const
{
    if(!m_isInit) {
        PTPMGMT_ERROR("not initialized yet");
        return 0;
    }
    timex tmx = {0};
    tmx.modes = ADJ_FREQUENCY;
    if(m_freq) {
        calcTicks();
        if(userTicks != 0) {
            tmx.modes |= ADJ_TICK;
            tmx.tick = round(freq / 1e3 / clockTicks) + userTicks;
            freq -= 1e3 * clockTicks * (tmx.tick - userTicks);
        }
    }
    tmx.freq = (long)(freq * PPB_TO_SCALE_PPM);
    if(clock_adjtime(m_clkId, &tmx) != 0) {
        PTPMGMT_ERROR_P("ADJ_FREQUENCY");
        return false;
    }
    PTPMGMT_ERROR_CLR;
    return true;
}
bool BaseClock::setPhase(int64_t offset) const
{
    if(!m_isInit) {
        PTPMGMT_ERROR("not initialized yet");
        return false;
    }
    timex tmx = {0};
    // ADJ_NANO: Use nanoseconds instead of microseconds!
    tmx.modes = ADJ_OFFSET | ADJ_NANO;
    tmx.offset = (long)offset;
    if(clock_adjtime(m_clkId, &tmx) != 0) {
        PTPMGMT_ERROR_P("ADJ_OFFSET");
        return false;
    }
    PTPMGMT_ERROR_CLR;
    return true;
}
SysClock::SysClock() : BaseClock(CLOCK_REALTIME, true) {}
PtpClock::~PtpClock()
{
    if(m_fd >= 0)
        close(m_fd);
}
bool PtpClock::isCharFile(const std::string &file)
{
    struct stat sb;
    if(stat(file.c_str(), &sb) != 0) {
        PTPMGMT_ERROR_P("stat");
        return false;
    }
    PTPMGMT_ERROR_CLR;
    return (sb.st_mode & S_IFMT) == S_IFCHR;
}
bool PtpClock::init(const char *device, bool readonly)
{
    int flags = readonly ? O_RDONLY : O_RDWR;
    int fd = open(device, flags);
    if(fd < 0) {
        PTPMGMT_ERROR_P("opening %s: %m", device);
        return false;
    }
    clockid_t cid = fd_to_clockid(fd);
    if(cid == CLOCK_INVALID) {
        PTPMGMT_ERROR("failed to read clock id");
        close(fd);
        return false;
    }
    timespec ts;
    if(clock_gettime(cid, &ts) != 0) {
        PTPMGMT_ERROR_P("clock_gettime");
        close(fd);
        return false;
    }
    m_fd = fd;
    m_clkId = cid;
    m_isInit = true;
    PTPMGMT_ERROR_CLR;
    return true;
}
bool PtpClock::initUsingDevice(const std::string &device, bool readonly)
{
    if(m_isInit) {
        PTPMGMT_ERROR("Alreay initialized");
        return false;
    }
    char file[PATH_MAX];
    if(!isCharFile(device)) {
        std::string dev;
        if(device.find('/') != std::string::npos) {
            PTPMGMT_ERROR("Wrong device '%s'", device.c_str());
            return false;
        }
        // A bare file name, lets try under '/dev/'
        dev = "/dev/";
        dev += device;
        // File can be symbolic link in the /dev folder
        if(!isCharFile(dev) || realpath(dev.c_str(), file) == nullptr) {
            PTPMGMT_ERROR("Wrong device '%s'", dev.c_str());
            return false;
        }
    } else if(realpath(device.c_str(), file) == nullptr) {
        PTPMGMT_ERROR("device does not exist '%s'", device.c_str());
        return false;
    }
    if(!init(file, readonly))
        return false;
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
    PTPMGMT_ERROR_CLR;
    return true;
}
bool PtpClock::initUsingIndex(int ptpIndex, bool readonly)
{
    if(m_isInit) {
        PTPMGMT_ERROR("Alreay initialized");
        return false;
    }
    std::string dev = ptp_dev;
    dev += std::to_string(ptpIndex);
    if(!isCharFile(dev))
        return false;
    if(!init(dev.c_str(), readonly))
        return false;
    m_ptpIndex = ptpIndex;
    m_device = dev;
    PTPMGMT_ERROR_CLR;
    return true;
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
    if(!m_isInit) {
        PTPMGMT_ERROR("not initialized yet");
        return false;
    }
    ptp_clock_caps cps = {0};
    if(ioctl(m_fd, PTP_CLOCK_GETCAPS, &cps) == -1) {
        PTPMGMT_ERROR_P("PTP_CLOCK_GETCAPS");
        return false;
    }
    caps.max_ppb = cps.max_adj;
    caps.num_alarm = cps.n_alarm;
    caps.num_external_channels = cps.n_ext_ts;
    caps.num_periodic_sig = cps.n_per_out;
    caps.pps = cps.pps > 0;
    caps.num_pins = cps.n_pins;
    caps.cross_timestamping = cps.cross_timestamping > 0;
    caps.adjust_phase = cps.adjust_phase > 0;
    caps.max_phase_adj = cps.max_phase_adj;
    PTPMGMT_ERROR_CLR;
    return true;
}
bool PtpClock::readPin(unsigned int index, PtpPin_t &pin) const
{
    if(!m_isInit) {
        PTPMGMT_ERROR("not initialized yet");
        return false;
    }
    ptp_pin_desc desc;
    memset(&desc, 0, sizeof desc);
    desc.index = index;
    if(ioctl(m_fd, PTP_PIN_GETFUNC, &desc) == -1) {
        PTPMGMT_ERROR_P("PTP_PIN_GETFUNC");
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
            PTPMGMT_ERROR("readPin unknown functional %d", desc.func);
            return false;
    };
    PTPMGMT_ERROR_CLR;
    return true;
}
bool PtpClock::writePin(PtpPin_t &pin) const
{
    if(!m_isInit) {
        PTPMGMT_ERROR("not initialized yet");
        return false;
    }
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
            PTPMGMT_ERROR("writePin wrong functional %d", pin.functional);
            return false;
    };
    desc.index = pin.index;
    desc.chan = pin.channel;
    if(ioctl(m_fd, PTP_PIN_SETFUNC, &desc) == -1) {
        PTPMGMT_ERROR_P("PTP_PIN_SETFUNC");
        return false;
    }
    PTPMGMT_ERROR_CLR;
    return true;
}
bool PtpClock::ExternTSEbable(unsigned int index, uint8_t flags) const
{
    if(!m_isInit) {
        PTPMGMT_ERROR("not initialized yet");
        return false;
    }
    unsigned long rid;
    if((PTP_EXTERN_TS_STRICT & flags) > 0)
        rid = PTP_EXTTS_REQUEST2;
    else
        rid = PTP_EXTTS_REQUEST;
    ptp_extts_request req = { .index = index };
    req.flags = flags | PTP_ENABLE_FEATURE;
    if(ioctl(m_fd, rid, &req) == -1) {
        PTPMGMT_ERROR_P("PTP_EXTTS_REQUEST");
        return false;
    }
    PTPMGMT_ERROR_CLR;
    return true;
}
bool PtpClock::ExternTSDisable(unsigned int index) const
{
    if(!m_isInit) {
        PTPMGMT_ERROR("not initialized yet");
        return false;
    }
    ptp_extts_request req = { .index = index };
    if(ioctl(m_fd, PTP_EXTTS_REQUEST, &req) == -1) {
        PTPMGMT_ERROR_P("PTP_EXTTS_REQUEST");
        return false;
    }
    PTPMGMT_ERROR_CLR;
    return true;
}
bool PtpClock::readEvent(PtpEvent_t &event) const
{
    if(!m_isInit) {
        PTPMGMT_ERROR("not initialized yet");
        return false;
    }
    ptp_extts_event ent;
    ssize_t ret = read(m_fd, &ent, sizeof ent);
    if(ret < 0) {
        PTPMGMT_ERROR_P("read");
        return false;
    }
    if(ret != sizeof ent) {
        PTPMGMT_ERROR("read wrong size %zd", ret);
        return false;
    }
    event = { ent.index, toTs(ent.t) };
    PTPMGMT_ERROR_CLR;
    return true;
}
const size_t PTP_BUF_TIMESTAMPS = 30; // From Linux kernel ptp_private.h
bool PtpClock::readEvents(std::vector<PtpEvent_t> &events, size_t max) const
{
    if(!m_isInit) {
        PTPMGMT_ERROR("not initialized yet");
        return -1;
    }
    if(max == 0)
        max = PTP_BUF_TIMESTAMPS;
    else
        max = std::min(max, PTP_BUF_TIMESTAMPS);
    ptp_extts_event ents[max];
    ssize_t cnt = read(m_fd, ents, sizeof ents);
    if(cnt < 0) {
        PTPMGMT_ERROR_P("read");
        return false;
    } else if(cnt == 0) {
        PTPMGMT_ERROR_CLR;
        return true;
    }
    div_t d = div(cnt, sizeof(ptp_extts_event));
    if(d.rem != 0) {
        PTPMGMT_ERROR("Wrong size %zd, not divisible", cnt);
        return false;
    }
    ptp_extts_event *ent = ents;
    for(int i = 0; i < d.quot; i++, ent++)
        events.push_back({ent->index, toTs(ent->t)});
    PTPMGMT_ERROR_CLR;
    return true;
}
bool PtpClock::setPinPeriod(unsigned int index, PtpPinPeriodDef_t times,
    uint8_t flags) const
{
    if(!m_isInit) {
        PTPMGMT_ERROR("not initialized yet");
        return false;
    }
    unsigned long rid;
    ptp_perout_request req;
    memset(&req, 0, sizeof req);
    req.flags = flags;
    if(flags == 0)
        rid = PTP_PEROUT_REQUEST;
    else
        rid = PTP_PEROUT_REQUEST2;
    if((PTP_PERIOD_WIDTH & flags) > 0)
        fromTs(req.on, times.width);
    if((PTP_PERIOD_PHASE & flags) > 0)
        fromTs(req.phase, times.phase);
    else {
        timespec ts;
        if(clock_gettime(m_clkId, &ts)) {
            PTPMGMT_ERROR_P("clock_gettime");
            return false;
        }
        req.start.sec = ts.tv_sec + 2;
        req.start.nsec = 0;
    }
    req.index = index;
    fromTs(req.period, times.period);
    if(ioctl(m_fd, rid, &req) == -1) {
        PTPMGMT_ERROR_P("PTP_PEROUT_REQUEST");
        return false;
    }
    PTPMGMT_ERROR_CLR;
    return true;
}
bool PtpClock::setPtpPpsEvent(bool enable) const
{
    if(!m_isInit) {
        PTPMGMT_ERROR("not initialized yet");
        return false;
    }
    int req = enable ? 1 : 0;
    if(ioctl(m_fd, PTP_ENABLE_PPS, &req) == -1) {
        PTPMGMT_ERROR_P("PTP_ENABLE_PPS");
        return false;
    }
    PTPMGMT_ERROR_CLR;
    return true;
}
bool PtpClock::samplePtpSys(size_t count,
    std::vector<PtpSample_t> &samples) const
{
    if(!m_isInit) {
        PTPMGMT_ERROR("not initialized yet");
        return false;
    }
    if(count == 0 || count > PTP_MAX_SAMPLES) {
        PTPMGMT_ERROR("Wrong count");
        return false;
    }
    ptp_sys_offset req = {0};
    req.n_samples = count;
    if(ioctl(m_fd, PTP_SYS_OFFSET, &req) == -1) {
        PTPMGMT_ERROR_P("PTP_SYS_OFFSET");
        return false;
    }
    ptp_clock_time *pct = req.ts;
    for(unsigned i = 0; i < req.n_samples; i++)
        samples.push_back({
        toTs(*pct++), // System clock
        toTs(*pct++)}); // PHP clock
    PTPMGMT_ERROR_CLR;
    return true;
}
bool PtpClock::extSamplePtpSys(size_t count,
    std::vector<PtpSampleExt_t> &samples) const
{
    if(!m_isInit) {
        PTPMGMT_ERROR("not initialized yet");
        return false;
    }
    if(count == 0 || count > PTP_MAX_SAMPLES) {
        PTPMGMT_ERROR("Wrong count");
        return false;
    }
    ptp_sys_offset_extended req = {0};
    req.n_samples = count;
    if(ioctl(m_fd, PTP_SYS_OFFSET_EXTENDED, &req) == -1) {
        PTPMGMT_ERROR_P("PTP_SYS_OFFSET_EXTENDED");
        return false;
    }
    for(unsigned i = 0; i < req.n_samples; i++) {
        ptp_clock_time *pct = req.ts[i];
        samples.push_back({
            toTs(*pct++), // System clock before
            toTs(*pct++), // PHP clock
            toTs(*pct++)}); // System clock after
    }
    PTPMGMT_ERROR_CLR;
    return true;
}
bool PtpClock::preciseSamplePtpSys(PtpSamplePrecise_t &sample) const
{
    if(!m_isInit) {
        PTPMGMT_ERROR("not initialized yet");
        return false;
    }
    ptp_sys_offset_precise req;
    memset(&req, 0, sizeof req);
    if(ioctl(m_fd, PTP_SYS_OFFSET_PRECISE, &req) == -1) {
        PTPMGMT_ERROR_P("PTP_SYS_OFFSET_PRECISE");
        return false;
    }
    sample.phcClk = toTs(req.device);
    sample.sysClk = toTs(req.sys_realtime);
    sample.monoClk = toTs(req.sys_monoraw);
    PTPMGMT_ERROR_CLR;
    return true;
}

__PTPMGMT_NAMESPACE_END
