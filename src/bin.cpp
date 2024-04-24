/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Allocate buffer
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 */

#include "config.h"
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include "comp.h"
#include "bin.h"

using namespace std;
__PTPMGMT_NAMESPACE_BEGIN

static const size_t min_alloc_size = 1 << 5; // 32 bytes
static const char *idsep = ":.-";
static inline bool parseByte(const char *cur, Binary &id)
{
    char *end;
    long a = strtol(cur, &end, 16);
    if(end == cur || *end != 0 || a < 0 || a > 0xff)
        return true;
    id += a;
    return false;
}
const char toB64[62] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K',
        'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
        'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o',
        'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
        '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
    };
const int B64[] = {62, 73, 82, -1, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61,
        -1, -1, -1, -1, -1, -1, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13,
        14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, 83, -1,
        26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43,
        44, 45, 46, 47, 48, 49, 50, 51
        };
// '+'           62
// ','           73  IMAP mailbox (63)
// '-'           82  URL (62)
// '/'           63
// '0' - '9'  52-61
// 'A' - 'Z'   0-25
// '_'           83  URL (63)
// 'a' - 'z'  26-51
static inline bool parseB64(const char c, uint8_t &v, uint8_t &safe)
{
#define URL   (0x1) //   0x1 - use URL and filename safe standard
#define BOTH  (0x2) //   0x2 - used standard or IMAP
#define STAND (0x4) //   0x6 - used standard only
#define IMAP  (0x8) //   0xa - used IMAP mailbox standard
    if(c < '+' || c > 'z')
        return true; // Character out of range
    int r = B64[c - '+'];
    switch(r) {
        case -1:
            return true; // Wrong character
        case 62: // standard or IMAP
            if((safe & URL) > 0)
                return true; // error mixed standard or IMAP with URL
            safe |= BOTH;
            break;
        case 63: // standard
            if((safe & (URL | IMAP)) > 0)
                return true; // error mixed standard with URL or IMAP
            safe = BOTH | STAND;
            break;
        case 73: // IMAP
            if((safe & (URL | STAND)) > 0)
                return true; // error mixed IMAP with URL or standard
            r -= 10;
            safe = BOTH | IMAP;
            break;
        case 82: // URL
            FALLTHROUGH;
        case 83:
            if((safe & BOTH) > 0)
                return true; // error mixed URL with standard or IMAP
            r -= 20;
            safe = URL;
            break;
        default:
            break; // No further check require
    }
    v = r;
    return false; // pass
}
void Binary::init()
{
    m_buf = nullptr;
    m_alloc = 0;
    m_size = 0;
}
bool Binary::iResize(size_t l_alloc)
{
    if(l_alloc <= m_alloc)
        return true;
    if(l_alloc <= min_alloc_size)
        l_alloc = min_alloc_size;
    else
        l_alloc += min_alloc_size - ((l_alloc - 1) & (min_alloc_size - 1)) - 1;
    uint8_t *l_buf = (uint8_t *)realloc(m_buf, l_alloc);
    if(l_buf == nullptr)
        return false;
    m_buf = l_buf;
    m_alloc = l_alloc;
    // Clear new allocated space
    if(m_alloc > m_size)
        memset(m_buf + m_size, 0, m_alloc - m_size);
    return true;
}
Binary::~Binary()
{
    if(m_buf != nullptr)
        free(m_buf);
}
Binary::Binary()
{
    init();
    iResize(min_alloc_size);
}
Binary::Binary(const Binary &rhs)
{
    init();
    setBin(rhs.m_buf, rhs.m_size);
    iResize(min_alloc_size);
}
Binary::Binary(const void *buf, const size_t length)
{
    init();
    setBin(buf, length);
    iResize(min_alloc_size);
}
Binary::Binary(const size_t length, uint8_t set)
{
    init();
    if(iResize(length)) {
        m_size = length;
        if(m_size > 0)
            memset(m_buf, set, m_size);
    }
}
Binary::Binary(const string &string)
{
    init();
    setBin(string);
    iResize(min_alloc_size);
}
Binary &Binary::operator=(const Binary &rhs)
{
    return setBin(rhs.m_buf, rhs.m_size);
}
Binary &Binary::setBin(const void *buf, const size_t length)
{
    if(buf == nullptr)
        m_size = 0;
    else if(iResize(length)) {
        m_size = length;
        if(m_size > 0)
            memcpy(m_buf, buf, m_size);
    }
    return *this;
}
Binary &Binary::setBin(const Binary &rhs)
{
    return setBin(rhs.m_buf, rhs.m_size);
}
Binary &Binary::setBin(const string &string)
{
    return setBin(string.c_str(), string.length());
}
Binary &Binary::setBin(const size_t position, const uint8_t value)
{
    if(iResize(position + 1)) {
        m_size = max(m_size, position + 1);
        m_buf[position] = value;
    }
    return *this;
}
const uint8_t Binary::getBin(const size_t position) const
{
    if(position >= m_size)
        return 0;
    return m_buf[position];
}
uint8_t &Binary::operator [](const size_t position)
{
    if(iResize(position + 1)) {
        m_size = max(m_size, position + 1);
        return m_buf[position];
    }
    // Used if allocation fails
    static uint8_t m_overflow;
    m_overflow = 0;
    return m_overflow;
}
Binary &Binary::resize(const size_t length)
{
    if(iResize(length)) {
        // Clear in case of shrink, as we leave allocated memory
        if(length > 0 && m_alloc > length)
            memset(m_buf + length, 0, m_alloc - length);
        m_size = length;
    }
    return *this;
}
Binary &Binary::append(const uint8_t octet)
{
    if(iResize(m_size + 1)) {
        m_buf[m_size] = octet;
        m_size++;
    }
    return *this;
}
Binary &Binary::append(const Binary &rhs)
{
    if(rhs.m_size > 0 && iResize(m_size + rhs.m_size)) {
        memcpy(m_buf + m_size, rhs.m_buf, rhs.m_size);
        m_size += rhs.m_size;
    }
    return *this;
}
string Binary::toIp() const
{
    char buf[INET6_ADDRSTRLEN];
    const char *ret = nullptr;
    switch(m_size) {
        case sizeof(in_addr):
            ret = inet_ntop(AF_INET, (in_addr *)get(), buf, INET_ADDRSTRLEN);
            break;
        case sizeof(in6_addr):
            ret = inet_ntop(AF_INET6, (in6_addr *)get(), buf, INET6_ADDRSTRLEN);
            break;
        default:
            return "";
    }
    if(ret == nullptr)
        return "";
    return ret;
}
bool Binary::fromIp(const string &string, int domain)
{
    if(string.length() < 2)
        return false;
    size_t len;
    char buf[sizeof(in6_addr)];
    switch(domain) {
        case AF_INET: // IP v4
            len = sizeof(in_addr);
            break;
        case AF_INET6: // IP v6
            len = sizeof(in6_addr);
            break;
        default:
            return false;
    }
    if(inet_pton(domain, string.c_str(), buf) != 1)
        return false;
    setBin(buf, len);
    return true;
}
bool Binary::fromIp(const string &string)
{
    if(string.length() < 2)
        return false;
    int domain;
    if(string.find('.') != string::npos)
        domain = AF_INET; // IP v4
    else if(string.find(':') != string::npos)
        domain = AF_INET6; // IP v6
    else
        return false;
    return fromIp(string, domain);
}
string Binary::bufToId(const uint8_t *id, size_t len)
{
    return bufToId(id, len, ":");
}
string Binary::bufToId(const uint8_t *id, size_t len, const std::string &sep)
{
    if(len < 1)
        return "";
    char buf[10];
    snprintf(buf, sizeof buf, "%02x", *id);
    string ret = buf;
    for(len--; len > 0; len--) {
        snprintf(buf, sizeof buf, "%02x", *++id);
        ret += sep;
        ret += buf;
    }
    return ret;
}
bool Binary::fromId(const string &string)
{
    if(string.length() < 2)
        return false;
    Token tkn(idsep);
    if(tkn.dup(string))
        return false;
    Binary id;
    for(const char *cur = tkn.first();
        cur != nullptr && *cur != 0; cur = tkn.next()) {
        if(parseByte(cur, id))
            return false;
    }
    setBin(id);
    return true;
}
bool Binary::eui48ToEui64()
{
    if(m_size == EUI64)
        return true;
    if(m_size != EUI48 || !iResize(EUI64))
        return false;
    m_size += 2;
    m_buf[7] = m_buf[5];
    m_buf[6] = m_buf[4];
    m_buf[5] = m_buf[3];
    m_buf[4] = 0xfe;
    m_buf[3] = 0xff;
    return true;
}
bool Binary::fromHex(const string &hex)
{
    if(hex.empty())
        return false;
    Token tkn(idsep);
    if(tkn.dup(hex))
        return false;
    Binary id;
    char nibbles[3];
    nibbles[2] = 0;
    const char *cur = tkn.first();
    while(cur != nullptr) {
        if(cur[0] != 0) { // parse byte
            nibbles[0] = cur[0];
            nibbles[1] = cur[1];
            if(parseByte(nibbles, id))
                return false;
        }
        if(cur[0] == 0 || cur[1] == 0 || cur[2] == 0)
            // Next token
            cur = tkn.next();
        else
            cur += 2; // next byte in token
    }
    setBin(id);
    return true;
}
string Binary::bufToHex(const uint8_t *bin, size_t len)
{
    if(len < 1)
        return "";
    char buf[10];
    string ret;
    for(; len > 0; len--) {
        snprintf(buf, sizeof buf, "%02x", *bin++);
        ret += buf;
    }
    return ret;
}
bool Binary::fromBase64(const std::string &bin64, bool pad)
{
    // Empty string is empty binary
    if(bin64.empty()) {
        clear();
        return true;
    }
    size_t len = bin64.size();
    const char *cur = bin64.c_str();
    uint8_t safe = 0; // String can use safe standard or normal standard not both!
    Binary id; // result
    uint8_t a[4]; // Convert 4 characters to 6 bits each
    // Hex need 4 bits, 2^4 = 16 equal 10 in base 16
    // base 64 needs 6 bits, 2^6 = 64 equal 10 in base 64
    for(; len > 4; len -= 4) {
        // Read 4 characters
        if(parseB64(*cur++, a[0], safe) ||
            parseB64(*cur++, a[1], safe) ||
            parseB64(*cur++, a[2], safe) ||
            parseB64(*cur++, a[3], safe))
            return false;
        // convert to 4 X 6 bits to 3 X 8 bits, octets. A 3/4 ratio.
        id += ((a[0] << 2) & 0xfc) | ((a[1] >> 4) & 0x03);
        id += ((a[1] << 4) & 0xf0) | ((a[2] >> 2) & 0x0f);
        id += ((a[2] << 6) & 0xc0) | (a[3] & 0x3f);
    }
    // = Here Length can be 1 to 4
    // minimum 2 character in last 4 characters
    if(len < 2 || (pad && len != 4))
        return false;
    // = Here Length can be 2 to 4
    // Strip padding
    if(len == 4 && cur[3] == '=')
        len--;
    if(len == 3 && cur[2] == '=')
        len--;
    // = Here Length can be 2 to 4
    // Read left characters
    for(size_t i = 0; i < len; i++) {
        if(parseB64(*cur++, a[i], safe))
            return false;
    }
    // First byte always exist
    id += ((a[0] << 2) & 0xfc) | ((a[1] >> 4) & 0x03);
    // When more than 2 characters used, the second byte exist
    if(len > 2) {
        id += ((a[1] << 4) & 0xf0) | ((a[2] >> 2) & 0x0f);
        // When all 4 characters are used, the third byte exist
        if(len == 4)
            id += ((a[2] << 6) & 0xc0) | (a[3] & 0x3f);
    }
    setBin(id);
    return true;
}
std::string Binary::toBase64(bool pad, char v62, char v63)
{
    if(m_size == 0) // Empty
        return "";
    char b64[64];
    std::string ret;
    size_t left = m_size;
    uint8_t *cur = m_buf;
    memcpy(b64, toB64, 62);
    b64[62] = v62;
    b64[63] = v63;
    for(; left > 3; left -= 3) {
        ret += b64[(cur[0] >> 2) & 0x3f];
        ret += b64[(((cur[0] & 0x03) << 4) & 0x30) | ((cur[1] >> 4) & 0x0f)];
        ret += b64[(((cur[1] & 0x0f) << 2) & 0x3c) | ((cur[2] >> 6) & 0x03)];
        ret += b64[cur[2] & 0x3f];
        cur += 3;
    }
    uint8_t v[3] = {0};
    v[0] = cur[0];
    if(left > 1) {
        v[1] = cur[1];
        if(left > 2)
            v[2] = cur[2];
    }
    ret += b64[(v[0] >> 2) & 0x3f];
    ret += b64[(((v[0] & 0x3) << 4) & 0x30) | ((v[1] >> 4) & 0xf)];
    if(left > 1) {
        ret += b64[(((v[1] & 0xf) << 2) & 0x3c) | ((v[2] >> 6) & 0x3)];
        if(left > 2)
            ret += b64[v[2] & 0x3f];
        else if(pad)
            ret += "=";
    } else if(pad)
        ret += "==";
    return ret;
}
bool Binary::eq(const Binary &rhs) const
{
    // We only equal if we have the same size
    // So if we are not the same size
    // We can be not less, not big and not equal!
    return m_size == rhs.m_size &&
        (m_size == 0 || memcmp(m_buf, rhs.m_buf, m_size) == 0);
}
bool Binary::less(const Binary &rhs) const
{
    size_t size = min(m_size, rhs.m_size);
    return size > 0 && memcmp(m_buf, rhs.m_buf, size) < 0;
}

__PTPMGMT_NAMESPACE_END
