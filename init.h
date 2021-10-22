/* SPDX-License-Identifier: LGPL-3.0-or-later */

/** @file
 * @brief init a pmc application
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 * @copyright GNU Lesser General Public License 3.0 or later
 *
 */

#ifndef __PMC_INIT_H
#define __PMC_INIT_H

#include "msg.h"
#include "ptp.h"
#include "opt.h"
#include "sock.h"

/**
 * Initilize configuration file, socket, message and
 *  command lines parsing objects.
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
    ~Init();

    /**
     * Close socket and delete it
     */
    void close();

    /**
     * Proccess initilizing
     * @param[in] opt object
     * @return xxxx
     */
    int proccess(Options &opt);

    /**
     * Get configuration file object
     * @return object
     */
    ConfigFile &cfg() { return m_cfg; }

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
    bool use_uds() { return m_use_uds; }
};

#endif /*__PMC_INIT_H*/
