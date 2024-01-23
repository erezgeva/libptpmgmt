/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief C interface to network and PHC classes
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2023 Erez Geva
 *
 */

#ifndef __PTPMGMT_C_IF_H
#define __PTPMGMT_C_IF_H

#include <time.h>
#include <linux/ptp_clock.h>

/** pointer to ptpmgmt network interface structure */
typedef struct ptpmgmt_ifInfo_t *ptpmgmt_ifInfo;

/** pointer to constant ptpmgmt network interface structure */
typedef const struct ptpmgmt_ifInfo_t *const_ptpmgmt_ifInfo;

/**
 * the ptpmgmt network interface structure hold the network interface object
 *  and call backs to call c++ methods
 */
struct ptpmgmt_ifInfo_t {
    /**< @cond internal */
    void *_this; /**< pointer to actual C++ network interface object */
    /**< @endcond */
    /**
     * Free a network interface object
     * @param[in] i network interface object
     */
    void (*free)(ptpmgmt_ifInfo i);
    /**
     * Find network interface information based on its name
     * @param[in, out] i network interface object
     * @param[in] ifName network interface name
     * @return true if network interface exists
     */
    bool (*initUsingName)(ptpmgmt_ifInfo i, const char *ifName);
    /**
     * Find network interface information based on its index
     * @param[in, out] i network interface object
     * @param[in] ifIndex network interface index
     * @return true if network interface exists
     */
    bool (*initUsingIndex)(ptpmgmt_ifInfo i, int ifIndex);
    /**
     * Is object initialized
     * @param[in] i network interface object
     * @return true if network interface was successfully initialized
     */
    bool (*isInit)(const_ptpmgmt_ifInfo i);
    /**
     * Get interface index
     * @param[in] i network interface object
     * @return interface index or NO_SUCH_IF if object is not initialized
     */
    int (*ifIndex)(const_ptpmgmt_ifInfo i);
    /**
     * Get interface name
     * @param[in] i network interface object
     * @return interface name or empty string if object is not initialized
     */
    const char *(*ifName)(const_ptpmgmt_ifInfo i);
    /**
     * Get interface MAC address
     * @param[in] i network interface object
     * @return binary from address or empty string if object is not initialized
     */
    const uint8_t *(*mac)(const_ptpmgmt_ifInfo i);
    /**
     * Get interface MAC address length
     * @param[in] i network interface object
     * @return binary from address length or 0 if object is not initialized
     */
    size_t (*mac_size)(const_ptpmgmt_ifInfo i);
    /**
     * Get interface PTP index
     * @param[in] i network interface object
     * @return ptp index or -1 if object is not initialized or
     *  interface does not support PTP
     * @note An interface may have multiple PHCs,
     *       this function return the index report by the get_ts_info
     */
    int (*ptpIndex)(const_ptpmgmt_ifInfo i);
};

/**
 * Allocate a new network interface object
 * @return a new network interface object or null on error
 */
ptpmgmt_ifInfo ptpmgmt_ifInfo_alloc();

/** pointer to ptpmgmt clocl structure */
typedef struct ptpmgmt_clock_t *ptpmgmt_clock;

/** pointer to constant ptpmgmt clock structure */
typedef const struct ptpmgmt_clock_t *const_ptpmgmt_clock;

/**
 * the ptpmgmt clock structure hold the clock object
 *  and call backs to call c++ methods
 */
struct ptpmgmt_clock_t {
    /**< @cond internal */
    void *_this; /**< pointer to actual C++ clock object */
    /**< @endcond */

    /**
     * Free a clock object
     * @param[in] clk pointer to clock structure
     */
    void (*free)(ptpmgmt_clock clk);

    /**
     * Get clock time
     * @param[in] clk pointer to clock structure
     * @return clock time or zero on error
     */
    struct timespec(*getTime)(const_ptpmgmt_clock clk);
    /**
     * Set clock time
     * @param[in] clk pointer to clock structure
     * @param[out] ts new clock time
     * @return true for success
     */
    bool (*setTime)(const_ptpmgmt_clock clk, const struct timespec *ts);
    /**
     * Offset clock time
     * @param[in] clk pointer to clock structure
     * @param[in] offset in nanoseconeds
     * @return true for success
     */
    bool (*offsetClock)(const_ptpmgmt_clock clk, int64_t offset);
    /**
     * Get clock adjustment frequancy
     * @param[in] clk pointer to clock structure
     * @return freq frequancy in ppb
     */
    double (*getFreq)(const_ptpmgmt_clock clk);
    /**
     * Set clock adjustment frequancy
     * @param[in] clk pointer to clock structure
     * @param[in] freq frequancy in ppb
     * @return true for success
     */
    bool (*setFreq)(const_ptpmgmt_clock clk, double freq);
    /**
     * Set clock phase offset
     * @param[in] clk pointer to clock structure
     * @param[in] offset in nanoseconeds
     * @return true for success
     */
    bool (*setPhase)(const_ptpmgmt_clock clk, int64_t offset);
    /**
     * Check file is a char file
     * @param[in] file name to check
     * @return true if file is char device
     * @note function will follow a symbolic link
     */
    bool (*isCharFile)(const char *file);
    /**
     * Initialize a POSIX clock based on its device name
     * @param[in, out] clk pointer to clock structure
     * @param[in] device name
     * @param[in] readonly open clock to read only
     * @return true if device is a POSIX clock
     */
    bool (*initUsingDevice)(ptpmgmt_clock clk, const char *device, bool readonly);
    /**
     * Initialize a PTP clock based on its index
     * @param[in, out] clk pointer to clock structure
     * @param[in] ptpIndex clock PTP index
     * @param[in] readonly open clock to read only
     * @return true if network interface exists
     */
    bool (*initUsingIndex)(ptpmgmt_clock clk, int ptpIndex, bool readonly);
    /**
     * Is object initialized
     * @param[in] clk pointer to clock structure
     * @return true if PTP clock exist and object was successfully initialized
     */
    bool (*isInit)(const_ptpmgmt_clock clk);
    /**
     * Get dynamic clock id
     * @param[in] clk pointer to clock structure
     * @return dynamic clock id or CLOCK_INVALID if not initialized
     */
    clockid_t (*clkId)(const_ptpmgmt_clock clk);
    /**
     * Get file description
     * @param[in] clk pointer to clock structure
     * @return file description
     */
    int (*getFd)(const_ptpmgmt_clock clk);
    /**
     * Get file description
     * @param[in] clk pointer to clock structure
     * @return file description
     */
    int (*fileno)(const_ptpmgmt_clock clk);
    /**
     * Get PTP index
     * @param[in] clk pointer to clock structure
     * @return index or NO_SUCH_PTP if not initialized or
     *         clock do not have PTP index
     */
    int (*ptpIndex)(const_ptpmgmt_clock clk);
    /**
     * Get POSIX device name
     * @param[in] clk pointer to clock structure
     * @return device name or empty string if not initialized
     */
    const char *(*device)(const_ptpmgmt_clock clk);
    /**
     * Set PHC time using the system clock
     * @param[in] clk pointer to clock structure
     * @return true for success
     */
    bool (*setTimeFromSys)(const_ptpmgmt_clock clk);
    /**
     * Set system clock using the PHC as source time
     * @param[in] clk pointer to clock structure
     * @return true for success
     */
    bool (*setTimeToSys)(const_ptpmgmt_clock clk);
    /**
     * Get PTP clock capabilities
     * @param[in] clk pointer to clock structure
     * @param[out] caps capabilities
     * @return true for success
     */
    bool (*fetchCaps)(const_ptpmgmt_clock clk, struct ptp_clock_caps *caps);
    /**
     * Read PHC clock pin current functional state
     * @param[in] clk pointer to clock structure
     * @param[in] index pin index to read
     * @param[out] pin hardware functional state
     * @return true for success
     * @note Pin index should be in the range (0, ptp_clock_caps.num_pins]
     */
    bool (*readPin)(const_ptpmgmt_clock clk, unsigned int index,
        struct ptp_pin_desc *pin);
    /**
     * Set PHC clock pin functional state
     * @param[in] clk pointer to clock structure
     * @param[in] pin hardware functional state
     * @return true for success
     * @note Pin index comes from ptp_pin_desc!
     * @note Pin index should be in the range (0, ptp_clock_caps.num_pins]
     */
    bool (*writePin)(const_ptpmgmt_clock clk, struct ptp_pin_desc *pin);
    /**
     * Enable PHC pin external events
     * @param[in] clk pointer to clock structure
     * @param[in] index pin index to enable
     * @param[in] flags using the PTP_EXTERN_TS_xxx flags
     * @return true for success
     * @note Pin index should be in the range (0, ptp_clock_caps.num_pins]
     * @note old kernel do not support PTP_EXTERN_TS_STRICT
     */
    bool (*ExternTSEbable)(const_ptpmgmt_clock clk, unsigned int index,
        uint8_t flags);
    /**
     * Disable PHC pin external events
     * @param[in] clk pointer to clock structure
     * @param[in] index pin index to enable
     * @return true for success
     * @note Pin index should be in the range (0, ptp_clock_caps.num_pins]
     */
    bool (*ExternTSDisable)(const_ptpmgmt_clock clk, unsigned int index);
    /**
     * Read single external event
     * @param[in] clk pointer to clock structure
     * @param[out] event retrieved event
     * @return true for success
     */
    bool (*readEvent)(const_ptpmgmt_clock clk, struct ptp_extts_event *event);
    /**
     * Read external events
     * @param[in] clk pointer to clock structure
     * @param[out] events pointer to array of retrieved events
     * @param[in, out] size input: array size, output: actual number of events
     * @return true for success
     * @note the maximum is trunced to 30 events
     */
    bool (*readEvents)(const_ptpmgmt_clock clk, struct ptp_extts_event *events,
        size_t *size);
    /**
     * Set PHC pin period signal
     * @param[in] clk pointer to clock structure
     * @param[in] index pin index
     * @param[in] times provides the times and flags used for the period
     * @return true for success
     * @note Pin index should be in the range (0, ptp_clock_caps.num_pins]
     * @note old kernel do not support flags
     */
    bool (*setPinPeriod)(const_ptpmgmt_clock clk, unsigned int index,
        struct ptp_perout_request *times);
    /**
     * Enable or disable Linux pulse per second event
     * @param[in] clk pointer to clock structure
     * @param[in] enable flag
     * @return true for success
     * @note Linux will create a pps device /dev/ppsN
     *       User can use the 'pps-tools' to use the device.
     */
    bool (*setPtpPpsEvent)(const_ptpmgmt_clock clk, bool enable);
    /**
     * Sample the PHC and system clock
     * So caller can compare offset and frequancy
     * @param[in] clk pointer to clock structure
     * @param[in] count number of sample to measure
     * @param[out] samples taken by kernel
     * @return true for success
     */
    bool (*samplePtpSys)(const_ptpmgmt_clock clk, size_t count,
        struct ptp_sys_offset *samples);
    /**
     * Extended sample the PHC and system clock
     * So caller can compare offset and frequancy
     * @param[in] clk pointer to clock structure
     * @param[in] count number of sample to measure
     * @param[out] samples taken by kernel
     * @return true for success
     * @note old kernel do not support
     */
    bool (*extSamplePtpSys)(const_ptpmgmt_clock clk, size_t count,
        struct ptp_sys_offset_extended *samples);
    /**
     * Precise sample the PHC using PCI cross time stamp
     * @param[in] clk pointer to clock structure
     * @param[out] sample taken by kernel
     * @return true for success
     * @note old kernel do not support
     */
    bool (*preciseSamplePtpSys)(const_ptpmgmt_clock clk,
        struct ptp_sys_offset_precise *sample);
};

/**
 * Check file is a char file
 * @param[in] file name to check
 * @return true if file is char device
 * @note function will follow a symbolic link
 */
bool ptpmgmt_clock_isCharFile(const char *file);

/**
 * Allocate a new ptpmgmt clock structure
 * @return a pointer to a new ptpmgmt clock structure or null on error
 */
ptpmgmt_clock ptpmgmt_clock_alloc();

/**
 * Allocate a new ptpmgmt clock structure for system clock
 * @return a pointer to a new ptpmgmt clock structure or null on error
 */
ptpmgmt_clock ptpmgmt_clock_alloc_sys();

#endif /* __PTPMGMT_C_IF_H */
