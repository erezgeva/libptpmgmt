/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2025 Intel Corporation. */

/** @file
 * @brief test message ExtractClassName
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2025 Intel Corporation.
 *
 */

#include "common/message.hpp"

using namespace clkmgr;

TEST(Message, ExtractClassName)
{
    std::string name = MSG_EXTRACT_CLASS_NAME;
    EXPECT_STREQ(name.c_str(), "Message_ExtractClassName_Test");
}
