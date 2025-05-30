/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Client connect message class. Implements client specific functionality.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "pub/clkmgr/timebase_configs.h"
#include "client/connect_msg.hpp"
#include "client/client_state.hpp"
#include "common/timebase.hpp"
#include "common/serialize.hpp"
#include "common/print.hpp"

__CLKMGR_NAMESPACE_USE;

using namespace std;

/**
 * @brief process the reply for connect msg from proxy.
 *
 * This function will be called when the transport layer
 * in client runtime received a CONNECT_MSG type (an echo reply from
 * proxy)
 * In this case, transport layer will rx a buffer in the client associated
 * listening message queue (listening to proxy) and call this function when
 * the enum ID corresponding to the CONNECT_MSG is received.
 * For client-runtime, this will always be the reply, since connect msg
 * is always send first from the client runtime first. The proxy will
 * echo-reply with a different ACK msg.
 *
 * @return true
 */
bool ClientConnectMessage::parseBufferTail()
{
    PrintDebug("[ClientConnectMessage]::parseBufferTail");
    size_t mapSize = 0;
    if(!PARSE_RX(FIELD, mapSize, rxBuf))
        return false;
    for(size_t i = 0; i < mapSize; ++i) {
        TimeBaseCfg newCfg = {};
        if(!PARSE_RX(FIELD, newCfg, rxBuf))
            return false;
        TimeBaseConfigurations::addTimeBaseCfg(newCfg);
    }
    if(!ClientState::connectReply(get_sessionId()))
        return false;
    set_msgAck(ACK_NONE);
    return true;
}
