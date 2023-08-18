/*! \file sighandler.hpp
    \brief Signal handling utilities

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#ifndef SIGHANDLER_HPP
#define SIGHANDLER_HPP

#include <thread>

namespace JClkLibCommon
{
	bool BlockStopSignal();
	bool WaitForStopSignal();
	bool EnableSyscallInterruptSignal();
	bool SendSyscallInterruptSignal(std::thread &t);
}
#endif/*SIGHANDLER_HPP*/

