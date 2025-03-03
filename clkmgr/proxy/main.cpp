/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Proxy application implementation
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include <cstdio>
#include <getopt.h>

#include "common/print.hpp"
#include "common/sighandler.hpp"
#include "proxy/connect_ptp4l.hpp"
#ifdef HAVE_LIBCHRONY
#include "proxy/connect_chrony.hpp"
#endif
#include "proxy/message.hpp"
#include "proxy/transport.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

int main(int argc, char *argv[])
{
    int level, verbose, syslog;
    bool startSyslog = true;
    int opt;
    while((opt = getopt(argc, argv, "l:v:s:h")) != -1) {
        switch(opt) {
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
                    " -l <lvl> Set log level\n"
                    "          0: ERROR, 1: INFO(default), 2: DEBUG, 3: TRACE\n"
                    " -v <0|1> Enable or disable verbose output\n"
                    "          0: disable, 1: enable(default)\n"
                    " -s <0|1> Enable or disable system log printing\n"
                    "          0: disable, 1: enable(default)\n"
                    " -h       Show this help message\n",
                    argv[0]);
                return EXIT_SUCCESS;
            default:
                fprintf(stderr, "Usage of %s:\n"
                    "Options:\n"
                    " -l <lvl> Set log level\n"
                    "          0: ERROR, 1: INFO(default), 2: DEBUG, 3: TRACE\n"
                    " -v <0|1> Enable or disable verbose output\n"
                    "          0: disable, 1: enable(default)\n"
                    " -s <0|1> Enable or disable system log printing\n"
                    "          0: disable, 1: enable(default)\n"
                    " -h       Show this help message\n",
                    argv[0]);
                return EXIT_FAILURE;
        }
    }
    if(startSyslog)
        PrintStartLog(argv[0]);
    BlockStopSignal();
    if(!ProxyTransport::init()) {
        PrintError("Transport init failed");
        return EXIT_FAILURE;
    }
    if(!ProxyMessage::init()) {
        PrintError("Message init failed");
        return EXIT_FAILURE;
    }
    std::vector<TimeBaseCfg> timeBaseCfgs;
    ConnectPtp4l::connect_ptp4l(timeBaseCfgs);
    #ifdef HAVE_LIBCHRONY
    ConnectChrony::connect_chrony(timeBaseCfgs);
    #endif
    WaitForStopSignal();
    PrintDebug("Got stop signal");
    ConnectPtp4l::disconnect_ptp4l();
    if(!ProxyTransport::stop()) {
        PrintError("stop failed");
        return EXIT_FAILURE;
    }
    if(!ProxyTransport::finalize()) {
        PrintError("finalize failed");
        return EXIT_FAILURE;
    }
    if(startSyslog)
        PrintStopLog();
    return EXIT_SUCCESS;
}
