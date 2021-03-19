/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* ptp.h Read network interface information and retrieve the PTP information
 *
 * Authors: Erez Geva <ErezGeva2@gmail.com>
 *
 */

#ifndef __IF_H
#define __IF_H

#include <string>
#include <stdint.h>
#include <net/if.h>
#include <linux/ethtool.h>

const size_t EUI48 = ETH_ALEN;
const size_t EUI64 = 8;

class ptpIf
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
    ptpIf() : m_isInit(false), m_ifIndex(-1), m_ptpIndex(-1) {}
    bool init(const char *ifName);
    bool init(const std::string &ifName);
    bool init(int ifIndex);
    bool isInit() { return m_isInit; }
    int ifIndex() { return m_ifIndex; }
    const std::string &ifName() { return m_ifName; }
    const char *ifName_c() { return m_ifName.c_str(); }
    const std::string &mac() { return m_mac; }
    const char *mac_c() { return m_mac.c_str(); }
    int ptpIndex() { return m_ptpIndex; }
    static std::string mac2str(const uint8_t *mac, size_t len);
    static std::string mac2str(const std::string &mac) {
        return mac2str((uint8_t *)mac.c_str(), mac.length());
    }
    static std::string str2mac(const char *str);
    static std::string str2mac(const std::string &str) {
        return str2mac(str.c_str(), str.length());
    }
    static std::string eui48toeui64(const uint8_t *mac, size_t len);
    static std::string eui48toeui64(const std::string &mac) {
        return eui48toeui64((uint8_t *)mac.c_str(), mac.length());
    }
};

/*
 * As a network interface might have more then 1 PHC
 * It is better to separate the classes.
 * The clock ID is dynamiclly allocated.
 * The socket in the object MUST stay open as long as
 * the application uses the clock ID!
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
    ptpClock(int ptpIndex);
    ~ptpClock();
    bool isInit() { return m_isInit; }
    clockid_t clkId() { return m_clkId; }
    int ptpIndex() { return m_ptpIndex; }
    const std::string &ptpDevice() { return m_ptpDevice; }
    const char *ptpDevice_c() { return m_ptpDevice.c_str(); }
};

#endif /*__IF_H*/
