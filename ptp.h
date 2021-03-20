/* SPDX-License-Identifier: LGPL-3.0-or-later */

/** @file
 * @brief Read network interface information and retrieve the PTP information
 *
 * @author Erez Geva <ErezGeva2@gmail.com>
 * @copyright 2021 Erez Geva
 *
 * @details
 *  A class to fetch network interface information and
 *  a class to fetch a dynamic clock ID for a PTP clock
 */

#ifndef __IF_H
#define __IF_H

#include <string>
#include <stdint.h>
#include <net/if.h>
#include <linux/ethtool.h>

const size_t EUI48 = ETH_ALEN; /**< 48 bits MAC address length */
const size_t EUI64 = 8;        /**< 64 bits MAC address length */

/**
 * @brief Network interface information
 */
class ifInfo
{
  private:
    bool m_isInit;
    int m_ifIndex;
    int m_ptpIndex;
    std::string m_ifName;
    std::string m_mac;

    bool initBase(const char *ifName);
    bool initPtp(int fd, struct ifreq &m_ifr);
    static std::string str2mac(const char *str, size_t len);

  public:
    ifInfo() : m_isInit(false), m_ifIndex(-1), m_ptpIndex(-1) {}
    /**
     * Find network interface information based on its name
     * @param[in] ifName network interface name
     * @return true if network interface exist
     */
    bool init(const char *ifName);
    /**
     * Find network interface information based on its name
     * @param[in] ifName network interface name
     * @return true if network interface exist
     */
    bool init(const std::string &ifName);
    /**
     * Find network interface information based on its index
     * @param[in] ifIndex network interface index
     * @return true if network interface exist
     */
    bool init(int ifIndex);
    /**
     * Is object initialized
     * @return true if network interface was successfully initialized
     */
    bool isInit() { return m_isInit; }
    /**
     * Get interface index
     * @return interface index or -1 if object is not initialized
     */
    int ifIndex() { return m_ifIndex; }
    /**
     * Get interface name
     * @return interface name or empty string if object is not initialized
     */
    const std::string &ifName() { return m_ifName; }
    /**
     * Get interface name
     * @return interface name or empty string if object is not initialized
     */
    const char *ifName_c() { return m_ifName.c_str(); }
    /**
     * Get interface MAC address
     * @return binary from address or empty string if object is not initialized
     */
    const std::string &mac() { return m_mac; }
    /**
     * Get interface MAC address
     * @return binary from address or empty string if object is not initialized
     */
    const uint8_t *mac_c() { return (uint8_t *)m_mac.c_str(); }
    /**
     * Get interface MAC address length
     * @return binary from address length or 0 if object is not initialized
     */
    size_t mac_size() { return m_mac.length(); }
    /**
     * Get interface PTP index
     * @return ptp index or -1 if object is not initialized or
     *  interface does not support PTP
     */
    int ptpIndex() { return m_ptpIndex; }
    /**
     * Convert binary from MAC address to string from
     * @param[in] mac binary from address
     * @param[in] len mac binary from length
     * @return string form MAC address
     */
    static std::string mac2str(const uint8_t *mac, size_t len);
    /**
     * Convert binary from MAC address to string from
     * @param[in] mac binary from address
     * @return string form MAC address
     */
    static std::string mac2str(const std::string &mac) {
        return mac2str((uint8_t *)mac.c_str(), mac.length());
    }
    /**
     * Convert string from MAC address to binary from
     * @param[in] string from address
     * @return string object with binary form MAC address
     */
    static std::string str2mac(const char *string);
    /**
     * Convert string from MAC address to binary from
     * @param[in] string from address
     * @return string object with binary form MAC address
     */
    static std::string str2mac(const std::string &string) {
        return str2mac(string.c_str(), string.length());
    }
    /**
     * Convert binary from MAC address to 64 bits binary from MAC address
     * @param[in] mac binary from address
     * @param[in] len mac binary from length
     * @return binary form 64 bits MAC address
     */
    static std::string eui48toeui64(const uint8_t *mac, size_t len);
    /**
     * Convert binary from MAC address to 64 bits binary from MAC address
     * @param[in] mac binary from address
     * @return binary form 64 bits MAC address
     */
    static std::string eui48toeui64(const std::string &mac) {
        return eui48toeui64((uint8_t *)mac.c_str(), mac.length());
    }
};

/**
 * @brief PTP clock id generator
 * @details
 *  Create a dynamic clock id for a PTP clock, that the application
 *  can use to fetch the PTP clock time
 * @note
 *  The dynamic clock id exist as long as the object exist.
 *  Do @b NOT delete the object while using the dynamic clock id.
 * @note
 *  As a network interface might have more then 1 PHC
 *  It is better to use a separate the class.
 */
class ptpClock
{
  private:
    int m_fd;
    int m_ptpIndex;
    bool m_isInit;
    clockid_t m_clkId;
    std::string m_ptpDevice;

  public:
    /**
     * Allocate a ptpClock object and initialize it
     * @param[in] ptpIndex PTP index
     */
    ptpClock(int ptpIndex);
    ~ptpClock();
    /**
     * Is object initialized
     * @return true if PTP clock exist and object was successfully initialized
     */
    bool isInit() { return m_isInit; }
    /**
     * Get dynamic clock id
     * @return dynamic clock id or -1 if not initialized
     */
    clockid_t clkId() { return m_clkId; }
    /**
     * Get PTP index
     * @return index or -1 if not initialized
     */
    int ptpIndex() { return m_ptpIndex; }
    /**
     * Get PTP device name
     * @return device name or empty string if not initialized
     */
    const std::string &ptpDevice() { return m_ptpDevice; }
    /**
     * Get PTP device name
     * @return device name or empty string if not initialized
     */
    const char *ptpDevice_c() { return m_ptpDevice.c_str(); }
};

#endif /*__IF_H*/
