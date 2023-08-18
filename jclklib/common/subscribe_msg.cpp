/*! \file subscribe_msg.cpp
    \brief Common subscribe message implementation. Implements common functions and (de-)serialization

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <common/subscribe_msg.hpp>

using namespace JClkLibCommon;
using namespace std;

MAKE_RXBUFFER_TYPE(SubscribeMessage::buildMessage)
{
	// Fill in fields

	return true;
}
