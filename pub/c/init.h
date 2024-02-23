/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief C interface to initialize a pmc application class
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2023 Erez Geva
 *
 */

#ifndef __PTPMGMT_C_INIT_H
#define __PTPMGMT_C_INIT_H

#include "c/opt.h"
#include "c/sock.h"
#include "c/msg.h"

/** pointer to ptpmgmt initialization structure */
typedef struct ptpmgmt_init_t *ptpmgmt_init;

/** pointer to constant ptpmgmt initialization structure */
typedef const struct ptpmgmt_init_t *const_ptpmgmt_init;

/**
 * The ptpmgmt initialization structure hold the initialization object
 *  and call backs to call C++ methods
 */
struct ptpmgmt_init_t {
    /**< @cond internal */
    void *_this; /**< pointer to actual C++ initialization object */
    ptpmgmt_cfg sCfg; /**< pointer to configuration structure */
    ptpmgmt_msg sMsg; /**< pointer to message structure */
    ptpmgmt_sk sSk; /**< pointer to socket sturcture */
    /**< @endcond */

    /**
     * Free init object
     * @param[in] i init object
     */
    void (*free)(ptpmgmt_init i);
    /**
     * close the socket in the init object
     * @param[in, out] i init object
     */
    void (*close)(ptpmgmt_init i);
    /**
     * Process PMC options
     * @param[in, out] i init object
     * @param[in] opt PMC options
     * @return 0 on scuccess
     * @note function return proper value to return from main()
     */
    int (*process)(ptpmgmt_init i, const_ptpmgmt_opt opt);
    /**
     * Get configuration file object
     * @param[in, out] i init object
     * @return configuration object
     */
    ptpmgmt_cfg(*cfg)(ptpmgmt_init i);
    /**
     * Get Message object
     * @return object
     */
    ptpmgmt_msg(*msg)(ptpmgmt_init i);
    /**
     * Get Socket object
     * @param[in, out] i init object
     * @return object or null if not exist
     * @note User @b should not try to free this socket object
     */
    ptpmgmt_sk(*sk)(ptpmgmt_init i);

    /**
     * Get network selection character
     * @param[in] i init object
     * @return
     *   'u' for unix socket using a SockUnix object,
     *   '4' for a PTP IPv4 socket using a SockIp4 object,
     *   '6' for a PTP IPv6 socket using a SockIp6 object,
     *   '2' for a PTP layer 2 socket using a SockRaw object,
     *   0 on error.
     */
    char (*getNetSelect)(const_ptpmgmt_init i);

    /**
     * Is the socket provide by this object, a Unix socket?
     * @param[in] i init object
     * @return true if the socket is a UDS socket
     */
    bool (*use_uds)(ptpmgmt_init i);
};

/**
 * Alocate init object
 * @return new allocated object or null in case of error
 */
ptpmgmt_init ptpmgmt_init_alloc();

#endif /* __PTPMGMT_C_INIT_H */
