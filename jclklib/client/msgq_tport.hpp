/*
 * SPDX-License-Identifier: LGPL-3.0-or-later
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. All rights reserved.
 */

/** @file msgq_tport.hpp
 * @brief Client POSIX message queue transport class.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation. All rights reserved.
 * @license LGPL-3.0-or-later
 *
 */

#include <cstdint>
#include <string>
#include <future>
#include <memory>

#include <mqueue.h>

#ifndef CLIENT_MSGQ_TPORT_HPP
#define CLIENT_MSGQ_TPORT_HPP

#include <client/transport.hpp>
#include <common/connect_msg.hpp>
#include <common/msgq_tport.hpp>
#include <common/util.hpp>

namespace JClkLibClient
{
        class ClientMessageQueueListenerContext : public JClkLibCommon::MessageQueueListenerContext
	{
		friend class ClientMessageQueue;
        protected:
		virtual LISTENER_CONTEXT_PROCESS_MESSAGE_TYPE(processMessage);
		ClientMessageQueueListenerContext(mqd_t mqListenerDesc) : MessageQueueListenerContext(mqListenerDesc) {}
	};

        class ClientMessageQueueTransmitterContext  : public JClkLibCommon::MessageQueueTransmitterContext
	{
		friend class ClientMessageQueue;
        protected:
		//virtual bool processMessage(JClkLibCommon::Message *msg);
		ClientMessageQueueTransmitterContext(mqd_t mqListenerDesc) : MessageQueueTransmitterContext(mqListenerDesc) {}
	};

	class ClientMessageQueue : public JClkLibCommon::MessageQueue, public ClientTransport
	{
	private:
		static mqd_t mqNativeClientTransmitterDesc;
		static std::string mqListenerName;
		static std::unique_ptr<JClkLibCommon::MessageQueueTransmitterContext> txContext;
	public:
		static bool initTransport();
		static bool stopTransport();
		static bool finalizeTransport();	
		static bool writeTransportClientId(JClkLibCommon::Message *msg);
		static SEND_CLIENT_MESSAGE(sendMessage);
	};
}

#endif/*CLIENT_MSGQ_TPORT_HPP*/
