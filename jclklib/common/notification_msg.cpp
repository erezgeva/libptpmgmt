/*! \file notification_msg.cpp
    \brief Common notification message implementation. Implements common functions and (de-)serialization

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <common/serialize.hpp>
#include <common/notification_msg.hpp>
#include <common/print.hpp>

using namespace JClkLibCommon;
using namespace std;

TRANSMIT_MESSAGE_TYPE(NotificationMessage::transmitMessage)
{
	PrintDebug("[NotificationMessage]::transmitMessage ");
	if (!presendMessage(&TxContext))
		return false;

	return TxContext.sendBuffer();
}
