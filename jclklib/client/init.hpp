/*! \file init.hpp
    \brief Client utilities to setup and cleanup the library.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#ifndef CLIENT_INIT_HPP
#define CLIENT_INIT_HPP

#include <common/jclklib_import.hpp>

namespace JClkLibClient {
	bool connect();
	bool disconnect();
	bool subscribe(JClkLibCommon::jcl_subscription &newSub, JClkLibCommon::jcl_state &currentState);
    int status_wait( unsigned timeout, JClkLibCommon::jcl_state &jcl_state , JClkLibCommon::jcl_state_event_count &eventCount);

};

#endif/*CLIENT_INIT_HPP*/
