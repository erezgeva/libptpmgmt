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
/**
 * Enable PHC pin events with rising edge flag
 * Equal to Linux kernel flag PTP_RISING_EDGE
 */
const uint8_t PTP_EXTERN_TS_RISING_EDGE = 1 << 1;
/**
 * Enable PHC pin events with falling edge flag
 * Equal to Linux kernel flag PTP_FALLING_EDGE
 */
const uint8_t PTP_EXTERN_TS_FALLING_EDGE = 1 << 2;
/**
 * Enable PHC pin events with strict flag
 * Equal to Linux kernel flag PTP_STRICT_FLAGS
 */
const uint8_t PTP_EXTERN_TS_STRICT = 1 << 3;
/**
 * Use PHC pin period signal once flag
 * Equal to Linux kernel flag PTP_PEROUT_ONE_SHOT
 */
const uint8_t PTP_PERIOD_ONE_SHOT = 1 << 0;
/**
 * Use PHC pin period pulse width flag
 * Equal to Linux kernel flag PTP_PEROUT_DUTY_CYCLE
 */
const uint8_t PTP_PERIOD_WIDTH = 1 << 1;
/**
 * Use PHC pin period phase flag
 * Equal to Linux kernel flag PTP_PEROUT_PHASE
 */
const uint8_t PTP_PERIOD_PHASE = 1 << 2;

/**
 * PHC pin event
 */
struct PtpEvent_t {
    unsigned int index; /**< pin index for the event */
    Timestamp_t time; /**< Event time */
};
/**
 * Sample of system clock and PHC
 */
struct PtpSample_t {
    Timestamp_t sysClk; /**< System clock sample */
    Timestamp_t phcClk; /**< PHC clock sample */
};
/**
 * Extended sample of system clock and PHC
 */
struct PtpSampleExt_t {
    Timestamp_t before; /**< System clock sample before */
    Timestamp_t phcClk; /**< PHC clock sample */
    Timestamp_t after; /**< System clock sample after */
};
/**
 * Precise sample of system clock and PHC
 */
struct PreciseSampleExt_t {
    Timestamp_t phcClk; /**< PHC clock sample */
    Timestamp_t sysClk; /**< System clock sample */
    Timestamp_t monoClk; /**< System clock monotonic raw sample */
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
    Timestamp_t getTime() const;
    /**
     * Set clock time in nanoseconds
     * @param[out] ts new clock time
     * @return true for success
     */
    bool setTime(const Timestamp_t &ts) const;
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
     * Check file is a char file
     * @param[in] file name to check
     * @return true if file is char device
     * @note function will follow a symbolic link
     */
    static bool isCharFile(const std::string &file);
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
     * @param[in] flags using the PTP_EXTERN_TS_xxx flags
     * @return true for success
     * @note Pin index should be in the range (0, PtpCaps_t.num_pins]
     * @note old kernel do not support PTP_EXTERN_TS_STRICT
     */
    bool ExternTSEbable(unsigned int index, uint8_t flags) const;
    /**
     * Disable PHC pin external events
     * @param[in] index pin index to enable
     * @return true for success
     * @note Pin index should be in the range (0, PtpCaps_t.num_pins]
     */
    bool ExternTSDisable(unsigned int index) const;
    /**
     * Read single external event
     * @param[out] event retrieved event
     * @return true for success
     */
    bool readEvent(PtpEvent_t &event) const;
    /**
     * Read external events
     * @param[out] events retrieved events
     * @param[in] max maximum number of events to read
     * @return the number of events read, or -1 on error
     * @note the maximum is trunced to 30 events
     */
    int readEvents(std::vector<PtpEvent_t> &events, size_t max = 0) const;
    /**
     * Set PHC pin period signal
     * @param[in] index pin index
     * @param[in] period cycle time
     * @param[in] flags using the PTP_PERIOD_xxx flags
     * @param[in] width time, used when PTP_PERIOD_WIDTH flag is used.
     * @param[in] phase time, used when PTP_PERIOD_PHASE flag is used.
     * @return true for success
     * @note Pin index should be in the range (0, PtpCaps_t.num_pins]
     * @note old kernel do not support flags
     */
    bool setPinPeriod(unsigned int index, const Timestamp_t &period,
        uint8_t flags = 0, const Timestamp_t &width = 0,
        const Timestamp_t &phase = 0) const;
    /**
     * Enable or disable Linux PTP_CLOCK_PPS event
     * @param[in] enable flag
     * @return true for success
     */
    bool setPtpPpsEvent(bool enable) const;
    /**
     * Sample the PHC and system clock
     * So caller can compare offset and frequancy
     * @param[in] count number of sample to measure
     * @param[out] samples taken by kernel
     * @return true for success
     */
    bool samplePtpSys(size_t count, std::vector<PtpSample_t> &samples) const;
    /**
     * Extended sample the PHC and system clock
     * So caller can compare offset and frequancy
     * @param[in] count number of sample to measure
     * @param[out] samples taken by kernel
     * @return true for success
     * @note old kernel do not support
     */
    bool extSamplePtpSys(size_t count, std::vector<PtpSampleExt_t> &samples) const;

    /**
     * Precise sample the PHC using PCI cross time stamp
     * @param[out] sample taken by kernel
     * @return true for success
     * @note old kernel do not support
     */
    bool preciseSamplePtpSys(PreciseSampleExt_t &sample) const;
};

#ifndef SWIG
}; /* namespace ptpmgmt */
#endif

#endif /* __PTPMGMT_IF_H */
