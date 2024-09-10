/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file
 * @brief Utility macros for (de-)serializing data
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef SERIALIZE_HPP
#define SERIALIZE_HPP

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <type_traits>

#include <common/transport.hpp>

#define PARSE_RX(type,var,lc)                       \
    ({                              \
        decltype(lc.getc_offset()) offset = \
            PARSE_##type(var,lc.getc_buffer().data() + lc.getc_offset(), \
                lc.getc_buffer().max_size() - lc.getc_offset()); \
        if (offset != (decltype(offset))-1) {               \
            lc.addOffset(offset);                   \
        }                               \
        (offset != (decltype(offset))-1 ? true : false);        \
    })

#define PARSE_FIELD(var,ptr,len) \
    _PARSE(std::remove_reference<decltype(var)>::type,var,ptr,len)
#define _PARSE(var_type,var,ptr,len)                    \
    ({                              \
        std::size_t var_len = sizeof(var);          \
        bool can_parse = len >= var_len;            \
        if (can_parse) var = *(var_type *)(ptr); \
        (can_parse ? var_len : (std::remove_reference<decltype(len)>::type)(-1)); \
    })

#define PARSE_ARRAY(arr,ptr,len)                    \
    ({                              \
        std::size_t var_len = arr.max_size()*sizeof(decltype(arr)::value_type); \
        bool can_parse = len >= var_len;            \
        if (can_parse) memcpy(arr.data(), ptr, var_len);    \
        (can_parse ? var_len : (std::remove_reference<decltype(len)>::type)(-1)); \
    })

#define CHECK_RSV(field)                        \
    (field == (std::remove_reference<decltype(field)>::type)0)

#define WRITE_TX(type,var,tc)                   \
    ({                                  \
        decltype(tc.getc_offset()) offset = \
            WRITE_##type(var,tc.get_buffer().data() + tc.get_offset(), \
                tc.get_buffer().max_size() - tc.get_offset()); \
        if (offset != (decltype(offset))-1) {               \
            tc.addOffset(offset);                   \
        }                               \
        (offset != (decltype(offset))-1 ? true : false);        \
    })

#define WRITE_FIELD(var,ptr,len) \
    _WRITE(std::remove_reference<decltype(var)>::type,var,ptr,len)
#define _WRITE(var_type,var,ptr,len)                    \
    ({                              \
        std::size_t var_len = sizeof(var);          \
        bool can_parse = (len) >= var_len;          \
        if (can_parse) *(var_type *)(ptr) = var; \
        (can_parse ? var_len : (std::remove_reference<decltype(len)>::type)(-1)); \
    })

/* For writing std::array */
#define WRITE_ARRAY(arr,ptr,len)                    \
    ({                              \
        std::size_t var_len = arr.max_size()*sizeof(decltype(arr)::value_type); \
        bool can_parse = len >= var_len;            \
        if (can_parse) memcpy(ptr, arr.data(), var_len);    \
        (can_parse ? var_len : (std::remove_reference<decltype(len)>::type)(-1)); \
    })

#endif /* SERIALIZE_HPP */
