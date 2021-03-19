/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* end.h endian functions
 *
 * Authors: Erez Geva <ErezGeva2@gmail.com>
 *
 * Created following "IEEE Std 1588-2008", PTP version 2
 */

#ifndef __PMC_ENDIAN_H
#define __PMC_ENDIAN_H

#include <stdint.h>
#include <endian.h>

inline uint16_t hton16(uint16_t val) {return htobe16(val);} // = htons
inline uint16_t ntoh16(uint16_t val) {return be16toh(val);} // = ntohs
inline uint32_t hton32(uint32_t val) {return htobe32(val);} // = htonl
inline uint32_t ntoh32(uint32_t val) {return be32toh(val);} // = ntohl
inline uint64_t hton64(uint64_t val) {return htobe64(val);}
inline uint64_t ntoh64(uint64_t val) {return be64toh(val);}

#endif /*__PMC_ENDIAN_H*/
