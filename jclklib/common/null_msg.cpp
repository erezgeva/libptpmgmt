/*! \file null_msg.cpp
    \brief Non-functional message implementation for debug

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <common/null_msg.hpp>

using namespace JClkLibCommon;
using namespace std;

MAKE_RXBUFFER_TYPE(CommonNullMessage::buildMessage)
{
	return true;
}

bool CommonNullMessage::initMessage()
{
	return true;
}
