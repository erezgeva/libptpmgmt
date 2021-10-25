/* SPDX-License-Identifier: LGPL-3.0-or-later */

/** @file
 * @brief Read network interface information and retrieve the PTP information
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 * @copyright GNU Lesser General Public License 3.0 or later
 *
 * @details
 *  A class to fetch network interface information and
 *  a class to fetch a dynamic clock ID for a PTP clock
 */

#ifndef __PMC_IF_H
#define __PMC_IF_H

#include <string>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <net/if.h>
#include <linux/ethtool.h>
#include "bin.h"

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
    IfInfo() : m_isInit(false), m_ifIndex(-1), m_ptpIndex(-1) {}
    /**
     * Find network interface information based on its name
     * @param[in] ifName network interface name
     * @return true if network interface exists
     */
    bool initUsingName(const std::string ifName);
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
    bool isInit() { return m_isInit; }
    /**
     * Get interface index
     * @return interface index or -1 if object is not initialized
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
     * @return ptp index or -1 if object is not initialized or
     *  interface does not support PTP
     */
    int ptpIndex() const { return m_ptpIndex; }
};

/**
 * @brief PTP clock id generator
 * @details
 *  Create a dynamic clock id for a PTP clock, that the application
 *  can use to fetch the PTP clock time
 * @note
 *  The dynamic clock id exists while the object exists.
 *  Do @b NOT delete the object while using the dynamic clock id.
 * @note
 *  As a network interface might have more than 1 PHC
 *  It is better to use a separate the class.
 */
class PtpClock
{
  private:
    int m_fd;
    int m_ptpIndex;
    bool m_isInit;
    clockid_t m_clkId;
    std::string m_ptpDevice;

  public:
    /**
     * Allocate a PtpClock object and initialize it
     * @param[in] ptpIndex PTP index
     */
    PtpClock(int ptpIndex);
    ~PtpClock();
    /**
     * Is object initialized
     * @return true if PTP clock exist and object was successfully initialized
     */
    bool isInit() const { return m_isInit; }
    /**
     * Get dynamic clock id
     * @return dynamic clock id or -1 if not initialized
     */
    clockid_t clkId() const { return m_clkId; }
    /**
     * Get PTP index
     * @return index or -1 if not initialized
     */
    int ptpIndex() const { return m_ptpIndex; }
    /**
     * Get PTP device name
     * @return device name or empty string if not initialized
     */
    const std::string &ptpDevice() const { return m_ptpDevice; }
    /**
     * Get PTP device name
     * @return device name or empty string if not initialized
     */
    const char *ptpDevice_c() const { return m_ptpDevice.c_str(); }
};

#endif /*__PMC_IF_H*/
