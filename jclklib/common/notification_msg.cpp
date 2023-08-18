/*! \file notification_msg.cpp
    \brief Common notification message implementation. Implements common functions and (de-)serialization

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <common/serialize.hpp>
#include <common/notification_msg.hpp>

using namespace JClkLibCommon;
using namespace std;

MAKE_RXBUFFER_TYPE(NotificationMessage::buildMessage)
{
	// Fill in the fields
	
	return true;
}
