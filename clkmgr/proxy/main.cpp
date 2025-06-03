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

#include <getopt.h>

__CLKMGR_NAMESPACE_USE;

using namespace std;

static inline int help(FILE *out, const char *me, int ret)
{
    fprintf(out,
        "Usage of %s:\n"
        "Options:\n"
        " -f [file] Read configuration from 'file'\n"
        " -l <lvl> Set log level\n"
        "          0: ERROR, 1: INFO(default), 2: DEBUG, 3: TRACE\n"
        " -v <0|1> Enable or disable verbose output\n"
        "          0: disable, 1: enable(default)\n"
        " -s <0|1> Enable or disable system log printing\n"
        "          0: disable(default), 1: enable\n"
        " -h       Show this help message\n",
        me);
    return ret;
}

int main(int argc, char *argv[])
{
    int level, opt;
    bool useSyslog = false; // Default value
    bool useVerbode = true; // Default value
    const char *file;
    const char *me = argv[0];
    JsonConfigParser &parser = JsonConfigParser::getInstance();
    while((opt = getopt(argc, argv, "f:l:v:s:h")) != -1) {
        switch(opt) {
            case 'f':
                file = optarg;
                if(file == nullptr || !parser.process_json(file)) {
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
            case 'v':
                useVerbode = atoi(optarg) != 0;
                break;
            case 's':
                useSyslog = atoi(optarg) != 0;
                break;
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
    if(!Client::init()) {
        PrintError("Proxy client init failed");
        return EXIT_FAILURE;
    }
    WaitForStopSignal();
    PrintDebug("Got stop signal");
    int ret = End::stopAll() ? EXIT_SUCCESS : EXIT_FAILURE;
    if(useSyslog)
        PrintStopLog();
    return ret;
}
