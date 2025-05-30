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

int main(int argc, char *argv[])
{
    int level, verbose, syslog;
    bool startSyslog = false;
    int opt;
    const char *file = nullptr;
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
                setLogLevel(static_cast<LogLevel>(level));
                break;
            case 'v':
                verbose = atoi(optarg);
                if(verbose < 0 || verbose > 1) {
                    fprintf(stderr, "Invalid verbose %d\n", verbose);
                    return EXIT_FAILURE;
                }
                setVerbose(verbose == 1);
                break;
            case 's':
                syslog = atoi(optarg);
                if(syslog < 0 || syslog > 1) {
                    fprintf(stderr, "Invalid syslog %d\n", syslog);
                    return EXIT_FAILURE;
                }
                startSyslog = (syslog == 1);
                break;
            case 'h':
                printf("Usage of %s:\n"
                    "Options:\n"
                    " -f [file] Read configuration from 'file'\n"
                    " -l <lvl> Set log level\n"
                    "          0: ERROR, 1: INFO(default), 2: DEBUG, 3: TRACE\n"
                    " -v <0|1> Enable or disable verbose output\n"
                    "          0: disable, 1: enable(default)\n"
                    " -s <0|1> Enable or disable system log printing\n"
                    "          0: disable(default), 1: enable\n"
                    " -h       Show this help message\n",
                    argv[0]);
                return EXIT_SUCCESS;
            default:
                fprintf(stderr, "Usage of %s:\n"
                    "Options:\n"
                    " -f [file] Read configuration from 'file'\n"
                    " -l <lvl> Set log level\n"
                    "          0: ERROR, 1: INFO(default), 2: DEBUG, 3: TRACE\n"
                    " -v <0|1> Enable or disable verbose output\n"
                    "          0: disable, 1: enable(default)\n"
                    " -s <0|1> Enable or disable system log printing\n"
                    "          0: disable(default), 1: enable\n"
                    " -h       Show this help message\n",
                    argv[0]);
                return EXIT_FAILURE;
        }
    }
    if(parser.size() == 0) {
        PrintError("Configuration file is missing or empty");
        return EXIT_FAILURE;
    }
    if(startSyslog)
        PrintStartLog(argv[0]);
    BlockStopSignal();
    if(!Client::init()) {
        PrintError("Proxy client init failed");
        return EXIT_FAILURE;
    }
    WaitForStopSignal();
    PrintDebug("Got stop signal");
    int ret = End::stopAll() ? EXIT_SUCCESS : EXIT_FAILURE;
    if(startSyslog)
        PrintStopLog();
    return ret;
}
