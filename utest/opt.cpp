/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2022 Erez Geva */

/** @file
 * @brief options class unit test
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2022 Erez Geva
 *
 */

#include "opt.h"
#include "ver.h"

using namespace ptpmgmt;

const char help[] =
    " Network Transport\n"
    "\n"
    " -2        IEEE 802.3\n"
    " -4        UDP IPV4 (default)\n"
    " -6        UDP IPV6\n"
    " -u        UDS local\n"
    "\n"
    " Other Options\n"
    "\n"
    " -b [num]  boundary hops, default 1\n"
    " -d [num]  domain number, default 0\n"
    " -f [file] read configuration from 'file'\n"
    " -h        prints this message and exits\n"
    " -i [dev]  interface device to use\n"
    " -s [path] server address for UDS, default '/var/run/ptp4l'\n"
    " -t [hex]  transport specific field, default 0x0\n"
    " -v        prints the software version and exits\n"
    " -z        send zero length TLV values with the GET actions\n";

const char *argv_ver[] = {"me", "-v"};
const size_t argc_ver = 2;
const char *argv_help[] = {"me", "-h"};
const size_t argc_help = 2;
const char *argv[] = {"me", "-4", "-d", "54", "-f", "dummy"};
const size_t argc = 6;
const char *argv_more[] = {"me", "-4", "-d", "54", "-f", "dummy", "\"get XX\""};
const size_t argc_more = 7;

// Tests Options empty constructor
// Options(bool useDef = true)
TEST(OptionsTest, MethodEmptyConstructor)
{
    Options o;
    EXPECT_STREQ(o.get_help(), help);
}

// Tests Options constructor
// Options(bool useDef)
TEST(OptionsTest, MethodConstructor)
{
    Options o(false);
    EXPECT_STREQ(o.get_help(), "");
}

// Tests use defualt
// void useDefOption()
TEST(OptionsTest, MethodUseDefOption)
{
    Options o(false);
    o.useDefOption();
    EXPECT_STREQ(o.get_help(), help);
}

// Tests insert option
// bool insert(const Pmc_option &opt)
TEST(OptionsTest, MethodInsert)
{
    Pmc_option n = {
        .short_name = 'p',
        .long_name = "dummy",
        .have_arg = true,
        .long_only = false,
        .help_msg = "test message",
        .arg_help = "number",
        .def_val = "1",
    };
    Options o;
    EXPECT_TRUE(o.insert(n));
    EXPECT_STREQ(o.get_help(),
        " Network Transport\n"
        "\n"
        " -2          IEEE 802.3\n"
        " -4          UDP IPV4 (default)\n"
        " -6          UDP IPV6\n"
        " -u          UDS local\n"
        "\n"
        " Other Options\n"
        "\n"
        " -b [num]    boundary hops, default 1\n"
        " -d [num]    domain number, default 0\n"
        " -f [file]   read configuration from 'file'\n"
        " -h          prints this message and exits\n"
        " -i [dev]    interface device to use\n"
        " -s [path]   server address for UDS, default '/var/run/ptp4l'\n"
        " -t [hex]    transport specific field, default 0x0\n"
        " -v          prints the software version and exits\n"
        " -z          send zero length TLV values with the GET actions\n"
        " -p [number] test message, default 1\n");
}

// Tests get help text
// const char *get_help();
TEST(OptionsTest, MethodGetHelp)
{
    Options o;
    EXPECT_STREQ(o.get_help(), help);
}

// Tests get message text
// const std::string &get_msg() const
TEST(OptionsTest, MethodGetMsg)
{
    Options o;
    EXPECT_EQ(o.parse_options(argc_ver, (char *const *)argv_ver), Options::OPT_MSG);
    EXPECT_STREQ(o.get_msg().c_str(), LIBPTPMGMT_VER);
}

// Tests get message text C string
// const char *get_msg_c() const
TEST(OptionsTest, MethodGetMsgC)
{
    Options o;
    EXPECT_EQ(o.parse_options(argc_ver, (char *const *)argv_ver), Options::OPT_MSG);
    EXPECT_STREQ(o.get_msg_c(), LIBPTPMGMT_VER);
}

// Tests parse options
// loop_val parse_options(int argc, char *const argv[]);
TEST(OptionsTest, MethodParseOptions)
{
    Options o;
    EXPECT_EQ(o.parse_options(argc_help, (char *const *)argv_help),
        Options::OPT_HELP);
    EXPECT_STREQ(o.get_help(), help);
}

// Tests query if option found
// bool have(char opt) const
TEST(OptionsTest, MethodHave)
{
    Options o;
    EXPECT_EQ(o.parse_options(argc, (char *const *)argv), Options::OPT_DONE);
    EXPECT_TRUE(o.have('f'));
    EXPECT_FALSE(o.have('b'));
}

// Tests retrieve value of option
// const std::string val(char opt) const
TEST(OptionsTest, MethodVal)
{
    Options o;
    EXPECT_EQ(o.parse_options(argc, (char *const *)argv), Options::OPT_DONE);
    EXPECT_STREQ(o.val('f').c_str(), "dummy");
}

// Tests retrieve value of option C string
// const char *val_c(char opt) const
TEST(OptionsTest, MethodValC)
{
    Options o;
    EXPECT_EQ(o.parse_options(argc, (char *const *)argv), Options::OPT_DONE);
    EXPECT_STREQ(o.val_c('f'), "dummy");
}

// Tests retrieve integer value of option
// int val_i(char opt) const
TEST(OptionsTest, MethodIntVal)
{
    Options o;
    EXPECT_EQ(o.parse_options(argc, (char *const *)argv), Options::OPT_DONE);
    EXPECT_EQ(o.val_i('d'), 54);
}

// Tests get network transport
// char get_net_transport() const
TEST(OptionsTest, MethodNet)
{
    Options o;
    EXPECT_EQ(o.parse_options(argc, (char *const *)argv), Options::OPT_DONE);
    EXPECT_EQ(o.get_net_transport(), '4');
}

// Tests if consume all commandline
// bool have_more() const
TEST(OptionsTest, MethodMore)
{
    Options o;
    EXPECT_EQ(o.parse_options(argc, (char *const *)argv), Options::OPT_DONE);
    EXPECT_FALSE(o.have_more());
    EXPECT_EQ(o.parse_options(argc_more, (char *const *)argv_more),
        Options::OPT_DONE);
    EXPECT_TRUE(o.have_more());
}

// Tests get first argument not consumed
// int procces_next() const
TEST(OptionsTest, MethodNext)
{
    Options o;
    EXPECT_EQ(o.parse_options(argc, (char *const *)argv), Options::OPT_DONE);
    EXPECT_FALSE(o.have_more());
    EXPECT_EQ(o.procces_next(), 6);
    EXPECT_EQ(o.parse_options(argc_more, (char *const *)argv_more),
        Options::OPT_DONE);
    EXPECT_TRUE(o.have_more());
    EXPECT_EQ(o.procces_next(), 6);
}
