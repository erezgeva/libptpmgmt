/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file init.hpp
 * @brief Client utilities to setup and cleanup the library.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
 */

#ifndef CLIENT_INIT_HPP
#define CLIENT_INIT_HPP

#include <common/jclklib_import.hpp>
#include <client/client_state.hpp>
#include <map>


namespace JClkLibClient {

class JClkLibClientApi {
    private :
        //std::map<PortIdentity_t, port_info> ports;
        static std::map <JClkLibCommon::sessionId_t, ClientState> clientStateMap;
        ClientState appClientState;
    public :
        JClkLibClientApi() {
        }
    static int init(){
            // In case we need to init something for API.
            return 0;
            };
    ClientState &getClientState() { return appClientState;};
	bool jcl_connect();
	bool jcl_disconnect();
	bool jcl_subscribe(JClkLibCommon::jcl_subscription &newSub,
                       JClkLibCommon::jcl_state &currentState);
    int jcl_status_wait(int timeout, JClkLibCommon::jcl_state &jcl_state,
                        JClkLibCommon::jcl_state_event_count &eventCount);

    };
}

#endif/*CLIENT_INIT_HPP*/
