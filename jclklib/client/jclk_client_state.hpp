/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file jclk_client_state.hpp
 * @brief Set and get the client subscribe event state
 *
 * @author Noor Azura Ahmad Tarmizi <noor.azura.ahmad.tarmizi@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
 */

#ifndef PROXY_CLIENT_STATE
#define PROXY_CLIENT_STATE

#include <atomic>
#include <string>

#include <common/jclklib_import.hpp>
#include <common/jcltypes.hpp>
#include <common/transport.hpp>
#include <common/util.hpp>

namespace JClkLibClient {
    struct jcl_state {
        uint8_t  gm_identity[8];
        bool     as_capable;
        bool     offset_in_range;
        bool     servo_locked;
        bool     gm_changed;
        bool     composite_event;
    };

    struct jcl_state_event_count {
        uint64_t offset_in_range_event_count;
        uint64_t gm_changed_event_count;
        uint64_t as_capable_event_count;
        uint64_t servo_locked_event_count;
        uint64_t composite_event_count;
    };

    class ClientState {
    private:
        std::atomic_bool connected = false;
        std::atomic_bool subscribed = false;
        JClkLibCommon::sessionId_t sessionId = JClkLibCommon::InvalidSessionId;
        JClkLibCommon::TransportClientId clientID = {};
        uint8_t ptp4l_id = 0;
        jcl_state eventState = {};
        jcl_state_event_count eventStateCount ={};
        JClkLibCommon::jcl_subscription eventSub ={};

    public:
        ClientState();
        ClientState (ClientState &newState);
        void set_clientState(ClientState &newState);
        bool get_connected();
        void set_connected(bool state);
        bool get_subscribed();
        void set_subscribed(bool subscriptionState);
        JClkLibCommon::TransportClientId get_clientID();
        void set_clientID(JClkLibCommon::TransportClientId &cID);
        jcl_state_event_count &get_eventStateCount();
        jcl_state &get_eventState();
        void set_eventStateCount(jcl_state_event_count eCount);
        void set_eventState(jcl_state eState);
        std::string toString();
        JClkLibCommon::jcl_subscription &get_eventSub();
        DECLARE_ACCESSOR(sessionId);
        DECLARE_ACCESSOR(ptp4l_id);
    };
}
#endif /* PROXY_CLIENT_STATE */
