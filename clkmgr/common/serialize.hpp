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

#define PARSE_RX(var, buff) ({                                             \
        size_t var_len = sizeof var;                                       \
        bool can_parse = buff.lenLeft() >= var_len;                        \
        if (can_parse) {                                                   \
            var = *(remove_reference<decltype(var)>::type *)buff.dataOff();\
            buff.addOffset(var_len);                                       \
        }                                                                  \
        can_parse; })

#define WRITE_TX(var, buff) ({                                             \
        size_t var_len = sizeof var;                                       \
        bool can_write = buff.sizeLeft() >= var_len;                       \
        if (can_write) {                                                   \
            *(remove_reference<decltype(var)>::type *)buff.dataOff() = var;\
            buff.addOffset(var_len);                                       \
        }                                                                  \
        can_write; })

#endif /* SERIALIZE_HPP */
