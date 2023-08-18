/*! \file test.cpp
    \brief Test client code

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include "init.hpp"

#include <unistd.h>

using namespace JClkLibClient;
using namespace std;

int main()
{
	connect();
	sleep(1);
 do_exit:
	disconnect();
}
