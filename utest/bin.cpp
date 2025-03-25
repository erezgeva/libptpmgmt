/* SPDX-License-Identifier: GPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2022 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Binary class unit tests
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2022 Erez Geva
 *
 */

#include "config.h"
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include "bin.h"

using namespace ptpmgmt;

// Tests Binary empty constructor
// Binary()
TEST(BinaryTest, MethodEmptyConstructor)
{
    Binary f;
    EXPECT_TRUE(f.getBinString() == "");
    EXPECT_EQ(f.length(), 0);
}

// Tests Binary copy constructor
// Binary(const Binary &rhs)
TEST(BinaryTest, MethodCopyConstructor)
{
    Binary g("\x1\x2\x3\x4");
    Binary f(g);
    EXPECT_EQ(f.getBinString(), "\x1\x2\x3\x4");
}

// Tests Binary move constructor
// Binary(Binary &&rhs)
TEST(BinaryTest, MethodMoveConstructor)
{
    Binary d("\x1\x2\x3\x4");
    const uint8_t *p = d.get();
    Binary f(std::move(d));
    EXPECT_EQ(d.get(), nullptr);
    EXPECT_EQ(d.size(), 0);
    EXPECT_EQ(f.get(), p);
    EXPECT_EQ(f.getBinString(), "\x1\x2\x3\x4");
    EXPECT_EQ(f.size(), 4);
}

// Tests Binary constructor from buffer
// Binary(const void *buf, const size_t length)
TEST(BinaryTest, MethodBufferConstructor)
{
    const uint8_t input[] = "\x1\x2\x3\x4";
    Binary f(input, sizeof input - 1);
    EXPECT_EQ(f.getBinString(), "\x1\x2\x3\x4");
}

// Tests Binary constructor buffer with fixed length
// Binary(const size_t length, uint8_t set = 0)
TEST(BinaryTest, MethodBufferFixedConstructor)
{
    Binary f(4, 0xfe);
    EXPECT_EQ(f.getBinString(), "\xfe\xfe\xfe\xfe");
}

// Tests Binary constructor from binary based string
// Binary(const std::string &string)
TEST(BinaryTest, MethodBinStringConstructor)
{
    const std::string input = "\x1\x2\x3\x4";
    Binary f(input);
    EXPECT_EQ(f.getBinString(), "\x1\x2\x3\x4");
}

// Tests Binary copy from another object
// Binary &operator=(const Binary &rhs)
TEST(BinaryTest, MethodCopyOther)
{
    Binary g("\x1\x2\x3\x4");
    Binary f = g;
    EXPECT_EQ(f.getBinString(), "\x1\x2\x3\x4");
}

// Tests the length method
// size_t length() const
TEST(BinaryTest, MethodLength)
{
    Binary f("\x1\x2\x3\x4");
    EXPECT_EQ(f.length(), 4);
}

// Tests the size method
// size_t size() const
TEST(BinaryTest, MethodSize)
{
    Binary f("\x1\x2\x3\x4\x5");
    EXPECT_EQ(f.length(), 5);
}

// Tests Binary empty method
// bool empty() const
TEST(BinaryTest, MethodEmpty)
{
    Binary f;
    EXPECT_TRUE(f.empty());
    f.setBin("\x1\x2\x3\x4\x5");
    EXPECT_FALSE(f.empty());
}
// Test get pointer to binary
// const uint8_t *get() const
TEST(BinaryTest, MethodGet)
{
    Binary f("\x1\x2\x3\x4");
    EXPECT_EQ(memcmp(f.get(), "\x1\x2\x3\x4", 4), 0);
}

// Test get binary based string
// std::string getBinString() const
TEST(BinaryTest, MethodGetBinString)
{
    Binary f("\x1\x2\x3\x4");
    EXPECT_EQ(f.getBinString(), "\x1\x2\x3\x4");
}

// Test set binary value
// Binary &setBin(const void *buf, const size_t length)
TEST(BinaryTest, MethodSet)
{
    const uint8_t input[] = "\x1\x2\x3\x4";
    Binary f
    (input, sizeof input - 1);
    EXPECT_EQ(f.getBinString(), "\x1\x2\x3\x4");
}

// Test set binary from binary
// Binary &setBin(const Binary &rhs);
TEST(BinaryTest, MethodSetBinBin)
{
    Binary g("\x1\x2\x3\x4");
    Binary f;
    f.setBin(g);
    EXPECT_EQ(f.getBinString(), "\x1\x2\x3\x4");
}

// Test set binary based string
// Binary &setBin(const std::string &string)
TEST(BinaryTest, MethodSetBinString)
{
    const std::string input = "\x1\x2\x3\x4";
    Binary f;
    f.setBin(input);
    EXPECT_EQ(f.getBinString(), "\x1\x2\x3\x4");
}

// Test set value in position
// Binary &setBin(const size_t position, const uint8_t value)
TEST(BinaryTest, MethodSetBinVal)
{
    Binary f("\x1\x2\x3\x4");
    f.setBin(2, 0xf);
    EXPECT_EQ(f.getBinString(), "\x1\x2\xf\x4");
}

// Test move value
// Binary &mvBin(Binary &rhs)
TEST(BinaryTest, MethodMoveBinVal)
{
    Binary f("\x1\x2\x3\x4");
    const uint8_t *p = f.get();
    Binary t;
    t.mvBin(f);
    EXPECT_EQ(f.get(), nullptr);
    EXPECT_EQ(f.size(), 0);
    EXPECT_EQ(t.get(), p);
    EXPECT_EQ(t.size(), 4);
}

// Test get value in position
// const uint8_t getBin(const size_t position) const
TEST(BinaryTest, MethodGetBinPosVal)
{
    Binary f("\x1\x2\x3\x4");
    EXPECT_EQ(f.getBin(2), 3);
}

// Test operator index
// const uint8_t operator [](const size_t position) const
TEST(BinaryTest, MethodOpIdx)
{
    Binary f("\x1\x2\x3\x4");
    EXPECT_EQ(f[2], 3);
    f[5] = 6;
    EXPECT_EQ(memcmp(f.get(), "\x1\x2\x3\x4\0\x6", 6), 0);
}

// Test resize
// Binary &resize(const size_t length)
TEST(BinaryTest, MethodResize)
{
    Binary f("\x1\x2\x3\x4");
    f.resize(3);
    EXPECT_EQ(f.getBinString(), "\x1\x2\x3");
    f.resize(6);
    EXPECT_EQ(memcmp(f.get(), "\x1\x2\x3\0\0", 6), 0);
}

// Test clear
// Binary &clear()
TEST(BinaryTest, MethodClear)
{
    Binary f("\x1\x2\x3\x4");
    EXPECT_FALSE(f.empty());
    f.clear();
    EXPECT_TRUE(f.empty());
}

// Test copy buffer
// void copy(uint8_t *target) const
TEST(BinaryTest, MethodCopyBuffer)
{
    uint8_t buf[4];
    Binary f("\x1\x2\x3\x4");
    f.copy(buf);
    EXPECT_EQ(memcmp(buf, "\x1\x2\x3\x4", 4), 0);
}

// Test append single octet using add equal operator
// Binary &operator += (const uint8_t octet)
TEST(BinaryTest, MethodAddOctetOp)
{
    Binary f("\x1\x2\x3\x4");
    (f += 0x5) += 0x6;
    EXPECT_EQ(f.getBinString(), "\x1\x2\x3\x4\x5\x6");
}

// Test append single octet
// Binary &append(const uint8_t octet)
TEST(BinaryTest, MethodAddOctet)
{
    Binary f("\x1\x2\x3\x4");
    f.append(0x5).append(0x6);
    EXPECT_EQ(f.getBinString(), "\x1\x2\x3\x4\x5\x6");
}

// Test append octets using add equal operator
// Binary &operator += (const Binary &rhs)
TEST(BinaryTest, MethodAddOctetsOp)
{
    Binary f("\x1\x2\x3\x4");
    Binary g("\x5\x6\x7\x8");
    f += g;
    EXPECT_EQ(f.getBinString(), "\x1\x2\x3\x4\x5\x6\x7\x8");
}

// Test append octets
// Binary &append(const Binary &rhs)
TEST(BinaryTest, MethodAddOctets)
{
    Binary f("\x1\x2\x3\x4");
    Binary g("\x5\x6\x7\x8");
    f.append(g);
    EXPECT_EQ(f.getBinString(), "\x1\x2\x3\x4\x5\x6\x7\x8");
}

// Test convert IPv4 address from binary to text
// std::string toIp() const
TEST(BinaryTest, MethodToIPv4)
{
    Binary f("\xc0\xa8\x1\x47");
    EXPECT_EQ(f.toIp(), "192.168.1.71");
}

// Test convert IPv6 address from binary to text
// std::string toIp() const
TEST(BinaryTest, MethodToIPv6)
{
    Binary f("\x80\x80\0\x5b\0\0\0\0\0\xc0\0\xa8\0\x1\0\x49", 16);
    EXPECT_EQ(f.toIp(), "8080:5b::c0:a8:1:49");
}

// Test convert IP address from text to binary
// bool fromIp(const std::string &string)
TEST(BinaryTest, MethodFromIP)
{
    Binary f;
    EXPECT_TRUE(f.fromIp("192.168.1.73"));
    EXPECT_EQ(f.length(), 4);
    EXPECT_EQ(f.getBinString(), "\xc0\xa8\x1\x49");
    EXPECT_TRUE(f.fromIp("8080:5b::c0:a8:1:49"));
    EXPECT_EQ(f.length(), 16);
    EXPECT_EQ(memcmp(f.get(), "\x80\x80\0\x5b\0\0\0\0\0\xc0\0\xa8\0\x1\0\x49",
            16), 0);
}

// Test convert IPv4/6 address from text to binary
// bool fromIp(const std::string &string, int domain)
TEST(BinaryTest, MethodFromIPDomain)
{
    Binary f;
    EXPECT_TRUE(f.fromIp("192.168.1.73", AF_INET));
    EXPECT_EQ(f.length(), 4);
    EXPECT_EQ(f.getBinString(), "\xc0\xa8\x1\x49");
    EXPECT_TRUE(f.fromIp("8080:5b::c0:a8:1:49", AF_INET6));
    EXPECT_EQ(f.length(), 16);
    EXPECT_EQ(memcmp(f.get(), "\x80\x80\0\x5b\0\0\0\0\0\xc0\0\xa8\0\x1\0\x49",
            16), 0);
}

// Test convert ID from binary to text
// std::string toId(const std::string &sep = ":") const
TEST(BinaryTest, MethodToID)
{
    Binary f("\x1b\xba\x23");
    EXPECT_EQ(f.toId(), "1b:ba:23");
    EXPECT_EQ(f.toId("-"), "1b-ba-23");
}

// Test convert ID from binary to text
// static std::string bufToId(const uint8_t *id, size_t length)
// static std::string bufToId(const uint8_t *id, size_t length,
//     const std::string &sep)
TEST(BinaryTest, MethodBufToId)
{
    const uint8_t input[] = "\x1b\xba\x23";
    EXPECT_EQ(Binary::bufToId(input, sizeof input - 1), "1b:ba:23");
    EXPECT_EQ(Binary::bufToId(input, sizeof input - 1, "-"), "1b-ba-23");
}

// Test convert ID from text to binary
// bool fromId(const std::string &string)
TEST(BinaryTest, MethodFromId)
{
    Binary f;
    EXPECT_TRUE(f.fromId("1b:ba:23"));
    EXPECT_EQ(f.length(), 3);
    EXPECT_EQ(f.getBinString(), "\x1b\xba\x23");
}

// Test convert MAC address from text to binary
// bool fromMac(const std::string &string)
TEST(BinaryTest, MethodFromMac)
{
    Binary f;
    EXPECT_TRUE(f.fromMac("1b:2c:3d:b5:a4:3f"));
    EXPECT_EQ(f.length(), 6);
    EXPECT_EQ(f.getBinString(), "\x1b\x2c\x3d\xb5\xa4\x3f");
    EXPECT_TRUE(f.fromMac("1b:2c:3d:b5:a4:3f:4e:3b"));
    EXPECT_EQ(f.length(), 8);
    EXPECT_EQ(f.getBinString(), "\x1b\x2c\x3d\xb5\xa4\x3f\x4e\x3b");
}

// Test for MAC address binary length
// bool isMacLen() const
TEST(BinaryTest, MethodIsMacLen)
{
    Binary f("\x1\x2\x3\x4\x5");
    EXPECT_FALSE(f.isMacLen()); // Legth 5 is invalid
    f += 6;
    EXPECT_TRUE(f.isMacLen()); // Length 6 is valid
    f += 7;
    EXPECT_FALSE(f.isMacLen()); // Legth 7 is invalid
    f += 8;
    EXPECT_TRUE(f.isMacLen()); // Length 8 is valid
    f += 9;
    EXPECT_FALSE(f.isMacLen()); // Legth 9 is invalid
}

// Test conver MAC address binary from 6 to 8 octets
// bool eui48ToEui64()
TEST(BinaryTest, MethodEui48ToEui64)
{
    Binary f("\x1\x2\x3\x4\x5\x6");
    EXPECT_TRUE(f.eui48ToEui64());
    EXPECT_EQ(f.getBinString(), "\x1\x2\x3\xff\xfe\x4\x5\x6");
}

// Test convert hex text to binary
// bool fromHex(const std::string hex)
TEST(BinaryTest, MethodFromHex)
{
    Binary f;
    EXPECT_TRUE(f.fromHex("4bafdd"));
    EXPECT_EQ(f.length(), 3);
    EXPECT_EQ(f.getBinString(), "\x4b\xaf\xdd");
    EXPECT_TRUE(f.fromHex("4b.af-7df5.bb"));
    EXPECT_EQ(f.length(), 5);
    EXPECT_EQ(f.getBinString(), "\x4b\xaf\x7d\xf5\xbb");
}

// Test convert hex binary to text
// std::string toHex() const
TEST(BinaryTest, MethodToHex)
{
    Binary f("\x1\x2\x3\x4\x5\x6");
    EXPECT_EQ(f.toHex(), "010203040506");
}

// Test convert hex binary buffer to text
// static std::string bufToHex(const uint8_t *bin, size_t length)
TEST(BinaryTest, MethodBufToHex)
{
    const uint8_t input[] = "\x1\x2\x3\x4\x5";
    EXPECT_EQ(Binary::bufToHex(input, sizeof input - 1), "0102030405");
}

// Convert from Base64 string
// bool fromBase64(const std::string &bin64, bool)
TEST(BinaryTest, MethodFromBase64)
{
    // 68 characters represent 51 bytes (ration of 3/4)
#define S_STD "wbwkYB4SuPQjmNaxj/Iq03u5152XBjI019v+GDU1MjPssH/lAi5+5Exoe/mShhm+oro9"
    // 68 characters with URL and filename safe standard ( with '-', '_' )
#define S_SAF "wbwkYB4SuPQjmNaxj_Iq03u5152XBjI019v-GDU1MjPssH_lAi5-5Exoe_mShhm-oro9"
    // 68 characters with IMAP ( with ',')
#define S_IMP "wbwkYB4SuPQjmNaxj,Iq03u5152XBjI019v+GDU1MjPssH,lAi5+5Exoe,mShhm+oro9"
    uint8_t b[54] = {0xc1, 0xbc, 0x24, 0x60, 0x1e, 0x12, 0xb8, 0xf4, 0x23, 0x98,
            0xd6, 0xb1, 0x8f, 0xf2, 0x2a, 0xd3, 0x7b, 0xb9, 0xd7, 0x9d, 0x97, 0x06,
            0x32, 0x34, 0xd7, 0xdb, 0xfe, 0x18, 0x35, 0x35, 0x32, 0x33, 0xec, 0xb0,
            0x7f, 0xe5, 0x02, 0x2e, 0x7e, 0xe4, 0x4c, 0x68, 0x7b, 0xf9, 0x92, 0x86,
            0x19, 0xbe, 0xa2, 0xba, 0x3d, 0x32, 0x0e, 0x41
        };
    // Empty
    Binary e(10);
    EXPECT_FALSE(e.empty());
    EXPECT_TRUE(e.fromBase64(""));
    EXPECT_TRUE(e.empty());
    // 54 bytes do not need any pad
    Binary b0t1, b0t2, b0t3, b0t4, b0t5, b0t6, bt(b, sizeof b);
    EXPECT_TRUE(b0t1.fromBase64(S_STD "Mg5B"));
    EXPECT_EQ(bt, b0t1);
    EXPECT_TRUE(b0t2.fromBase64(S_SAF "Mg5B"));
    EXPECT_EQ(bt, b0t2);
    EXPECT_TRUE(b0t3.fromBase64(S_IMP "Mg5B"));
    EXPECT_EQ(bt, b0t3);
    EXPECT_TRUE(b0t4.fromBase64(S_STD "Mg5B", true));
    EXPECT_EQ(bt, b0t4);
    EXPECT_TRUE(b0t5.fromBase64(S_SAF "Mg5B", true));
    EXPECT_EQ(bt, b0t5);
    EXPECT_TRUE(b0t6.fromBase64(S_IMP "Mg5B", true));
    EXPECT_EQ(bt, b0t6);
    // 53 bytes with a single pad (or none)
    Binary m1t1, m1t2, m1t3, m1(b, sizeof b - 1);
    EXPECT_TRUE(m1t1.fromBase64(S_STD "Mg4"));
    EXPECT_EQ(m1, m1t1);
    EXPECT_TRUE(m1t2.fromBase64(S_STD "Mg4=", true));
    EXPECT_EQ(m1, m1t2);
    EXPECT_TRUE(m1t3.fromBase64(S_STD "Mg4="));
    EXPECT_EQ(m1, m1t3);
    // 52 bytes with 2 pads (or none)
    Binary m2t1, m2t2, m2t3, m2(b, sizeof b - 2);
    EXPECT_TRUE(m2t1.fromBase64(S_STD "Mg"));
    EXPECT_EQ(m2, m2t1);
    EXPECT_TRUE(m2t2.fromBase64(S_STD "Mg==", true));
    EXPECT_EQ(m2, m2t2);
    EXPECT_TRUE(m2t3.fromBase64(S_STD "Mg=="));
    EXPECT_EQ(m2, m2t3);
}

// Convert to Base64 string
// std::string Binary::toBase64(bool pad, char v62, char v63)
TEST(BinaryTest, MethodToBase64)
{
    uint8_t bv[54] = {0xc1, 0xbc, 0x24, 0x60, 0x1e, 0x12, 0xb8, 0xf4, 0x23, 0x98,
            0xd6, 0xb1, 0x8f, 0xf2, 0x2a, 0xd3, 0x7b, 0xb9, 0xd7, 0x9d, 0x97, 0x06,
            0x32, 0x34, 0xd7, 0xdb, 0xfe, 0x18, 0x35, 0x35, 0x32, 0x33, 0xec, 0xb0,
            0x7f, 0xe5, 0x02, 0x2e, 0x7e, 0xe4, 0x4c, 0x68, 0x7b, 0xf9, 0x92, 0x86,
            0x19, 0xbe, 0xa2, 0xba, 0x3d, 0x32, 0x0e, 0x41
        };
    Binary b(bv, 54);
    std::string ret = b.toBase64();
    EXPECT_STREQ(ret.c_str(), S_STD "Mg5B");
    ret = b.toBase64(true, '-', '_');
    EXPECT_STREQ(ret.c_str(), S_SAF "Mg5B");
    b.resize(53);
    ret = b.toBase64();
    EXPECT_STREQ(ret.c_str(), S_STD "Mg4=");
    ret = b.toBase64(false);
    EXPECT_STREQ(ret.c_str(), S_STD "Mg4");
    b.resize(52);
    ret = b.toBase64();
    EXPECT_STREQ(ret.c_str(), S_STD "Mg==");
    ret = b.toBase64(false);
    EXPECT_STREQ(ret.c_str(), S_STD "Mg");
    b.resize(51);
    ret = b.toBase64();
    EXPECT_STREQ(ret.c_str(), S_STD);
}

// Test operator equal to other object
// bool operator==(const Binary &rhs)
TEST(BinaryTest, MethodEqualOp)
{
    Binary f("\x1\x2\x3\x4\x5\x6");
    Binary g("\x1\x2\x3\x4\x5\x6");
    EXPECT_TRUE(f == g);
}

// Test equal to other object
// bool eq(const Binary &rhs)
TEST(BinaryTest, MethodEqual)
{
    Binary f("\x1\x2\x3\x4\x5\x6");
    Binary g("\x1\x2\x3\x4\x5\x6");
    EXPECT_TRUE(f.eq(g));
}

// Test operator less to other object
// bool operator<(const Binary &rhs)
TEST(BinaryTest, MethodLessOp)
{
    Binary f("\x1\x2\x3\x4\x5\x6");
    Binary g("\x1\x2\x3\x5");
    EXPECT_TRUE(f < g);
}

// Test less to other object
// bool less(const Binary &rhs)
TEST(BinaryTest, MethodLess)
{
    Binary f("\x1\x2\x3\x4\x5\x6");
    Binary g("\x1\x2\x3\x5");
    EXPECT_TRUE(f.less(g));
}
