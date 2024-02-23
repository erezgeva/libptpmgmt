/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief options class unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 *
 */

#include "opt.h"
#include "ver.h" // Class opt return the library version

char help[] =
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

char *argv_ver[] = {"me", "-v"};
const size_t argc_ver = 2;
char *argv_help[] = {"me", "-h"};
const size_t argc_help = 2;
char *argv[] = {"me", "-4", "-d", "54", "-f", "dummy"};
const size_t argc = 6;
char *argv_more[] = {"me", "-4", "-d", "54", "-f", "dummy", "\"get XX\""};
const size_t argc_more = 7;

// Tests default options
// ptpmgmt_opt ptpmgmt_opt_alloc()
// void free(ptpmgmt_opt _this)
Test(OptionsTest, MethodEmptyConstructor)
{
    ptpmgmt_opt o = ptpmgmt_opt_alloc();
    cr_expect(eq(str, (char *)o->get_help(o), help));
    o->free(o);
}

// Tests empty options
// ptpmgmt_opt ptpmgmt_opt_alloc_empty()
Test(OptionsTest, MethodConstructor)
{
    ptpmgmt_opt o = ptpmgmt_opt_alloc_empty();
    cr_expect(eq(str, (char *)o->get_help(o), ""));
    o->free(o);
}

// Tests use defualt
// void useDefOption(ptpmgmt_opt _this)
Test(OptionsTest, MethodUseDefOption)
{
    ptpmgmt_opt o = ptpmgmt_opt_alloc_empty();
    o->useDefOption(o);
    cr_expect(eq(str, (char *)o->get_help(o), help));
    o->free(o);
}

// Tests insert option
// int insert(ptpmgmt_opt _this, struct ptpmgmt_opt_option *opt)
Test(OptionsTest, MethodInsert)
{
    struct ptpmgmt_opt_option n = {
        .short_name = 'p',
        .long_name = "dummy",
        .have_arg = true,
        .long_only = false,
        .help_msg = "test message",
        .arg_help = "number",
        .def_val = "1",
    };
    ptpmgmt_opt o = ptpmgmt_opt_alloc();
    cr_expect(o->insert(o, &n));
    cr_expect(eq(str, (char *)o->get_help(o),
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
            " -p [number] test message, default 1\n"));
    o->free(o);
}

// Tests get help text
// const char *get_help(ptpmgmt_opt _this)
Test(OptionsTest, MethodGetHelp)
{
    ptpmgmt_opt o = ptpmgmt_opt_alloc();
    cr_expect(eq(str, (char *)o->get_help(o), help));
    o->free(o);
}

// Tests get message text
// const char *get_msg(ptpmgmt_opt _this)
Test(OptionsTest, MethodGetMsg)
{
    ptpmgmt_opt o = ptpmgmt_opt_alloc();
    cr_expect(eq(i32, o->parse_options(o, argc_ver, argv_ver), PTPMGMT_OPT_MSG));
    cr_expect(eq(str, (char *)o->get_msg(o), LIBPTPMGMT_VER));
    o->free(o);
}

// Tests parse options
// enum ptpmgmt_opt_loop_val parse_options(ptpmgmt_opt _this, int argc,
//     char *argv[])
Test(OptionsTest, MethodParseOptions)
{
    ptpmgmt_opt o = ptpmgmt_opt_alloc();
    cr_expect(eq(i32, o->parse_options(o, argc_help, argv_help), PTPMGMT_OPT_HELP));
    cr_expect(eq(str, (char *)o->get_help(o), help));
    o->free(o);
}

// Tests query if option found
// int have(ptpmgmt_opt _this, char opt)
Test(OptionsTest, MethodHave)
{
    ptpmgmt_opt o = ptpmgmt_opt_alloc();
    cr_expect(eq(i32, o->parse_options(o, argc, argv), PTPMGMT_OPT_DONE));
    cr_expect(o->have(o, 'f'));
    cr_expect(not(o->have(o, 'b')));
    o->free(o);
}

// Tests retrieve value of option
// const char *val(ptpmgmt_opt _this, char opt)
Test(OptionsTest, MethodVal)
{
    ptpmgmt_opt o = ptpmgmt_opt_alloc();
    cr_expect(eq(i32, o->parse_options(o, argc, argv), PTPMGMT_OPT_DONE));
    cr_expect(eq(str, (char *)o->val(o, 'f'), "dummy"));
    o->free(o);
}

// Tests retrieve integer value of option
// int val_i(ptpmgmt_opt _this, char opt)
Test(OptionsTest, MethodIntVal)
{
    ptpmgmt_opt o = ptpmgmt_opt_alloc();
    cr_expect(eq(i32, o->parse_options(o, argc, argv), PTPMGMT_OPT_DONE));
    cr_expect(eq(i32, o->val_i(o, 'd'), 54));
    o->free(o);
}

// Tests get network transport
// char get_net_transport(ptpmgmt_opt _this)
Test(OptionsTest, MethodNet)
{
    ptpmgmt_opt o = ptpmgmt_opt_alloc();
    cr_expect(eq(i32, o->parse_options(o, argc, argv), PTPMGMT_OPT_DONE));
    cr_expect(eq(i32, o->get_net_transport(o), '4'));
    o->free(o);
}

// Tests if consume all commandline
// int have_more(ptpmgmt_opt _this)
Test(OptionsTest, MethodMore)
{
    ptpmgmt_opt o = ptpmgmt_opt_alloc();
    cr_expect(eq(i32, o->parse_options(o, argc, argv), PTPMGMT_OPT_DONE));
    cr_expect(not(o->have_more(o)));
    cr_expect(eq(i32, o->parse_options(o, argc_more, argv_more), PTPMGMT_OPT_DONE));
    cr_expect(o->have_more(o));
    o->free(o);
}

// Tests get first argument not consumed
// int process_next(ptpmgmt_opt _this)
Test(OptionsTest, MethodNext)
{
    ptpmgmt_opt o = ptpmgmt_opt_alloc();
    cr_expect(eq(i32, o->parse_options(o, argc, argv), PTPMGMT_OPT_DONE));
    cr_expect(not(o->have_more(o)));
    cr_expect(eq(i32, o->process_next(o), 6));
    cr_expect(eq(i32, o->parse_options(o, argc_more, argv_more), PTPMGMT_OPT_DONE));
    cr_expect(o->have_more(o));
    cr_expect(eq(i32, o->process_next(o), 6));
    o->free(o);
}
