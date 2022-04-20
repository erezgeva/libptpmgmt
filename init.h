/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/** @file
 * @brief init a pmc application
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#ifndef __PTPMGMT_INIT_H
#define __PTPMGMT_INIT_H

#include <string>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include "msg.h"
#include "ptp.h"
#include "opt.h"
#include "sock.h"

/**
 * Initilize configuration file, socket, message based on PMC options.
 * The user can write an application using the pmc command line.
 */
class Init
{
  private:
    ConfigFile m_cfg;
    Message m_msg;
    SockBase *m_sk;
    bool m_use_uds;

  public:

    Init() : m_sk(nullptr) {}
    ~Init() { close(); }

    /**
     * Close socket and delete it
     */
    void close();

    /**
     * Proccess PMC options
     * @param[in] opt PMC options
     * @return 0 on scuccess
     */
    int proccess(const Options &opt);

    /**
     * Get configuration file object
     * @return object
     */
    const ConfigFile &cfg() const { return m_cfg; }

    /**
     * Get Message object
     * @return object
     */
    Message &msg() { return m_msg; }

    /**
     * Get Socket object
     * @return object
     */
    SockBase *sk() { return m_sk; }

    /**
     * Do we use UDS socket?
     * @return true if use UDS socket
     */
    bool use_uds() const { return m_use_uds; }
};

#endif /* __PTPMGMT_INIT_H */
