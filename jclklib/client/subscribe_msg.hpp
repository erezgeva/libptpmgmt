/*! \file subscribe_msg.hpp
    \brief Client subscribe message class. Implements client specific functionality.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    @author: Noor Azura Ahmad Tarmizi <noor.azura.ahmad.tarmizi@intel.com>
*/

#ifndef CLIENT_SUBSCRIBE_MSG_HPP
#define CLIENT_SUBSCRIBE_MSG_HPP

#include <common/subscribe_msg.hpp>
#include <client/message.hpp>

namespace JClkLibClient
{
	class ClientSubscribeMessage : virtual public JClkLibCommon::CommonSubscribeMessage,
				     virtual public ClientMessage
	{
	public:
		ClientSubscribeMessage() : MESSAGE_SUBSCRIBE() {};
		/**
		 * @brief process the reply for subscribe msg from proxy.
		 * @param LxContext client run-time transport listener context
		 * @param TxContext client run-time transport transmitter context
		 * @return true
		 */
		virtual PROCESS_MESSAGE_TYPE(processMessage);

		/**
		 * @brief Create the ClientSubscribeMessage object
		 * @param msg msg structure to be fill up
		 * @param LxContext client run-time transport listener context
		 * @return true
		 */
		static MAKE_RXBUFFER_TYPE(buildMessage);

		/**
		 * @brief Add client's SUBSCRIBE_MSG type and its builder to transport layer.
		 * @return true
		 */
		static bool initMessage();

		virtual PARSE_RXBUFFER_TYPE(parseBuffer);
	};
}

#endif/*CLIENT_SUBSCRIBE_MSG_HPP*/
