/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Utility macros for (de-)serializing data
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef SERIALIZE_HPP
#define SERIALIZE_HPP

#include <cstring>

#define PARSE_RX(type,var,lc) ({                                            \
        ssize_t offset = PARSE_##type(var,lc.dataOff(), lc.lenLeft());      \
        if (offset > 0)                                                     \
            lc.addOffset(offset);                                           \
        offset >= 0; })

#define PARSE_FIELD(var,ptr,len) ({                                         \
        size_t var_len = sizeof var;                                        \
        bool can_parse = len >= var_len;                                    \
        if (can_parse)                                                      \
            var = *(remove_reference<decltype(var)>::type *)(ptr);          \
        can_parse ? var_len : -1; })

#define WRITE_TX(type,var,tc) ({                                            \
        ssize_t offset = WRITE_##type(var,tc.dataOff(), tc.sizeLeft());     \
        if (offset > 0)                                                     \
            tc.addOffset(offset);                                           \
        offset >= 0; })

#define WRITE_FIELD(var,ptr,len) ({                                         \
        size_t var_len = sizeof var;                                        \
        bool can_write = len >= var_len;                                    \
        if (can_write)                                                      \
            *(remove_reference<decltype(var)>::type *)(ptr) = var;          \
        can_write ? var_len : -1; })

#endif /* SERIALIZE_HPP */
