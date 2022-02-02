/* SPDX-License-Identifier: LGPL-3.0-or-later */

/** @file
 * @brief endian functions
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 * @details
 *  provide functions to convert unsigned integers from
 *  host byte order to network byte order
 */

#ifndef __PMC_ENDIAN_H
#define __PMC_ENDIAN_H

#include <cstdint>
#include <endian.h>

/**
 * convert unsigned 16 bits integer from host order to network order
 * @param[in] value host order unsigned 16 bits integer
 * @return network order unsigned 16 bits integer
 */
inline uint16_t cpu_to_net16(uint16_t value) {return htobe16(value);}
/**
 * convert unsigned 16 bits integer from network order to host order
 * @param[in] value network order unsigned 16 bits integer
 * @return host order unsigned 16 bits integer
 */
inline uint16_t net_to_cpu16(uint16_t value) {return be16toh(value);}
/**
 * convert unsigned 32 bits integer from host order to network order
 * @param[in] value host order unsigned 32 bits integer
 * @return network order unsigned 32 bits integer
 */
inline uint32_t cpu_to_net32(uint32_t value) {return htobe32(value);}
/**
 * convert unsigned 32 bits integer from network order to host order
 * @param[in] value network order unsigned 32 bits integer
 * @return host order unsigned 32 bits integer
 */
inline uint32_t net_to_cpu32(uint32_t value) {return be32toh(value);}
/**
 * convert unsigned 64 bits integer from host order to network order
 * @param[in] value host order unsigned 64 bits integer
 * @return network order unsigned 64 bits integer
 */
inline uint64_t cpu_to_net64(uint64_t value) {return htobe64(value);}
/**
 * convert unsigned 64 bits integer from network order to host order
 * @param[in] value network order unsigned 64 bits integer
 * @return host order unsigned 64 bits integer
 */
inline uint64_t net_to_cpu64(uint64_t value) {return be64toh(value);}

/**
 * convert unsigned 64 bits integer from host order to little endian order
 * @param[in] value host order unsigned 64 bits integer
 * @return little endian order unsigned 64 bits integer
 * @note used for linuxptp statistics
 */
inline uint64_t cpu_to_le64(uint64_t value) {return htole64(value);}
/**
 * convert unsigned 64 bits integer from little endian order to host order
 * @param[in] value little endian order unsigned 64 bits integer
 * @return host order unsigned 64 bits integer
 * @note used for linuxptp statistics
 */
inline uint64_t le_to_cpu64(uint64_t value) {return le64toh(value);}

#endif /* __PMC_ENDIAN_H */
