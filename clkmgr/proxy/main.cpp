/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy application implementation
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "common/termin.hpp"

#ifdef HAVE_LIBCHRONY
#include "proxy/connect_chrony.hpp"
#endif
#include "proxy/config_parser.hpp"
#include "proxy/connect_ptp4l.hpp"
#include "proxy/msgq_tport.hpp"
#include "proxy/message.hpp"
#include "common/sighandler.hpp"
#include "common/print.hpp"

#include <getopt.h>

__CLKMGR_NAMESPACE_USE;

using namespace std;

int main(int argc, char *argv[])
{
    int level, verbose, syslog;
    bool startSyslog = false;
    bool getJsonConfig = false;
    int opt;
    const char *file = nullptr;
    while((opt = getopt(argc, argv, "f:l:v:s:h")) != -1) {
        switch(opt) {
            case 'f':
                file = optarg;
                if(file == nullptr ||
                    !JsonConfigParser::getInstance().process_json(file)) {
                    fprintf(stderr, "Failed to process json file\n");
                    return EXIT_FAILURE;
                }
                getJsonConfig = true;
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
    if(!getJsonConfig) {
        PrintError("No configuration file provided");
        return EXIT_FAILURE;
    }
    if(startSyslog)
        PrintStartLog(argv[0]);
    BlockStopSignal();
    if(!proxyQueueInit()) {
        PrintError("Proxy queue init failed");
        return EXIT_FAILURE;
    }
    if(!ProxyMessage::Register()) {
        PrintError("Message init failed");
        return EXIT_FAILURE;
    }
    ConnectPtp4l::connect_ptp4l();
    #ifdef HAVE_LIBCHRONY
    ConnectChrony::connect_chrony();
    #endif
    WaitForStopSignal();
    PrintDebug("Got stop signal");
    int ret = End::stopAll() ? EXIT_SUCCESS : EXIT_FAILURE;
    if(startSyslog)
        PrintStopLog();
    return ret;
}
