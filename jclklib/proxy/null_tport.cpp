/*! \file null_tport.cpp
    \brief Null transport implementation. Non-functional for testing only.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <proxy/null_tport.hpp>
#include <common/print.hpp>

#include <cstdint>

using namespace JClkLibProxy;
using namespace std;

SEND_PROXY_MESSAGE(NullProxyTransport::sendMessage)
{
	PrintError("NullTransport should not be used");
	return false;
}

