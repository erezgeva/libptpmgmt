/*! \file message.cpp
    \brief Proxy message base implementation. Extended for specific messages such as connect and subscribe.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <proxy/null_msg.hpp>
#include <proxy/connect_msg.hpp>
#include <proxy/subscribe_msg.hpp>
#include <proxy/notification_msg.hpp>

using namespace JClkLibProxy;
using namespace JClkLibCommon;

bool ProxyMessage::init()
{
	return _initMessage<ProxyNullMessage, ProxyConnectMessage, ProxySubscribeMessage, ProxyNotificationMessage>();
}
		


