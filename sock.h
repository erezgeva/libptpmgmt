/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* sock.h use unix socket to communicate with ptp4l
 *
 * Authors: Erez Geva <ErezGeva2@gmail.com>
 *
 */

#ifndef __SOCK_H
#define __SOCK_H

#include <string>
#include <stdint.h>
#include <netinet/in.h>
#include <sys/un.h>
#include "msg.h"
#include "cfg.h"
#include "ptp.h"

class sockBase
{
  protected:
    int m_fd;
    bool m_isInit;
    sockBase();
    bool sendReply(ssize_t cnt, size_t len);
  public:
    virtual ~sockBase();
    virtual void close();
    virtual bool init() = 0;
    virtual bool send(const void *msg, size_t len) = 0;
    virtual ssize_t rcv(void *buf, size_t bufSize, bool block = true) = 0;
    bool send(message &msg);
    int getFd() { return m_fd; }
    /*
     * Single socket polling
     * If user need multiple socket,
     * then fetch the file description with getFd()
     * And implement it, or merge it into an existing polling
     */
    bool poll(uint64_t timeout_ms = 0);
    bool tpoll(uint64_t &timeout_ms); // poll with timeout update
};

class sockUnix : public sockBase
{
  private:
    std::string m_me;
    std::string m_peer;
    std::string m_homeDir;
    sockaddr_un m_peerAddr;
    bool setPeerInternal(const std::string &str);
    bool setPeerInternal(const char *str);
    bool sendAny(const void *msg, size_t len, sockaddr_un &addr);

  public:
    sockUnix();
    void close();
    bool init();
    const std::string &getPeerAddress();
    const char *getPeerAddress_c();
    bool setPeerAddress(const std::string &str);
    bool setPeerAddress(const char *str);
    bool setPeerAddress(configFile &cfg, const char *section = nullptr);
    bool setPeerAddress(configFile &cfg, const std::string &section);
    const std::string &getSelfAddress();
    const char *getSelfAddress_c();
    bool setSelfAddress(const std::string &str);
    bool setSelfAddress(const char *str);
    bool setDefSelfAddress(std::string &rootBase, std::string &useDef);
    bool setDefSelfAddress(std::string &rootBase);
    bool setDefSelfAddress(const char *rootBase = nullptr,
        const char *useDef = nullptr);
    const std::string &getHomeDir();
    const char *getHomeDir_c();
    bool send(const void *msg, size_t len);
    bool sendTo(const void *msg, size_t len, const char *addrStr);
    bool sendTo(const void *msg, size_t len, std::string &addrStr);
    ssize_t rcv(void *buf, size_t bufSize, bool block = true);
    ssize_t rcvFrom(void *buf, size_t bufSize, std::string &from,
        bool block = true);
};

class sockBaseIf : public sockBase
{
  protected:
    std::string m_ifName; // interface to use
    std::string m_mac;
    int m_ifIndex;
    bool m_have_if;
    bool set(ptpIf &ifObj);
    sockBaseIf() : m_have_if(false) {}

  public:
    bool setIf(const std::string &ifName);
    bool setIf(const char *ifName);
    bool setIf(int ifIndex);
    bool setIf(ptpIf &ifObj);
};

class sockIp : public sockBaseIf
{
  protected:
    int m_domain;
    int m_udp_ttl;
    /* First for bind then for send */
    sockaddr *m_addr;
    size_t m_addr_len;
    const char *m_mcast_str; // string form
    uint8_t m_mcast[INET6_ADDRSTRLEN]; // binary form
    sockIp(int domain, const char *mcast, sockaddr *addr, size_t len);
    virtual bool init2() = 0;

  public:
    bool setUdpTtl(uint8_t udp_ttl);
    bool setUdpTtl(configFile &cfg, const char *section = nullptr);
    bool setUdpTtl(configFile &cfg, const std::string &section);
    bool send(const void *msg, size_t len);
    ssize_t rcv(void *buf, size_t bufSize, bool block = true);
    bool init();
};

class sockIp4 : public sockIp
{
  private:
    sockaddr_in m_addr4;

  protected:
    bool init2();

  public:
    sockIp4();
};

class sockIp6 : public sockIp
{
  private:
    sockaddr_in6 m_addr6;
    int m_udp6_scope;

  protected:
    bool init2();

  public:
    sockIp6();
    bool setScope(uint8_t udp6_scope);
    bool setScope(configFile &cfg, const char *section = nullptr);
    bool setScope(configFile &cfg, const std::string &section);
};

class sockRaw : public sockBaseIf
{
  private:
    std::string m_ptp_dst_mac;
    int m_socket_priority;
    iovec m_iov_tx[2], m_iov_rx[2];
    msghdr m_msg_tx, m_msg_rx;
    ethhdr m_hdr;
    uint8_t m_rx_buf[sizeof(ethhdr)];

  public:
    sockRaw();
    // String form
    bool setPtpDstMacStr(const std::string &str);
    bool setPtpDstMacStr(const char *str);
    // binary form
    bool setPtpDstMac(const std::string &ptp_dst_mac);
    bool setPtpDstMac(const uint8_t *ptp_dst_mac, size_t len);
    // using configuration file
    bool setPtpDstMac(configFile &cfg, const char *section = nullptr);
    bool setPtpDstMac(configFile &cfg, const std::string &section);
    bool setSocketPriority(uint8_t udp6_scope);
    bool setSocketPriority(configFile &cfg, const char *section = nullptr);
    bool setSocketPriority(configFile &cfg, const std::string &section);
    bool init();
    bool send(const void *msg, size_t len);
    ssize_t rcv(void *buf, size_t bufSize, bool block = true);
};

#endif /*__SOCK_H*/
