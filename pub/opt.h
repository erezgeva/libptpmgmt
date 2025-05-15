/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Handle command line options
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 */

#ifndef __PTPMGMT_OPT_H
#define __PTPMGMT_OPT_H

#ifdef __cplusplus
#include <map>
#include <vector>
#include "name.h"
#include "buf.h"
#ifdef __PTPMGMT_HAVE_GETOPT_H
#include <getopt.h>
#endif

__PTPMGMT_NAMESPACE_BEGIN

/**
 * Structre to add new option
 */
struct Pmc_option {
    /**
     * Uniq single character
     * For long only options, used by application
     */
    char short_name;
    std::string long_name; /**< Optional long option */
    bool have_arg; /**< Use argument flag */
    bool long_only; /**< Use long option only flag */
    std::string help_msg; /**< Help message */
    std::string arg_help; /**< Argument name for help */
    std::string def_val; /**< Default value for help */
};

/**
 * Hold and process options
 */
class Options
{
  private:

    static Pmc_option startOptions[];
    size_t m_max_arg_name = 0;
    std::vector<option> m_long_opts_list;
    Buf m_strings; /**< store the string used in option */
    size_t m_last_strings = 0; /**< next string location in m_strings */
    std::map<int, std::string> m_opts;
    std::string m_net_opts, m_all_opts, m_all_short_opts, m_with_opts, m_msg;
    char m_net_select = 0;
    int m_argc = 0, m_end_optind = 0;
    bool m_useDef = false;

    class helpStore
    {
      private:
        std::string m_start, m_end;
      public:
        helpStore(const char *s, const char *e = nullptr): m_start(s) {
            if(e != nullptr)
                m_end = e;
        }
        helpStore &addStart(const std::string &s) {m_start += s; return *this;}
        helpStore &addStart(const char s) {m_start += s; return *this;}
        helpStore &addEnd(const std::string &e) {m_end += e; return *this;}
        std::string get(size_t length) const;
    };
    std::string help;
    std::vector<helpStore> helpVec;
    void update_help();

  public:
    /** parsing return code */
    enum loop_val {
        OPT_ERR, /**< Parsing error */
        OPT_MSG, /**< Pass a message from parsing like a version string */
        OPT_HELP, /**< Need to print help */
        OPT_DONE, /**< parsing success */
    };
    /**
     * constructor
     * @param[in] useDef use PMC default options
     */
    Options(bool useDef = true);
    /**
     * Use PMC default options
     */
    void useDefOption();
    /**
     * Insert option
     * @param[in] opt new option parameters
     * @return true on adding the option
     * @note should be called before calling parse_options()
     */
    bool insert(const Pmc_option &opt);
    /**
     * Get help message
     * @return help message
     */
    const char *get_help() const;
    /**
     * Get parse_options() message
     * @return message from last parse_options()
     */
    const std::string &get_msg() const;
    /**
     * Get parse_options() message
     * @return message from last parse_options()
     */
    const char *get_msg_c() const;
    /**
     * Parse command line
     * @param[in] argc number of arguments
     * @param[in] argv array of command line arguments
     * @return Parse state
     * @note Follow the command line.
     *       Argument 0 is the application name, @n
     *       as appear in the command line. @n
     *       The class do not store it.
     * @note Regarding using scripts: @n
     *  Python, Go and PHP uses full argument list. @n
     *  With Ruby, Perl, Tcl and Lua, users need to add argument 0.
     * To call from scripts: @n
     *  Python  @code{.py} obj.parse_options(sys.argv) @endcode @n
     *  Go      @code{.go} obj.Parse_options(os.Args) @endcode @n
     *  PHP     @code{.php} $obj->parse_options($argv) @endcode @n
     *  Ruby    @code{.rb} $obj.parse_options([$0] + ARGV) @endcode @n
     *  Perl    @code{.pl} obj->parse_options([$0, @@ARGV]); @endcode @n
     *  Tcl     @code{.tcl}
     *          obj parse_options [list {*}$argv0 {*}$@::argv] @endcode @n
     *  Lua     @code{.lua}
     *          table.insert(arg, 1, arg[0])
     *          obj:parse_options(arg) @endcode
     */
    loop_val parse_options(int argc, char *const argv[]);
    /**
     * Is option on command line
     * @param[in] opt short option character
     * @return true if option on command line
     */
    bool have(char opt) const;
    /**
     * get option value
     * @param[in] opt short option character
     * @return option value
     * @note relevant for option with argument
     */
    const std::string &val(char opt) const;
    /**
     * get option value
     * @param[in] opt short option character
     * @return option char pointer of value string (C style)
     * @note relevant for option with argument
     */
    const char *val_c(char opt) const;
    /**
     * get option integer value
     * @param[in] opt short option character
     * @return option integer value
     * @note relevant for option with argument of integer value
     */
    int val_i(char opt) const;
    /**
     * get Network Transport value
     * @return Network Transport
     * @note return 0 if not select on command line
     */
    char get_net_transport() const;
    /**
     * Do we have more argumends on the command line, left unprocessed
     * @return true if we have more to process
     */
    bool have_more() const;
    /**
     * First argumend on the command line, left unprocessed
     * @return index of argument
     */
    int process_next() const;
};

__PTPMGMT_NAMESPACE_END
#else /* __cplusplus */
#include "c/opt.h"
#endif /* __cplusplus */

#endif /* __PTPMGMT_OPT_H */
