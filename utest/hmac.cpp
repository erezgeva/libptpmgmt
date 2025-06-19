/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2024 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Unit tests for hmac libraries.
 *
 * This is internal API test,
 * to ensure all libraries gives the same result.
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2024 Erez Geva
 *
 */

#include "comp.h"

__PTPMGMT_NAMESPACE_USE;

// openssl rand -hex 32
static uint8_t bkey[] = {14, 0xdd, 0xb0, 0xfd, 0x50, 0x13, 0xff, 0xca, 0x46,
        0x41, 0x7f, 0xf2, 0x9d, 0x34, 0x35, 0xd4, 0x9d, 0xaa, 1, 0xc3, 0x37,
        0xce, 0x4c, 0x4c, 0x1c, 0x2c, 0x46, 0x75, 0xae, 0x60, 0x5c, 0xcd
    };
static uint8_t hData[] = {0xdc, 0xae, 0x14, 0xc5, 0x36, 0x59, 0x91, 0x53, 0xa8,
        0x3a, 0x7d, 0x28, 0x9f, 0x65, 0xda, 0xce, 0x65, 0xa2, 0xfe, 0xfb, 0xa3,
        0x1c, 0x2b, 0x13, 0xfd, 0x7d, 0x9c, 0xc4, 0x89, 0xe0, 0xa8, 0xf2, 0x26,
        0xad, 0x30, 0xb5, 0x9c, 0xbc, 0x58, 10, 0x34, 0x52, 0xde, 0x2b, 0xee,
        0xa9, 0xb5, 0xd2, 0xa9, 0xf7, 0x35, 0x52, 0x34, 0x96, 0xab, 0x2f, 0x9c,
        0xbf, 0x71, 0x5c, 0x46, 0xb6, 0xf, 0x67, 0xdf, 0x18, 0xf0, 0x5f, 0x65,
        0x1d, 0x81, 0x6b, 0xef, 0x83, 0x5f, 0x68, 0xb7, 0xa2, 0x55, 0x3c, 0x2c,
        0x91, 0x5e, 0xdc, 0x63, 0x4b, 0x22, 0x20, 0xb9, 0x6d, 0xbb, 0xf5, 0x37,
        0x68, 0xa6, 0xe7, 0xcf, 0x82, 0x28, 0xfd, 0xd4, 0x11, 0xdc, 0xfe, 0x82,
        0x69, 0xe0, 0x99, 0xe1, 0x8f, 0xe1, 0xf1, 0x3b, 0xa9, 0x15, 0x43, 0xf7,
        0xbd, 0xee, 0x2a, 0x78, 0xac, 0x8d, 0xd4, 0x8b, 0xca, 3, 0x3c, 0x48,
        0x76, 0x7a, 0xef, 0xec, 0x6e, 0x68, 0xbb, 0x8c
    };

static std::string ssl("libptpmgmt_openssl.a");
static std::string gcrypt("libptpmgmt_gcrypt.a");
static std::string gnutls("libptpmgmt_gnutls.a");
static std::string nettle("libptpmgmt_nettle.a");

// Tests loadLibrary
// const char *hmac_loadLibrary()
TEST(hmacTest, MethodLoadLibrary)
{
    // We use static link.
    // We get the name of the static library.
    std::string name(hmac_loadLibrary());
    EXPECT_TRUE(name == ssl || name == gcrypt || name == gnutls || name == nettle);
}

// Tests selectLib method
// bool hmac_selectLib(const std::string &libMatch)
TEST(hmacTest, MethodSelectLib)
{
    // We use static link :-)
    // We alway return false here.
    EXPECT_FALSE(hmac_selectLib("gnutls"));
}

// Tests isLibShared method
// bool hmac_isLibShared();
TEST(hmacTest, MethodIsLibShared)
{
    // We use static link, not shared :-)
    EXPECT_FALSE(hmac_isLibShared());
}

TEST(hmacTest, SHA256_128)
{
    Binary key(bkey, 32);
    EXPECT_EQ(hmac_count(), 0);
    HMAC_Key *hmac = hmac_allocHMAC(HMAC_SHA256, key);
    ASSERT_NE(hmac, nullptr);
    EXPECT_EQ(hmac_count(), 1);
    EXPECT_EQ(hmac->m_type, HMAC_SHA256);
    EXPECT_EQ(hmac->m_key, key);
    Binary mac(16);
    EXPECT_TRUE(hmac->digest(hData, sizeof hData, mac));
    //printf("digest: %s\n", Error::getError().c_str());
    //printf("mac: %zu %s\n", mac.size(), mac.toId().c_str());
    uint8_t ret[16] = { 0x28, 0x51, 0xef, 0x15, 0x3a, 0xb6, 0x5b, 0x43, 0x98,
            0x5e, 0x80, 0x7f, 0x93, 0xd6, 14, 0xa5
        };
    EXPECT_EQ(memcmp(mac.get(), ret, mac.size()), 0);
    EXPECT_TRUE(hmac->verify(hData, sizeof hData, mac));
    delete hmac;
    EXPECT_EQ(hmac_count(), 0);
}

TEST(hmacTest, SHA256)
{
    Binary key(bkey, 32);
    HMAC_Key *hmac = hmac_allocHMAC(HMAC_SHA256, key);
    ASSERT_NE(hmac, nullptr);
    EXPECT_EQ(hmac->m_type, HMAC_SHA256);
    EXPECT_EQ(hmac->m_key, key);
    Binary mac(32);
    EXPECT_TRUE(hmac->digest(hData, sizeof hData, mac));
    uint8_t ret[32] = {0x28, 0x51, 0xef, 0x15, 0x3a, 0xb6, 0x5b, 0x43, 0x98, 0x5e,
            0x80, 0x7f, 0x93, 0xd6, 14, 0xa5, 0xef, 0xe4, 0xaa, 0xd3, 0xba,
            0x3f, 0xde, 0x3b, 0x9a, 0xc4, 0x43, 0xc5, 0xc1, 0xc4, 0x50, 0x51
        };
    EXPECT_EQ(memcmp(mac.get(), ret, mac.size()), 0);
    EXPECT_TRUE(hmac->verify(hData, sizeof hData, mac));
    delete hmac;
}

TEST(hmacTest, AES128)
{
    Binary key(bkey, 16);
    HMAC_Key *hmac = hmac_allocHMAC(HMAC_AES128, key);
    ASSERT_NE(hmac, nullptr);
    EXPECT_EQ(hmac->m_type, HMAC_AES128);
    EXPECT_EQ(hmac->m_key, key);
    Binary mac(16);
    EXPECT_TRUE(hmac->digest(hData, sizeof hData, mac));
    uint8_t ret[16] = { 0xe3, 0x47, 0xda, 0x49, 0xea, 0x7d, 0xd6, 0xb6,
            0xff, 0xae, 0x44, 2, 0x40, 0x8f, 0xcd, 0x93
        };
    EXPECT_EQ(memcmp(mac.get(), ret, mac.size()), 0);
    EXPECT_TRUE(hmac->verify(hData, sizeof hData, mac));
    delete hmac;
}

TEST(hmacTest, AES256)
{
    Binary key(bkey, 32);
    HMAC_Key *hmac = hmac_allocHMAC(HMAC_AES256, key);
    ASSERT_NE(hmac, nullptr);
    EXPECT_EQ(hmac->m_type, HMAC_AES256);
    EXPECT_EQ(hmac->m_key, key);
    Binary mac(16);
    EXPECT_TRUE(hmac->digest(hData, sizeof hData, mac));
    uint8_t ret[16] = { 0x2b, 0x7c, 0xc1, 0x7b, 0xd0, 0xa9, 0x7a, 0xb1, 0x6f,
            0x82, 0x80, 0x6d, 0xb0, 0xaf, 0xed, 0x51
        };
    EXPECT_EQ(memcmp(mac.get(), ret, mac.size()), 0);
    EXPECT_TRUE(hmac->verify(hData, sizeof hData, mac));
    delete hmac;
}
