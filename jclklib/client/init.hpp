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
	bool subscribe(JClkLibCommon::jcl_subscription &subscription);
};

#endif/*CLIENT_INIT_HPP*/
