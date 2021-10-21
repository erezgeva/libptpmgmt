/* SPDX-License-Identifier: LGPL-3.0-or-later */

/** @file
 * @brief handle command line options
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 * @copyright GNU Lesser General Public License 3.0 or later
 *
 */

#ifndef __PMC_OPT_H
#define __PMC_OPT_H

#include <string>
#include <getopt.h>
#include <vector>
#include <map>

/**
 * @brief structre to add new option
 */
struct pmc_option {
    /**
     * Uniq single character
     * For long only options, used by application
     */
    const char short_name;
    const char *long_name; /**< Optional long option */
    bool have_arg; /**< Use argument flag */
    bool long_only; /**< Use long option only flag */
    const char *help_msg; /**< Help message */
    const char *arg_help; /**< Argument name for help */
    const char *def_val; /**< Defualt value for help */
};

/**
 * @brief hold and proccess options
 */
class Options
{
  private:
    static pmc_option start[];
    size_t max_arg_name;
    std::vector<option> long_options_list;
    std::map<int, std::string> options;
    std::string help;
    std::string net_options;
    std::string all_options;
    std::string all_short_options;
    std::string with_options;
    std::string msg;
    char net_select;
    int argc;
    int end_optind;

  public:
    /** parsing return code */
    enum loop_val {
        OPT_ERR, /**< Parsing error */
        OPT_EMSG, /**< Parsing error with message */
        OPT_MSG, /**< Message from parsing */
        OPT_HELP, /**< Need to print help */
        OPT_DONE, /**< parsing success */
    };
    /**
     * constructor
     * @param[in] max_arg_name maximum option argument name for help message
     */
    Options(size_t max_arg_name = 4);
    /**
     * Insert option
     * @param[in] opt new option parameters
     * @return true on adding the option
     * @note should be called before calling parse_options()
     */
    bool insert(const pmc_option &opt);
    /**
     * Get help message
     * @return help message
     */
    const char *get_help() { return help.c_str(); }
    /**
     * Get parse_options() message
     * @return message from last parse_options()
     */
    const char *get_msg() { return msg.c_str(); }
    /**
     * Parse command line
     * @param[in] argc number of arguments
     * @param[in] argv array of command line arguments
     * @return Parse state
     */
    loop_val parse_options(int argc, char *const argv[]);
    /**
     * Is option on command line
     * @param[in] opt short option character
     * @return true if option on command line
     */
    bool have(char opt) { return options.count(opt) > 0; }
    /**
     * get option value
     * @param[in] opt short option character
     * @return option value
     * @note relevant for option with argument
     */
    const std::string val(char opt) { return have(opt) ? options[opt] : ""; }
    /**
     * get option value
     * @param[in] opt short option character
     * @return option value in char pointer (C style)
     * @note relevant for option with argument
     */
    const char *val_c(char opt) { return have(opt) ? options[opt].c_str() : ""; }
    /**
     * get option integer value
     * @param[in] opt short option character
     * @return option value in char pointer (C style)
     * @note relevant for option with argument
     */
    const int val_i(char opt) { return have(opt) ? atoi(options[opt].c_str()) : 0; }
    /**
     * get Network Transport value
     * @return Network Transport
     * @note return 0 if not select on command line
     */
    const char get_net_transport() { return net_select; }
    /**
     * Do we have more argumends on the command line we did not proccess
     * @return true if we have more to proccess
     */
    bool have_more() { return end_optind < argc; }
    /**
     * First argumend on the command line that we did not proccess
     * @return index of argument
     */
    int procces_next() { return end_optind; }
};

#endif /*__PMC_OPT_H*/
