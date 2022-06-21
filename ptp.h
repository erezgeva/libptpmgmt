/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/** @file
 * @brief Read network interface information and retrieve the PTP information
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 * @details
 *  A class to fetch network interface information and
 *  a class to fetch a dynamic clock ID for a PTP clock
 */

#ifndef __PTPMGMT_IF_H
#define __PTPMGMT_IF_H

#include <string>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <vector>
#include <net/if.h>
#include <linux/ethtool.h>
#include "bin.h"
#include "types.h"

#ifndef SWIG
namespace ptpmgmt
{
#endif

/* The macro might be used, we prefer C++ const */
#ifndef CLOCK_INVALID
/** ivalid or not exist POSIX clock */
const clockid_t CLOCK_INVALID = -1;
#endif
/** No such network interface */
const int NO_SUCH_IF = -1;
/** No such PTP clock */
const int NO_SUCH_PTP = -1;

/**
 * @brief Network interface information
 */
class IfInfo
{
  private:
    bool m_isInit;
    int m_ifIndex;
    int m_ptpIndex;
    std::string m_ifName;
    Binary m_mac;

    bool initPtp(int fd, struct ifreq &m_ifr);

  public:
    IfInfo() : m_isInit(false), m_ifIndex(NO_SUCH_IF), m_ptpIndex(NO_SUCH_PTP) {}
    /**
     * Find network interface information based on its name
     * @param[in] ifName network interface name
     * @return true if network interface exists
     */
    bool initUsingName(const std::string &ifName);
    /**
     * Find network interface information based on its index
     * @param[in] ifIndex network interface index
     * @return true if network interface exists
     */
    bool initUsingIndex(int ifIndex);
    /**
     * Is object initialized
     * @return true if network interface was successfully initialized
     */
    bool isInit() const { return m_isInit; }
    /**
     * Get interface index
     * @return interface index or NO_SUCH_IF if object is not initialized
     */
    int ifIndex() const { return m_ifIndex; }
    /**
     * Get interface name
     * @return interface name or empty string if object is not initialized
     */
    const std::string &ifName() const { return m_ifName; }
    /**
     * Get interface name
     * @return interface name or empty string if object is not initialized
     */
    const char *ifName_c() const { return m_ifName.c_str(); }
    /**
     * Get interface MAC address
     * @return binary from address or empty string if object is not initialized
     */
    const Binary &mac() const { return m_mac; }
    /**
     * Get interface MAC address
     * @return binary from address or empty string if object is not initialized
     */
    const uint8_t *mac_c() const { return m_mac.get(); }
    /**
     * Get interface MAC address length
     * @return binary from address length or 0 if object is not initialized
     */
    size_t mac_size() const { return m_mac.length(); }
    /**
     * Get interface PTP index
     * @return ptp index or NO_SUCH_PTP if object is not initialized or
     *  interface does not support PTP
     * @note An interface may have multiple PHCs,
     *       this function return the index report by the get_ts_info
     */
    int ptpIndex() const { return m_ptpIndex; }
};
/**
 * Store Clock time
 */
struct ClockTime {
    int64_t seconds; /**< Seconds */
    uint32_t nanoseconds; /**< Nano seconds */
    ClockTime() : seconds(0), nanoseconds(0) {}
    /**
     * Copy constructor
     * @param[in] ct another ClockTime
     * @note Ensure compiler create a copy contructor
     */
    ClockTime(const ClockTime &ct) = default;
    /**
     * Constructor
     * @param[in] secs Seconds
     * @param[in] nsecs Nanoseconds
     */
    ClockTime(int64_t secs, uint32_t nsecs) : seconds(secs), nanoseconds(nsecs) {}
    #ifndef SWIG
    /**
     * Convert from timespec
     * @param[in] ts timespec structure
     * @note scripts should not use the timespec structure
     */
    ClockTime(const timespec &ts) {
        seconds = ts.tv_sec;
        nanoseconds = ts.tv_nsec;
    }
    /**
     * Convert to timespec
     * @note scripts should not use the timespec structure
     */
    operator timespec() const { timespec ts; toTimespec(ts); return ts; }
    /**
     * Convert to timespec
     * @param[in, out] ts timespec structure
     * @note scripts should not use the timespec structure
     */
    void toTimespec(timespec &ts) const {
        ts.tv_sec = seconds;
        ts.tv_nsec = nanoseconds;
    }
    /**
     * Convert from timeval
     * @param[in] tv timeval structure
     * @note scripts should not use the timeval structure
     */
    ClockTime(const timeval &tv);
    /**
     * Convert to timespec
     * @note scripts should not use the timeval structure
     */
    operator timeval() const { timeval tv; toTimeval(tv); return tv; }
    /**
     * Convert to timeval
     * @param[in, out] tv timeval structure
     * @note scripts should not use the timeval structure
     */
    void toTimeval(timeval &tv) const;
    #endif /*SWIG*/
    /**
     * Convert from Timestamp
     * @param[in] ts Timestamp structure
     */
    ClockTime(const Timestamp_t &ts) {
        seconds = ts.secondsField;
        nanoseconds = ts.nanosecondsField;
    }
    #ifndef SWIG
    /**
     * Convert to Timestamp
     * @note scripts should not use the Timestamp structure
     * @note scripts can use the toTimestamp() method
     */
    operator Timestamp_t() const { Timestamp_t ts; toTimestamp(ts); return ts; }
    #endif
    /**
     * Convert to Timestamp
     * @param[in, out] ts Timestamp structure
     */
    void toTimestamp(Timestamp_t &ts) const {
        ts.secondsField = seconds;
        ts.nanosecondsField = nanoseconds;
    }
    /**
     * Convert from seconds with fractions
     * @param[in] seconds with fractions
     */
    ClockTime(long double seconds) {formFloat(seconds);}
    /**
     * Convert from seconds with fractions
     * @param[in] seconds with fractions
     */
    void formFloat(long double seconds);
    #ifndef SWIG
    /**
     * Convert to seconds with fractions
     * @note scripts can use the toFloat() method
     */
    operator long double() const { return toFloat(); }
    #endif
    /**
     * Convert to seconds with fractions
     * @return seconds with fractions
     */
    long double toFloat() const;
    /**
     * Convert from nanoseconds
     * @param[in] nanoseconds
     */
    void fromNanoseconds(int64_t nanoseconds);
    /**
     * Convert to nanoseconds
     * @return nanoseconds
     */
    int64_t toNanoseconds() const;
    #ifndef SWIG
    /**
     * Convert to string of seconds with fractions
     * @note scripts can use the string() method
     */
    operator std::string() const { return string(); }
    #endif
    /**
     * Convert to string of seconds with fractions
     * @return string
     */
    std::string string() const;
    /**
     * Compare to another clock time
     * @param[in] ts another clock time
     * @return true if the same time
     */
    bool operator==(const ClockTime &ts) const { return eq(ts); }
    /**
     * Compare to another clock time
     * @param[in] ts another clock time
     * @return true if the same time
     */
    bool eq(const ClockTime &ts) const {
        return seconds == ts.seconds && nanoseconds == ts.nanoseconds;
    }
    /**
     * Compare to seconds with fractions
     * @param[in] seconds
     * @return true if the same time
     */
    bool operator==(long double seconds) const { return eq(seconds); }
    /**
     * Compare to seconds with fractions
     * @param[in] seconds
     * @return true if the same time
     */
    bool eq(long double seconds) const;
    /**
     * Compare to another clock time
     * @param[in] ts another clock time
     * @return true if smaller then other time
     * @note when compare to a number,
     *       the number will be converted as seconds with fractions
     */
    bool operator<(const ClockTime &ts) const { return less(ts); }
    /**
     * Compare to another clock time
     * @param[in] ts another clock time
     * @return true if smaller then other time
     */
    bool less(const ClockTime &ts) const {
        return seconds < ts.seconds ||
            (seconds == ts.seconds && nanoseconds < ts.nanoseconds);
    }
    /**
     * Compare to seconds with fractions
     * @param[in] seconds
     * @return true if smaller
     */
    bool operator<(long double seconds) const { return less(seconds); }
    /**
     * Compare to seconds with fractions
     * @param[in] seconds
     * @return true if smaller
     */
    bool less(long double seconds) const;
    /**
     * Add another clock time
     * @param[in] ts another clock time
     * @return reference to itself
     */
    ClockTime &operator+(const ClockTime &ts) { return add(ts); }
    #ifndef SWIG
    /**
     * Add another clock time
     * @param[in] ts another clock time
     * @return reference to itself
     */
    ClockTime &operator+=(const ClockTime &ts) { return add(ts); }
    #endif /*SWIG*/
    /**
     * Add another clock time
     * @param[in] ts another clock time
     * @return reference to itself
     */
    ClockTime &add(const ClockTime &ts);
    /**
     * Add a seconds with fractions
     * @param[in] seconds
     * @return reference to itself
     */
    ClockTime &operator+(long double seconds) { return add(seconds); }
    #ifndef SWIG
    /**
     * Add a seconds with fractions
     * @param[in] seconds
     * @return reference to itself
     */
    ClockTime &operator+=(long double seconds) { return add(seconds); }
    #endif /*SWIG*/
    /**
     * Add a seconds with fractions
     * @param[in] seconds
     * @return reference to itself
     */
    ClockTime &add(long double seconds);
    /**
     * Subtract another clock time
     * @param[in] ts another clock time
     * @return reference to itself
     */
    ClockTime &operator-(const ClockTime &ts) { return subt(ts); }
    #ifndef SWIG
    /**
     * Subtract another clock time
     * @param[in] ts another clock time
     * @return reference to itself
     */
    ClockTime &operator-=(const ClockTime &ts) { return subt(ts); }
    #endif /*SWIG*/
    /**
     * Subtract another clock time
     * @param[in] ts another clock time
     * @return reference to itself
     */
    ClockTime &subt(const ClockTime &ts);
    /**
     * Subtract seconds with fractions
     * @param[in] seconds
     * @return reference to itself
     */
    ClockTime &operator-(long double seconds) { return add(-seconds); }
    #ifndef SWIG
    /**
     * Subtract seconds with fractions
     * @param[in] seconds
     * @return reference to itself
     */
    ClockTime &operator-=(long double seconds) { return add(-seconds); }
    #endif /*SWIG*/
    /**
     * Subtract seconds with fractions
     * @param[in] seconds
     * @return reference to itself
     */
    ClockTime &subt(long double seconds) { return add(-seconds); }
  private:
    ClockTime &normNano();
};
/**
 * Bridge to Linux kernel struct ptp_clock_caps
 */
struct PtpCaps_t {
    int max_ppb; /**< Maximum frequency adjustment in parts per billon. */
    int num_alarm; /**< Number of programmable alarms. */
    int num_external_channels; /**< Number of external time stamp channels. */
    int num_periodic_sig; /**< Number of programmable periodic signals. */
    bool pps; /**< Clock supports a PPS callback. */
    int num_pins; /**< Number of input/output pins. */
    /** Clock supports precise system-device cross timestamps. */
    bool cross_timestamping;
    bool adjust_phase; /**< Clock supports adjust phase */
};
/**
 * PHC hardware pin functional state value enumerator
 */
enum PtpPinFunc_e {
    PTP_PIN_UNUSED,       /**< pin is currently unused */
    PTP_PIN_EXTERNAL_TS,  /**< pin uses external time stamp */
    PTP_PIN_PERIODIC_OUT, /**< pin produce a periodic signal */
    PTP_PIN_PHY_SYNC,     /**< pin calibrate by physical input */
};
/**
 * PHC hardware pin functional state
 */
struct PtpPin_t {
    unsigned int index; /**< pin index */
    /**
     * pin description
     * @note read only, set by kernel driver
     */
    std::string description;
    PtpPinFunc_e functional; /**< pin current functional state */
    unsigned int channel; /**< pin channel */
};
/** Enable PHC pin events with rising edge flag */
const uint8_t PTP_EXTERN_TS_RISING_EDGE = 1 << 1;
/** Enable PHC pin events with falling edge flag */
const uint8_t PTP_EXTERN_TS_FALLING_EDGE = 1 << 2;
/** Enable PHC pin events with strict flag */
const uint8_t PTP_EXTERN_TS_STRICT = 1 << 3;
/**
 * PHC pin event
 */
struct PtpEvent_t {
    unsigned int index; /**< pin index for the event */
    ClockTime time; /**< Event time */
};
/**
 * Base class for clock classes
 */
class BaseClock
{
  protected:
    /** @cond internal
     * For internal use
     */
    clockid_t m_clkId;
    bool m_isInit;
    BaseClock(clockid_t clkId, bool init) : m_clkId(clkId), m_isInit(init) {}
    BaseClock() : m_clkId(CLOCK_INVALID), m_isInit(false) {}
    /**< @endcond */
  public:
    /**
     * Get clock time in nanoseconds
     * @return clock time or zero on error
     */
    ClockTime getTime() const;
    /**
     * Set clock time in nanoseconds
     * @param[out] ts new clock time
     * @return true for success
     */
    bool setTime(const ClockTime &ts) const;
};
/**
 * System clock
 */
class SysClock : public BaseClock
{
  public:
    SysClock();
};
/**
 * @brief POSIX dynamic clock id generator
 * @details
 *  Create a dynamic clock id for a POSIX clock, that the application
 *  can use to fetch the PTP clock time and perform actions on PHC hardware.
 * @note
 *  The dynamic clock id exists while the object exists.
 *  Do @b NOT delete the object while using the dynamic clock id.
 * @note A network interface may have multiple PHCs
 */
class PtpClock : public BaseClock
{
  private:
    int m_fd;
    int m_ptpIndex;
    std::string m_device;
    bool init(const char *device, bool readonly);

  public:
    PtpClock() : m_fd(-1), m_ptpIndex(NO_SUCH_PTP) {}
    virtual ~PtpClock();
    /**
     * Initialize a POSIX clock based on its device name
     * @param[in] device name
     * @param[in] readonly open clock to read only
     * @return true if device is a POSIX clock
     */
    bool initUsingDevice(const std::string &device, bool readonly = false);
    /**
     * Initialize a PTP clock based on its index
     * @param[in] ptpIndex clock PTP index
     * @param[in] readonly open clock to read only
     * @return true if network interface exists
     */
    bool initUsingIndex(int ptpIndex, bool readonly = false);
    /**
     * Is object initialized
     * @return true if PTP clock exist and object was successfully initialized
     */
    bool isInit() const { return m_isInit; }
    /**
     * Get dynamic clock id
     * @return dynamic clock id or CLOCK_INVALID if not initialized
     */
    clockid_t clkId() const { return m_clkId; }
    /**
     * Get PTP index
     * @return index or NO_SUCH_PTP if not initialized or
     *         clock do not have PTP index
     */
    int ptpIndex() const { return m_ptpIndex; }
    /**
     * Get POSIX device name
     * @return device name or empty string if not initialized
     */
    const std::string &device() const { return m_device; }
    /**
     * Get POSIX device name
     * @return device name or empty string if not initialized
     */
    const char *device_c() const { return m_device.c_str(); }
    /**
     * Check file is a char file
     * @param[in] file name to check
     * @return true if file is char device
     * @note function will follow a symbolic link
     */
    static bool isCharFile(const std::string &file);
    /**
     * Get PTP clock capabilities
     * @param[out] caps capabilities
     * @return true for success
     */
    bool fetchCaps(PtpCaps_t &caps) const;
    /**
     * Read PHC clock pin current functional state
     * @param[in] index pin index to read
     * @param[out] pin hardware functional state
     * @return true for success
     * @note Pin index should be in the range (0, PtpCaps_t.num_pins]
     */
    bool readPin(unsigned int index, PtpPin_t &pin) const;
    /**
     * Set PHC clock pin functional state
     * @param[in] pin hardware functional state
     * @return true for success
     * @note Pin index comes from PtpPin_t!
     * @note Pin index should be in the range (0, PtpCaps_t.num_pins]
     */
    bool writePin(PtpPin_t &pin) const;
    /**
     * Enable PHC pin external events
     * @param[in] index pin index to enable
     * @param[in] flags using the PTP_EXTERN_TS_xxx
     * @return true for success
     * @note old kernel do not support PTP_EXTERN_TS_STRICT
     */
    bool ExternTSEbable(unsigned int index, uint8_t flags) const;
    /**
     * Disable PHC pin external events
     * @param[in] index pin index to enable
     * @return true for success
     */
    bool ExternTSDisable(unsigned int index) const;
    /**
     * Read single event
     * @param[out] event retrieved event
     * @return true for success
     */
    bool readEvent(PtpEvent_t &event) const;
    /**
     * Read events
     * @param[out] events retrieved events
     * @param[in] max maximum number of events to read
     * @return the number of events read, or -1 on error
     * @note the maximum is trunced to 30 events
     */
    int readEvents(std::vector<PtpEvent_t> &events, int max = -1) const;
};

#ifndef SWIG
}; /* namespace ptpmgmt */
#endif

#endif /* __PTPMGMT_IF_H */
