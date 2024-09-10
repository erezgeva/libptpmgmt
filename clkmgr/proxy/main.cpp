/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file
 * @brief Proxy application implementation
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include <iostream>
#include <getopt.h>

#include <common/print.hpp>
#include <common/sighandler.hpp>
#include <proxy/connect_ptp4l.hpp>
#include <proxy/message.hpp>
#include <proxy/transport.hpp>

__CLKMGR_NAMESPACE_USE

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
                    std::cerr << "Invalid transport specific value: " << "\n";
                    return EXIT_FAILURE;
                }
                transport_specific = static_cast<uint8_t>(value);
                break;
            case 'h':
                std::cout << "Usage of " << argv[0] << " :\n"
                    "Options:\n"
                    " -t transport specific\n"
                    "    Default: 0x" << std::hex <<
                    static_cast<unsigned int>(transport_specific) << "\n";
                return EXIT_SUCCESS;
            default:
                std::cerr << "Usage of " << argv[0] << " :\n"
                    "Options:\n"
                    " -t transport specific\n"
                    "    Default: 0x" << std::hex <<
                    static_cast<unsigned int>(transport_specific) << "\n";
                return EXIT_FAILURE;
        }
    }
    BlockStopSignal();
    if(!ProxyTransport::init()) {
        cout << "Transport init failed" << endl;
        return -1;
    }
    if(!ProxyMessage::init()) {
        cout << "Message init failed" << endl;
        return -1;
    }
    Connect::connect(transport_specific);
    WaitForStopSignal();
    PrintDebug("Got stop signal");
    Connect::disconnect();
    if(!ProxyTransport::stop()) {
        cout << "stop failed" << endl;
        return -1;
    }
    if(!ProxyTransport::finalize()) {
        cout << "finalize failed" << endl;
        return -1;
    }
    return 0;
}
