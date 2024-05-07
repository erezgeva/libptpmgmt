/*! \file init.hpp
    \brief Client utilities to setup and cleanup the library.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#ifndef CLIENT_INIT_HPP
#define CLIENT_INIT_HPP

#include <common/jclklib_import.hpp>

namespace JClkLibClient {
	bool jcl_connect();
	bool jcl_disconnect();
	bool jcl_subscribe(JClkLibCommon::jcl_subscription &newSub,
                       JClkLibCommon::jcl_state &currentState);
    int jcl_status_wait(int timeout, JClkLibCommon::jcl_state &jcl_state,
                        JClkLibCommon::jcl_state_event_count &eventCount);

};

#endif/*CLIENT_INIT_HPP*/
