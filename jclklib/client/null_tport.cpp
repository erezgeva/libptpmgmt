/*! \file null_tport.cpp
    \brief Null transport implementation. Non-functional for testing only.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <client/null_tport.hpp>
#include <common/print.hpp>

#include <cstdint>

using namespace JClkLibClient;
using namespace std;

bool NullClientTransport::sendMessage(uint8_t *buffer, size_t length)
{
	PrintError("NullTransport should not be used");
	return false;
}

