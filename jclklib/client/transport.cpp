/*! \file transport.cpp
    \brief Client transport base implementation for JClkLib. It is extended for specific transports such as POSIX message queue.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <client/transport.hpp>
#include <client/null_tport.hpp>
#include <client/msgq_tport.hpp>
#include <common/print.hpp>
#include <common/sighandler.hpp>
#include <common/util.hpp>

using namespace JClkLibClient;
using namespace JClkLibCommon;
using namespace std;

bool ClientTransport::init()
{
	if (!Transport::init())
		return false;
	PrintDebug("Finished common init");
	
	return JClkLibCommon::_initTransport<ClientMessageQueue>();
}

bool ClientTransport::stop()
{
	if (!Transport::stop())
		return false;

	/* Do any transport specific stop */
	return JClkLibCommon::_stopTransport<ClientMessageQueue>();
}

bool ClientTransport::finalize()
{
	if (!Transport::finalize())
		return false;

	return JClkLibCommon::_finalizeTransport<ClientMessageQueue>();
}

