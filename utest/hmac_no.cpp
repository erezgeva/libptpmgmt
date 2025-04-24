/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Unit tests for absent of any hmac library.
 *
 * This is internal API test,
 * to ensure static link without any hmac library works.
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 *
 */

#include "comp.h"

__PTPMGMT_NAMESPACE_USE;

TEST(hmacTest, NoHmac)
{
    Binary key(16);
    EXPECT_EQ(hmac_count(), 0);
    HMAC_Key *hmac = hmac_allocHMAC(HMAC_AES128, key);
    /* There is no HMAC library, so we return nullptr! */
    ASSERT_EQ(hmac, nullptr);
    EXPECT_EQ(hmac_count(), 0);
    /* No library is loaded! */
    EXPECT_EQ(hmac_loadLibrary(), nullptr);
}
