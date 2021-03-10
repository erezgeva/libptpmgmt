/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* sock.h use unix socket to communicate with ptp4l
 *
 * Authors: Erez Geva <ErezGeva2@gmail.com>
 *
 */

#ifndef __SOCK_H
#define __SOCK_H

#include <stdint.h>
#include <string>
#include <sys/un.h>
#include "cfg.h"

class sockUnix
{
private:
    int m_fd;
    bool m_isInit;
    std::string m_me;
    std::string m_peer;
    std::string m_homeDir;
    struct sockaddr_un m_peerAddr;
    bool setPeerInternal(const std::string &str);
    bool setPeerInternal(const char *str);
    bool sendAny(const void *msg, size_t len, struct sockaddr_un &addr);
public:
    sockUnix();
    ~sockUnix();
    void close();
    bool init();
    int getFd();
    const std::string &getPeerAddress();
    const char *getPeerAddress_c();
    bool setPeerAddress(const std::string &str);
    bool setPeerAddress(const char *str);
    bool setPeerAddress(configFile &cfg, const char *section = nullptr);
    bool setPeerAddress(configFile &cfg, std::string &section);
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
    /*
     * Single socket polling
     * If user need multiple socket,
     * then fetch the file description with getFd()
     * And implement it, or merge it into an existing polling
     */
    bool poll(uint64_t timeout_ms = 0);
};

#endif /*__SOCK_H*/
