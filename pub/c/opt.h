/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2023 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief C interface to command line options parsing class
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2023 Erez Geva
 *
 */

#ifndef __PTPMGMT_C_OPT_H
#define __PTPMGMT_C_OPT_H

#include "name.h"

/** Parsing return code. Identical to Options::loop_val */
enum ptpmgmt_opt_loop_val {
    PTPMGMT_OPT_ERR, /**< Parsing error */
    PTPMGMT_OPT_MSG, /**< Message from parsing */
    PTPMGMT_OPT_HELP, /**< Need to print help */
    PTPMGMT_OPT_DONE, /**< parsing success */
};
/** Structre to add new option */
struct ptpmgmt_opt_option {
    /** Uniq single character required as it is used by application */
    char short_name;
    const char *long_name; /**< Optional long option */
    bool have_arg; /**< Use argument flag */
    bool long_only; /**< Use long option only flag */
    const char *help_msg; /**< Help message */
    const char *arg_help; /**< Argument name for help */
    const char *def_val; /**< Default value for help */
};

/** pointer to ptpmgmt options structure */
typedef struct ptpmgmt_opt_t *ptpmgmt_opt;

/** pointer to constant ptpmgmt options structure */
typedef const struct ptpmgmt_opt_t *const_ptpmgmt_opt;

/**
 * The ptpmgmt options structure hold the options object
 *  and call backs to call C++ methods
 */
struct ptpmgmt_opt_t {
    /**< @cond internal */
    void *_this; /**< pointer to actual C++ options object */
    /**< @endcond */

    /**
     * Free an options object
     * @param[in] opt options object to free
     */
    void (*free)(ptpmgmt_opt opt);
    /**
     * Use PMC default options
     * @param[in, out] opt options object
     */
    void (*useDefOption)(ptpmgmt_opt opt);
    /**
     * Insert option
     * @param[in, out] opt options object
     * @param[in] addOpt new option parameters
     * @return true on adding the option
     * @note should be called before calling parse_options()
     */
    bool (*insert)(ptpmgmt_opt opt, struct ptpmgmt_opt_option *addOpt);
    /**
     * Get help message
     * @param[in, out] opt options object
     * @return help message
     */
    const char *(*get_help)(ptpmgmt_opt opt);
    /**
     * Get parse_options() message
     * @param[in] opt options object
     * @return message from last parse_options()
     */
    const char *(*get_msg)(ptpmgmt_opt opt);
    /**
     * Parse command line
     * @param[in, out] opt options object
     * @param[in] argc number of arguments
     * @param[in] argv array of command line arguments
     * @return Parse state
     * @note Follow the command line.
     *       Argument 0 is the application name,
     *       as appear in the command line.
     *       This class do not store it.
     */
    enum ptpmgmt_opt_loop_val(*parse_options)(ptpmgmt_opt opt, int argc,
        char *argv[]);
    /**
     * Is option on command line
     * @param[in] opt options object
     * @param[in] askOpt short option character
     * @return true if option on command line
     */
    bool (*have)(ptpmgmt_opt opt, char askOpt);
    /**
     * get option value
     * @param[in] opt options object
     * @param[in] valOpt short option character
     * @return option char pointer of value string (C style)
     * @note relevant for option with argument
     */
    const char *(*val)(ptpmgmt_opt opt, char valOpt);
    /**
     * get option integer value
     * @param[in] opt options object
     * @param[in] valOpt short option character
     * @return option integer value
     * @note relevant for option with argument of integer value
     */
    int (*val_i)(ptpmgmt_opt opt, char valOpt);
    /**
     * get Network Transport value
     * @param[in] opt options object
     * @return Network Transport
     * @note return 0 if not select on command line
     */
    char (*get_net_transport)(ptpmgmt_opt opt);
    /**
     * Do we have more argumends on the command line, left unprocessed
     * @param[in] opt options object
     * @return true if we have more to process
     */
    bool (*have_more)(ptpmgmt_opt opt);
    /**
     * First argumend on the command line, left unprocessed
     * @param[in] opt options object
     * @return index of argument
     */
    int (*process_next)(ptpmgmt_opt opt);
};

/**
 * Allocate new options object
 * @return new options object or null on error
 */
ptpmgmt_opt ptpmgmt_opt_alloc();
/**
 * Allocate new options object without the PMC default options
 * @return new options object or null on error
 */
ptpmgmt_opt ptpmgmt_opt_alloc_empty();

#endif /* __PTPMGMT_C_OPT_H */
