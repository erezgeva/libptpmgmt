/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2026 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Utest Swig helper library file
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2026 Erez Geva
 *
 */

#include "libutest.h"
#include "msg.h"
#include "comp.h"

__PTPMGMT_NAMESPACE_USE;

static inline size_t start(uint8_t *buf, size_t size)
{
    Message msg;
    if(size < 44 || msg.build(buf, size, 1) != MNG_PARSE_ERROR_OK)
        return 0;
    buf[0] = (buf[0] & 0xf0) | Signaling; // messageType
    buf[32] = 5; // controlField
    // signaling = 36 header + 10 targetPortIdentity = 44
    return 44;
}
static inline size_t addTlv_f(uint16_t *cur, tlvType_e type, uint8_t *data,
    size_t len)
{
    *cur++ = cpu_to_net16(type);
    *cur++ = cpu_to_net16(len);
    memcpy(cur, data, len);
    return len + 4;
}
static inline size_t setBuf(uint8_t *buf, size_t curLen)
{
    // header.messageLength
    *(uint16_t *)(buf + 2) = cpu_to_net16(curLen);
    return curLen;
}
#define startSig\
    size_t curLen = start(buf, bufSize);\
    if(curLen == 0) return 0;\
    uint16_t *cur = (uint16_t *)(buf + curLen)
#define addTlv(type, data) do { \
        if(bufSize < curLen + 4 + sizeof data) return 0;\
        size_t len = addTlv_f(cur, type, data, sizeof data);\
        curLen += len; cur += len >> 1; } while(0)
static inline size_t get2MngTlvsSigIntern(uint8_t *buf, size_t bufSize)
{
    startSig;
    uint8_t m1[4] = {0x20, 6, 119}; // PRIORITY2 priority2 = 119
    addTlv(MANAGEMENT, m1);
    uint8_t m2[4] = {0x20, 7, 7};  // DOMAIN domainNumber = 7
    addTlv(MANAGEMENT, m2);
    return setBuf(buf, curLen);
}
size_t get2MngTlvsSig(void *buf, size_t sizeBuf)
{
    if(buf != nullptr && sizeBuf > 0)
        return get2MngTlvsSigIntern((uint8_t *)buf, sizeBuf);
    return 0;
}
