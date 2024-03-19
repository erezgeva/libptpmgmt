/*! \file connect_msg.hpp
    \brief Proxy connect message class. Implements proxy specific connect message function.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#ifndef PROXY_CONNECT_MSG_HPP
#define PROXY_CONNECT_MSG_HPP

#include <proxy/message.hpp>
#include <common/connect_msg.hpp>

namespace JClkLibProxy
{
	class ProxyConnectMessage : virtual public ProxyMessage, virtual public JClkLibCommon::CommonConnectMessage
	{
	protected:
		ProxyConnectMessage() : MESSAGE_CONNECT() {};
	public:
		/**
		 * @brief process the connect msg from client-runtime
		 * @param LxContext proxy transport listener context
		 * @param TxContext proxy transport transmitter context
		 * @return true
		 */
		virtual PROCESS_MESSAGE_TYPE(processMessage);

		bool generateResponse(uint8_t *msgBuffer, std::size_t &length,
				      const ClockStatus &status)
		{ return false; }

		/**
		 * @brief Create the ProxyConnectMessage object
		 * @param msg msg structure to be fill up
		 * @param LxContext proxy transport listener context
		 * @return true
		 */
		static MAKE_RXBUFFER_TYPE(buildMessage);

		/** @brief Add proxy's CONNECT_MSG type and its builder to transport layer.
		 * @return true
		 */
		static bool initMessage();
	};
}

#endif/*PROXY_CONNECT_MSG_HPP*/
