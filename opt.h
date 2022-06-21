/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/** @file
 * @brief handle command line options
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#ifndef __PTPMGMT_OPT_H
#define __PTPMGMT_OPT_H

#include <string>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <getopt.h>
#include <vector>
#include <map>

#ifndef SWIG
namespace ptpmgmt
{
#endif

/**
 * @brief structre to add new option
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
    std::string def_val; /**< Defualt value for help */
};

/**
 * @brief hold and proccess options
 */
class Options
{
  private:
    static Pmc_option startOptions[];
    size_t max_arg_name;
    std::vector<option> long_options_list;
    /* We need to store the string used in option */
    std::vector<std::string> long_options_list_string;
    std::map<int, std::string> options;
    std::string net_options;
    std::string all_options;
    std::string all_short_options;
    std::string with_options;
    std::string msg;
    char net_select;
    int m_argc;
    int m_end_optind;
    bool m_useDef;

    class helpStore
    {
      private:
        std::string start;
        std::string end;
      public:
        helpStore(const char *s, const char *e = nullptr): start(s) {
            if(e != nullptr)
                end = e;
        }
        helpStore &addStart(const std::string &s) {start += s; return *this;}
        helpStore &addStart(const char s) {start += s; return *this;}
        helpStore &addEnd(const std::string &e) {end += e; return *this;}
        std::string get(size_t length) const;
    };
    std::string help;
    std::vector<helpStore> helpVec;
    bool helpUpdate;

  public:
    /** parsing return code */
    enum loop_val {
        OPT_ERR, /**< Parsing error */
        OPT_MSG, /**< Message from parsing */
        OPT_HELP, /**< Need to print help */
        OPT_DONE, /**< parsing success */
    };
    /**
     * constructor
     * @param[in] useDef use PMC default options
     */
    Options(bool useDef = true);
    /**
     * Use PMC defult options
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
    const char *get_help();
    /**
     * Get parse_options() message
     * @return message from last parse_options()
     */
    const std::string &get_msg() const { return msg; }
    /**
     * Get parse_options() message
     * @return message from last parse_options()
     */
    const char *get_msg_c() const { return msg.c_str(); }
    /**
     * Parse command line
     * @param[in] argc number of arguments
     * @param[in] argv array of command line arguments
     * @return Parse state
     * @note This class do not store argument 0,
     *       i.e application name from command line!
     * @note regarding using scripts: @n
     *  Python and PHP uses full argument list.
     *  For Ruby, Perl and Tcl user need to add argument 0.
     *  Lua miss argument 0 completly, so user should add name manually! @n
     * To call from scripts: @n
     *  Python  obj.parse_options(sys.argv) @n
     *  PHP     $obj->parse_options($argv) @n
     *  Tcl     obj parse_options [list {*}$argv0 {*}$@::argv] @n
     *  Perl    obj->parse_options([$0, @@ARGV]); @n
     *  Ruby    $obj.parse_options([$0] + ARGV) @n
     *  Lua     table.insert(arg, 1, "myname"); obj:parse_options(arg)
     */
    loop_val parse_options(int argc, char *const argv[]);
    /**
     * Is option on command line
     * @param[in] opt short option character
     * @return true if option on command line
     */
    bool have(char opt) const { return options.count(opt) > 0; }
    /**
     * get option value
     * @param[in] opt short option character
     * @return option value
     * @note relevant for option with argument
     */
    const std::string val(char opt) const
    { return have(opt) ? options.at(opt) : ""; }
    /**
     * get option value
     * @param[in] opt short option character
     * @return option char pointer of value string (C style)
     * @note relevant for option with argument
     */
    const char *val_c(char opt) const
    { return have(opt) ? options.at(opt).c_str() : ""; }
    /**
     * get option integer value
     * @param[in] opt short option character
     * @return option integer value
     * @note relevant for option with argument of integer value
     */
    int val_i(char opt) const
    { return have(opt) ? atoi(options.at(opt).c_str()) : 0; }
    /**
     * get Network Transport value
     * @return Network Transport
     * @note return 0 if not select on command line
     */
    char get_net_transport() const { return net_select; }
    /**
     * Do we have more argumends on the command line we did not proccess
     * @return true if we have more to proccess
     */
    bool have_more() const { return m_end_optind < m_argc; }
    /**
     * First argumend on the command line that we did not proccess
     * @return index of argument
     */
    int procces_next() const { return m_end_optind; }
};

#ifndef SWIG
}; /* namespace ptpmgmt */
#endif

#endif /* __PTPMGMT_OPT_H */
