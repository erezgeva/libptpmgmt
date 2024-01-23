/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief C interface to sockets classes
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2023 Erez Geva
 *
 */

#ifndef __PTPMGMT_C_SOCK_H
#define __PTPMGMT_C_SOCK_H

#include <sys/types.h>
#include "c/cfg.h"
#include "c/ptp.h"

/** pointer to ptpmgmt socket structure */
typedef struct ptpmgmt_sk_t *ptpmgmt_sk;

/** pointer to constant ptpmgmt socket structure */
typedef const struct ptpmgmt_sk_t *const_ptpmgmt_sk;

/**
 * The ptpmgmt socket structure hold the socket object
 *  and call backs to call C++ methods
 */
struct ptpmgmt_sk_t {
    /**< @cond internal */
    void *_this; /**< pointer to actual C++ socket object */
    /**< @endcond */

    /**
     * Free a socket object
     * @param[in] sk socket
     */
    void (*free)(ptpmgmt_sk sk);

    /**
     * close socket and release its resources
     * @param[in, out] sk socket
     */
    void (*close)(ptpmgmt_sk sk);
    /**
     * Allocate the socket and initialize it with current parameters
     * @param[in, out] sk socket
     * @return true if socket creation success
     */
    bool (*init)(ptpmgmt_sk sk);
    /**
     * Send the message using the socket
     * @param[in, out] sk socket
     * @param[in] msg pointer to message memory buffer
     * @param[in] len message length
     * @return true if message is sent
     * @note true does @b NOT guarantee the frame was successfully
     *  arrives its target. Only the network layer sends it.
     */
    bool (*send)(ptpmgmt_sk sk, const void *msg, size_t len);
    /**
     * Receive a message using the socket
     * @param[in, out] sk socket
     * @param[in, out] buf pointer to a memory buffer
     * @param[in] bufSize memory buffer size
     * @param[in] block true, wait till a packet arrives.
     *                  false, do not wait, return error
     *                  if no packet available
     * @return number of bytes received or negative on failure
     */
    ssize_t (*rcv)(ptpmgmt_sk sk, void *buf, size_t bufSize, bool block);
    /**
     * Get socket file description
     * @param[in] sk socket
     * @return socket file description
     * @note Can be used to poll, send or receive from socket.
     *  The user is advice to use properly. Do @b NOT free the socket.
     *  If you want to close the socket use the close function @b ONLY.
     */
    int (*getFd)(const_ptpmgmt_sk sk);
    /**
     * Get socket file description
     * @param[in] sk socket
     * @return socket file description
     * @note Can be used to poll, send or receive from socket.
     *  The user is advice to use properly. Do @b NOT free the socket.
     *  If you want to close the socket use the close function @b ONLY.
     */
    int (*fileno)(const_ptpmgmt_sk sk);
    /**
     * Single socket polling
     * @param[in] sk socket
     * @param[in] timeout_ms timeout in milliseconds,
     *  until receive a packet. use 0 for blocking.
     * @return true if a packet is ready for receive
     * @note If user need multiple socket,
     *  then fetch the file description with fileno()
     *  And implement it, or merge it into an existing polling
     */
    bool (*poll)(ptpmgmt_sk sk, uint64_t timeout_ms);
    /**
     * Single socket polling and update timeout
     * @param[in] sk socket
     * @param[in, out] timeout_ms timeout in milliseconds
     *  until receive a packet. use 0 for blocking.
     * @return true if a packet is ready for receive
     * @note The function will reduce the wait time from timeout
     *  when packet arrives. The user is advice to ensure the timeout
     *  is positive, as @b zero cause to block until receive a packet.
     */
    bool (*tpoll)(ptpmgmt_sk sk, uint64_t *timeout_ms);
    /**
     * Get peer address
     * @param[in] sk socket
     * @return string object with peer address
     */
    const char *(*getPeerAddress)(const_ptpmgmt_sk sk);
    /**
     * Is peer address abstract?
     * @param[in] sk socket
     * @return true if peer address is abstract address
     */
    bool (*isPeerAddressAbstract)(const_ptpmgmt_sk sk);
    /**
     * Set peer address
     * @param[in, out] sk socket
     * @param[in] string object with peer address
     * @return true if peer address is updated
     */
    bool (*setPeerAddress)(ptpmgmt_sk sk, const char *string);
    /**
     * Set peer address with Abstract address
     * @param[in, out] sk socket
     * @param[in] string object with peer address
     * @return true if peer address is updated
     * @note add '0' byte at the start of the address
     *       to mark it as abstract socket address
     */
    bool (*setPeerAddressA)(ptpmgmt_sk sk, const char *string);
    /**
     * Set peer address using configuration file
     * @param[in, out] sk socket
     * @param[in] cfg reference to configuration file object
     * @param[in] section in configuration file
     * @return true if peer address is updated
     * @note calling without section will fetch value from @"global@" section
     */
    bool (*setPeerAddressCfg)(ptpmgmt_sk sk, const_ptpmgmt_cfg cfg,
        const char *section);
    /**
     * Get self address
     * @param[in] sk socket
     * @return string object with self address
     */
    const char *(*getSelfAddress)(const_ptpmgmt_sk sk);
    /**
     * Is self address abstract?
     * @param[in] sk socket
     * @return true if self address is abstract address
     */
    bool (*isSelfAddressAbstract)(const_ptpmgmt_sk sk);
    /**
     * Set self address
     * @param[in, out] sk socket
     * @param[in] string object with self address
     * @return true if self address is updated
     * @note address can not be changed after initializing.
     *  User can close the socket, change this value, and
     *  initialize a new socket.
     */
    bool (*setSelfAddress)(ptpmgmt_sk sk, const char *string);
    /**
     * Set self address with Abstract address
     * @param[in, out] sk socket
     * @param[in] string object with self address
     * @return true if self address is updated
     * @note address can not be changed after initializing.
     *  User can close the socket, change this value, and
     *  initialize a new socket.
     * @note add '0' byte at the start of the address
     *       to mark it as abstract socket address
     */
    bool (*setSelfAddressA)(ptpmgmt_sk sk, const char *string);
    /**
     * Set self address using predefined algorithm
     * @param[in, out] sk socket
     * @param[in] rootBase base used for root user
     * @param[in] useDef base used for non root user
     * @return true if self address is updated
     * @note address can not be changed after initializing.
     *  User can close the socket, change this value, and
     *  initialize a new socket.
     */
    bool (*setDefSelfAddress)(ptpmgmt_sk sk, const char *rootBase,
        const char *useDef);
    /**
     * Get user home directory
     * @param[in, out] sk socket
     * @return string object with home directory
     */
    const char *(*getHomeDir)(ptpmgmt_sk sk);
    /**
     * Send the message using the socket to a specific address
     * @param[in] sk socket
     * @param[in] msg pointer to message memory buffer
     * @param[in] len message length
     * @param[in] addrStr Unix socket address (socket file)
     * @return true if message is sent
     * @note true does @b NOT guarantee the frame was successfully
     *  arrives its target. Only the network layer sends it.
     */
    bool (*sendTo)(ptpmgmt_sk sk, const void *msg, size_t len,
        const char *addrStr);
    /**
     * Send the message using the socket to a specific abstract soket address
     * @param[in] sk socket
     * @param[in] msg pointer to message memory buffer
     * @param[in] len message length
     * @param[in] addrStr Unix abstract socket address
     * @return true if message is sent
     * @note true does @b NOT guarantee the frame was successfully
     *  arrives its target. Only the network layer sends it.
     * @note add '0' byte at the start of the address
     *       to mark it as abstract socket address
     */
    bool (*sendToA)(ptpmgmt_sk sk, const void *msg, size_t len,
        const char *addrStr);
    /**
     * Receive a message using the socket from any address
     * @param[in] sk socket
     * @param[in, out] buf pointer to a memory buffer
     * @param[in] bufSize memory buffer size
     * @param[out] from Unix socket address (socket file)
     * @param[in, out] fromSize from string buffer size
     *                  Actual from string size on return
     * @param[in] block true, wait till a packet arrives.
     *                  false, do not wait, return error
     *                  if no packet available
     * @return number of bytes received or negative on failure
     * @note from store the origin address which send the packet
     */
    ssize_t (*rcvFrom)(ptpmgmt_sk sk, void *buf, size_t bufSize, char *from,
        size_t *fromSize, bool block);
    /**
     * Receive a message using the socket from any address
     * @param[in, out] sk socket
     * @param[in, out] buf pointer to a memory buffer
     * @param[in] bufSize memory buffer size
     * @param[in] block true, wait till a packet arrives.
     *                  false, do not wait, return error
     *                  if no packet available
     * @return number of bytes received or negative on failure
     * @note use getLastFrom() to fetch origin address which send the packet
     */
    ssize_t (*rcvFromA)(ptpmgmt_sk sk, void *buf, size_t bufSize, bool block);
    /**
     * Fetch origin address from last rcvFrom() call
     * @param[in] sk socket
     * @return Unix socket address
     * @note store address only on the rcvFrom() call without the from parameter
     * @attention no protection or thread safe, fetch last rcvFrom() call with
     *  this object.
     */
    const char *(*getLastFrom)(const_ptpmgmt_sk sk);
    /**
     * Is last from address abstract?
     * @param[in] sk socket
     * @return true if last from address is abstract address
     */
    bool (*isLastFromAbstract)(const_ptpmgmt_sk sk);
    /**
     * Set network interface using its name
     * @param[in, out] sk socket
     * @param[in] ifName interface name
     * @return true if network interface exists and updated.
     * @note network interface can not be changed after initializing.
     *  User can close the socket, change this value, and
     *  initialize a new socket.
     */
    bool (*setIfUsingName)(ptpmgmt_sk sk, const char *ifName);
    /**
     * Set network interface using its index
     * @param[in, out] sk socket
     * @param[in] ifIndex interface index
     * @return true if network interface exists and updated.
     * @note network interface can not be changed after initializing.
     *  User can close the socket, change this value, and
     *  initialize a new socket.
     */
    bool (*setIfUsingIndex)(ptpmgmt_sk sk, int ifIndex);
    /**
     * Set network interface using a network interface object
     * @param[in, out] sk socket
     * @param[in] ifObj initialized network interface object
     * @return true if network interface exists and updated.
     * @note network interface can not be changed after initializing.
     *  User can close the socket, change this value, and
     *  initialize a new socket.
     */
    bool (*setIf)(ptpmgmt_sk sk, const_ptpmgmt_ifInfo ifObj);
    /**
     * Set all socket parameters using a network interface object and
     *  a configuration file
     * @param[in, out] sk socket
     * @param[in] ifObj initialized network interface object
     * @param[in] cfg reference to configuration file object
     * @param[in] section in configuration file
     * @return true if network interface exists and update all parameters.
     * @note parameters can not be changed after initializing.
     *  User can close the socket, change any of the parameters value and
     *  initialize a new socket.
     * @note calling without section will fetch value from @"global@" section
     */
    bool (*setAll)(ptpmgmt_sk sk, const_ptpmgmt_ifInfo ifObj,
        const_ptpmgmt_cfg cfg, const char *section);
    /**
     * Set all socket parameters using a network interface object and
     *  a configuration file and initialize
     * @param[in, out] sk socket
     * @param[in] ifObj initialized network interface object
     * @param[in] cfg reference to configuration file object
     * @param[in] section in configuration file
     * @return true if network interface exists and update all parameters.
     * @note parameters can not be changed after initializing.
     *  User can close the socket, change any of the parameters value and
     *  initialize a new socket.
     * @note calling without section will fetch value from @"global@" section
     */
    bool (*setAllInit)(ptpmgmt_sk sk, const_ptpmgmt_ifInfo ifObj,
        const_ptpmgmt_cfg cfg, const char *section);
    /**
     * Set IP ttl value
     * @param[in, out] sk socket
     * @param[in] udp_ttl IP time to live
     * @return true if IP ttl is updated
     * @note in IP version 6 the value is used for multicast hops
     * @note IP ttl can not be changed after initializing.
     *  User can close the socket, change this value, and
     *  initialize a new socket.
     */
    bool (*setUdpTtl)(ptpmgmt_sk sk, uint8_t udp_ttl);
    /**
     * Set IP ttl value using configuration file
     * @param[in, out] sk socket
     * @param[in] cfg reference to configuration file object
     * @param[in] section in configuration file
     * @return true if IP ttl is updated
     * @note in IP version 6 the value is used for multicast hops
     * @note IP ttl can not be changed after initializing.
     *  User can close the socket, change this value, and
     *  initialize a new socket.
     * @note calling without section will fetch value from @"global@" section
     */
    bool (*setUdpTtlCfg)(ptpmgmt_sk sk, const_ptpmgmt_cfg cfg,
        const char *section);
    /**
     * Set IP version 6 address scope
     * @param[in, out] sk socket
     * @param[in] udp6_scope IP version 6 address scope
     * @return true if IPv6 scope is updated
     * @note IPv6 scope can not be changed after initializing.
     *  User can close the socket, change this value, and
     *  initialize a new socket.
     */
    bool (*setScope)(ptpmgmt_sk sk, uint8_t udp6_scope);
    /**
     * Set IP version 6 address scope using configuration file
     * @param[in, out] sk socket
     * @param[in] cfg reference to configuration file object
     * @param[in] section in configuration file
     * @return true if IPv6 scope is updated
     * @note IPv6 scope can not be changed after initializing.
     *  User can close the socket, change this value, and
     *  initialize a new socket.
     * @note calling without section will fetch value from @"global@" section
     */
    bool (*setScopeCfg)(ptpmgmt_sk sk, const_ptpmgmt_cfg cfg,
        const char *section);
    /**
     * Set PTP multicast address using string from
     * @param[in, out] sk socket
     * @param[in] string address in a string object
     * @return true if PTP multicast address is updated
     * @note PTP multicast address can not be changed after initializing.
     *  User can close the socket, change this value, and
     *  initialize a new socket.
     * @note function convert address to binary form and return false
     *  if conversion fail (address is using wrong format).
     */
    bool (*setPtpDstMacStr)(ptpmgmt_sk sk, const char *string);
    /**
     * Set PTP multicast address using binary from
     * @param[in, out] sk socket
     * @param[in] ptp_dst_mac address in binary string object
     * @param[in] len MAC address length
     * @return true if PTP multicast address is updated
     * @note PTP multicast address can not be changed after initializing.
     *  User can close the socket, change this value, and
     *  initialize a new socket.
     */
    bool (*setPtpDstMac)(ptpmgmt_sk sk, const void *ptp_dst_mac,
        size_t len);
    /**
     * Set PTP multicast address using configuration file
     * @param[in, out] sk socket
     * @param[in] cfg reference to configuration file object
     * @param[in] section in configuration file
     * @return true if PTP multicast address is updated
     * @note PTP multicast address can not be changed after initializing.
     *  User can close the socket, change this value, and
     *  initialize a new socket.
     * @note calling without section will fetch value from @"global@" section
     */
    bool (*setPtpDstMacCfg)(ptpmgmt_sk sk, const_ptpmgmt_cfg cfg,
        const char *section);
    /**
     * Set socket priority
     * @param[in, out] sk socket
     * @param[in] socket_priority socket priority value
     * @return true if socket priority is updated
     * @note socket priority can not be changed after initializing.
     *  User can close the socket, change this value, and
     *  initialize a new socket.
     * @note the priority is used by network layer,
     *  it is not part of the packet
     */
    bool (*setSocketPriority)(ptpmgmt_sk sk, uint8_t socket_priority);
    /**
     * Set socket priority using configuration file
     * @param[in, out] sk socket
     * @param[in] cfg reference to configuration file object
     * @param[in] section in configuration file
     * @return true if socket priority is updated
     * @note socket priority can not be changed after initializing.
     *  User can close the socket, change this value, and
     *  initialize a new socket.
     * @note calling without section will fetch value from @"global@" section
     */
    bool (*setSocketPriorityCfg)(ptpmgmt_sk sk, const_ptpmgmt_cfg cfg,
        const char *section);
};

/**
 * Enumerator for the available socket classes, we support
 */
enum ptpmgmt_socket_class {
    ptpmgmt_SockUnix,
    ptpmgmt_SockIp4,
    ptpmgmt_SockIp6,
    ptpmgmt_SockRaw,
};

/**
 * Allocate a new socket structure
 * @param[in] type select which class to use
 * Return pointer to a new ptpmgmt_sk socket structre with new object
 *        or null on error
 */
ptpmgmt_sk ptpmgmt_sk_alloc(enum ptpmgmt_socket_class type);

#endif /* __PTPMGMT_C_SOCK_H */
