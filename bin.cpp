/* SPDX-License-Identifier: LGPL-3.0-or-later */

/** @file
 * @brief allocate buffer
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 * @copyright GNU Lesser General Public License 3.0 or later
 *
 */

#include <arpa/inet.h>
#include "bin.h"

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
    Token(const std::string &str) : buf(strdup(str.c_str())) {}
    bool fail() {return buf == nullptr;}
    const char *first() {return strtok_r(buf, idsep, &save);}
    const char *next() {return strtok_r(nullptr, idsep, &save);}
};

static inline bool parseByte(const char *cur, std::string &id)
{
    char *end;
    long a = strtol(cur, &end, 16);
    if(end == cur || *end != 0 || a < 0 || a > 0xff)
        return true;
    id += a;
    return false;
}

std::string Binary::toIp() const
{
    char buf[INET6_ADDRSTRLEN];
    const char *ret = nullptr;
    size_t len = length();
    switch(len) {
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
bool Binary::fromIp(const std::string &string, int domain)
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
    m_str = std::string(buf, len);
    return true;
}
bool Binary::fromIp(const std::string &string)
{
    if(string.length() < 2)
        return false;
    int domain;
    if(string.find('.') != std::string::npos)
        domain = AF_INET; // IP v4
    else if(string.find(':') != std::string::npos)
        domain = AF_INET6; // IP v6
    else
        return false;
    return fromIp(string, domain);
}
std::string Binary::bufToId(const uint8_t *id, size_t len)
{
    if(len < 1)
        return "";
    char buf[10];
    snprintf(buf, sizeof(buf), "%02x", *id);
    std::string ret = buf;
    for(len--; len > 0; len--) {
        snprintf(buf, sizeof(buf), ":%02x", *++id);
        ret += buf;
    }
    return ret;
}
bool Binary::fromId(const std::string &string)
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
    m_str = id;
    return true;
}
bool Binary::eui48ToEui64()
{
    size_t len = length();
    if(len == EUI64)
        return true;
    if(len != EUI48)
        return false;
    std::string str = m_str.substr(0, 3);
    str += (char)0xff;
    str += (char)0xfe;
    str += m_str.substr(3);
    m_str = str;
    return true;
}
bool Binary::fromHex(const std::string hex)
{
    if(hex.empty())
        return false;
    Token tkn(hex);
    if(tkn.fail())
        return false;
    std::string id;
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
    m_str = id;
    return true;
}
std::string Binary::bufToHex(const uint8_t *bin, size_t len)
{
    if(len < 1)
        return "";
    char buf[10];
    std::string ret;
    for(; len > 0; len--) {
        snprintf(buf, sizeof(buf), "%02x", *bin++);
        ret += buf;
    }
    return ret;
}
