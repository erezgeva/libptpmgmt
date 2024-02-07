/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Allocate buffer
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 */

#include <arpa/inet.h>
#include "bin.h"

using namespace std;
__PTPMGMT_NAMESPACE_BEGIN

static const size_t min_alloc_size = 1 << 5; // 32 bytes
static const char *idsep = ":.-";
class Token
{
  private:
    char *buf;
    char *save;
  public:
    ~Token() {
        if(buf != nullptr)
            free(buf);
    }
    Token(const string &str) : buf(strdup(str.c_str())) {}
    bool fail() {return buf == nullptr;}
    const char *first() {return strtok_r(buf, idsep, &save);}
    const char *next() {return strtok_r(nullptr, idsep, &save);}
};

static inline bool parseByte(const char *cur, string &id)
{
    char *end;
    long a = strtol(cur, &end, 16);
    if(end == cur || *end != 0 || a < 0 || a > 0xff)
        return true;
    id += a;
    return false;
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
    // Allocate in 16 bytes blocks
    size_t fraq = l_alloc & (min_alloc_size - 1);
    if(fraq > 0)
        l_alloc += min_alloc_size - fraq;
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
Binary::Binary()
{
    init();
    iResize(min_alloc_size);
}
Binary::~Binary()
{
    if(m_buf != nullptr)
        free(m_buf);
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
    iResize(min_alloc_size);
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
    if(len < 1)
        return "";
    char buf[10];
    snprintf(buf, sizeof buf, "%02x", *id);
    string ret = buf;
    for(len--; len > 0; len--) {
        snprintf(buf, sizeof buf, ":%02x", *++id);
        ret += buf;
    }
    return ret;
}
bool Binary::fromId(const string &string)
{
    if(string.length() < 2)
        return false;
    Token tkn(string);
    if(tkn.fail())
        return false;
    std::string id;
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
    Token tkn(hex);
    if(tkn.fail())
        return false;
    string id;
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
