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
#include "proxy/message.hpp"
#include "proxy/transport.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

int main(int argc, char *argv[])
{
    uint8_t transport_specific = 0;
    int value = 0;
    int opt;
    while((opt = getopt(argc, argv, "t:h")) != -1) {
        switch(opt) {
            case 't':
                value = std::stoi(optarg);
                if(value < 0 || value > 255) {
                    fprintf(stderr, "Invalid transport specific value: %s\n",
                        optarg);
                    return EXIT_FAILURE;
                }
                transport_specific = static_cast<uint8_t>(value);
                break;
            case 'h':
                printf("Usage of %s:\n"
                    "Options:\n"
                    " -t transport specific\n"
                    "    Default: 0x%x\n",
                    argv[0], transport_specific);
                return EXIT_SUCCESS;
            default:
                fprintf(stderr, "Usage of %s:\n"
                    "Options:\n"
                    " -t transport specific\n"
                    "    Default: 0x%x\n",
                    argv[0],
                    transport_specific);
                return EXIT_FAILURE;
        }
    }
    BlockStopSignal();
    if(!ProxyTransport::init()) {
        printf("Transport init failed\n");
        return -1;
    }
    if(!ProxyMessage::init()) {
        printf("Message init failed\n");
        return -1;
    }
    Connect::connect(transport_specific);
    WaitForStopSignal();
    PrintDebug("Got stop signal");
    Connect::disconnect();
    if(!ProxyTransport::stop()) {
        printf("stop failed\n");
        return -1;
    }
    if(!ProxyTransport::finalize()) {
        printf("finalize failed\n");
        return -1;
    }
    return 0;
}
