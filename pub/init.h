/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Init a pmc application
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 */

#ifndef __PTPMGMT_INIT_H
#define __PTPMGMT_INIT_H

#ifdef __cplusplus
#include "msg.h"
#include "opt.h"
#include "sock.h"

__PTPMGMT_NAMESPACE_BEGIN

/**
 * Initilize configuration file, socket, message based on PMC options.
 * The user can write an application using the pmc command line.
 */
class Init
{
  private:
    ConfigFile m_cfg;
    SaFile m_sa;
    Message m_msg;
    std::unique_ptr<SockBase> m_sk;
    char m_net_select = 0;
    bool m_use_uds = false;
    uint8_t m_allow_unauth = 0;

  public:
    /**
     * Close socket
     */
    void close();

    /**
     * Process PMC options
     * @param[in] opt PMC options
     * @return 0 on scuccess
     * @note function return proper value to return from main()
     */
    int process(const Options &opt);

    /**
     * Get configuration file object
     * @return object
     */
    const ConfigFile &cfg() const;

    /**
     * Get authentication security association file object
     * @return object
     */
    const SaFile &sa() const;

    /**
     * Get Message object
     * @return object
     */
    Message &msg();

    /**
     * Get Socket object
     * @return object or null if not exist
     * @note User @b should not try to free this socket object
     */
    SockBase *sk() const;

    /**
     * Get network selection character
     * @return
     *   'u' for unix socket using a SockUnix object,
     *   '4' for a PTP IPv4 socket using a SockIp4 object,
     *   '6' for a PTP IPv6 socket using a SockIp6 object,
     *   '2' for a PTP layer 2 socket using a SockRaw object,
     */
    char getNetSelect() const;

    /**
     * Is the socket provide by this object, a Unix socket?
     * @return true if the socket is a UDS socket
     */
    bool use_uds() const;

    /**
     * Allow unauthnticated parameter
     * @return Allow unauthnticated value
     */
    uint8_t allow_unauth() const;
};

__PTPMGMT_NAMESPACE_END
#else /* __cplusplus */
#include "c/init.h"
#endif /* __cplusplus */

#endif /* __PTPMGMT_INIT_H */
