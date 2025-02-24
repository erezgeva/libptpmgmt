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
    int opt;
    while((opt = getopt(argc, argv, "t:h")) != -1) {
        switch(opt) {
            case 'h':
                printf("Usage of :\n"
                    "Options:\n");
                return EXIT_SUCCESS;
            default:
                fprintf(stderr, "Usage of:\n"
                    "Options:\n");
                return EXIT_FAILURE;
        }
    }
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
    PrintStopLog();
    return EXIT_SUCCESS;
}
