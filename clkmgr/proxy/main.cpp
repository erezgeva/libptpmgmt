/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy application implementation
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "proxy/config_parser.hpp"
#include "proxy/client.hpp"
#include "common/termin.hpp"
#include "common/sighandler.hpp"
#include "common/print.hpp"

// Use version of libptpmgmt
#include "ver.h"

#include <getopt.h>

__CLKMGR_NAMESPACE_USE;

using namespace std;

static inline int help(FILE *out, const char *me, int ret)
{
    fprintf(out,
        "Usage of %s:\n"
        "Options:\n"
        " -f [file] Read configuration from 'file'\n"
        " -a <0|1> Open message queue in access all mode\n"
        "          0: disable(default), 1: enable\n"
        " -c <0|1> Enable or disable cleanup of residual message queues\n"
        "          0: disable, 1: enable(default)\n"
        " -l <lvl> Set log level\n"
        "          0: ERROR, 1: INFO(default), 2: DEBUG, 3: TRACE\n"
        " -q <0|1> Enable or disable quiet mode\n"
        "          0: disable(default), 1: enable\n"
        " -s <0|1> Enable or disable use of system logging\n"
        "          0: disable(default), 1: enable\n"
        " -v       Show version\n"
        " -h       Show help message\n",
        me);
    return ret;
}

int main(int argc, char *argv[])
{
    int level, opt;
    bool useSyslog = false; // Default value
    bool useVerbode = true; // Default value
    bool useMsgQAllAccess = false; // Default value
    bool useMsgQCleanup = true; // Default value
    const char *me = strrchr(argv[0], '/');
    // Remove leading path
    me = me == nullptr ? argv[0] : me + 1;
    JsonConfigParser &parser = JsonConfigParser::getInstance();
    while((opt = getopt(argc, argv, "a:c:f:l:q:s:vh")) != -1) {
        switch(opt) {
            case 'a':
                useMsgQAllAccess = atoi(optarg) != 0;
                break;
            case 'c':
                useMsgQCleanup = atoi(optarg) != 0;
                break;
            case 'f':
                if(optarg == nullptr || !parser.process_json(optarg)) {
                    fprintf(stderr, "Failed to process json file\n");
                    return EXIT_FAILURE;
                }
                break;
            case 'l':
                level = atoi(optarg);
                if(level < 0 || level > 3) {
                    fprintf(stderr, "Invalid log level %d\n", level);
                    return EXIT_FAILURE;
                }
                setLogLevel(level);
                break;
            case 'q':
                useVerbode = atoi(optarg) == 0;
                break;
            case 's':
                useSyslog = atoi(optarg) != 0;
                break;
            case 'v':
                // Print version of compile time!
                // The actual libptpmgmt library can be newer!
                printf("%s Version %s\n", me, LIBPTPMGMT_VER);
                return EXIT_SUCCESS;
            case 'h':
                return help(stdout, me, EXIT_SUCCESS);
            default:
                return help(stderr, me, EXIT_FAILURE);
        }
    }
    setVerbose(useVerbode);
    if(useSyslog)
        PrintStartLog(me);
    if(parser.size() == 0) {
        PrintError("Configuration file is missing or empty");
        return EXIT_FAILURE;
    }
    BlockStopSignal();
    if(!Client::init(useMsgQAllAccess, useMsgQCleanup)) {
        PrintError("Proxy client init failed");
        return EXIT_FAILURE;
    }
    WaitForStopSignal();
    PrintDebug("Got stop signal");
    Client::notifyDisconnect();
    int ret = End::stopAll() ? EXIT_SUCCESS : EXIT_FAILURE;
    if(useSyslog)
        PrintStopLog();
    return ret;
}
