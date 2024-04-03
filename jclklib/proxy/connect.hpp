/*! \file connect.hpp
    \brief Proxy connect message class.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Lai Peter Jun Ann <peter.jun.ann.lai@intel.com>
*/

#include <proxy/message.hpp>
#include <common/connect_msg.hpp>

namespace JClkLibProxy
{
    class Connect
    {
    private:
    public:
        static int connect();
    };
}