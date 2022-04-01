/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright 2021 Erez Geva */

/** @file
 * @brief buffer for send, receive, build, and parse
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#include <cstdlib>
#include "buf.h"

Buf::~Buf()
{
    if(m_buf != nullptr)
        free(m_buf);
}
bool Buf::alloc(size_t size)
{
    if(size == 0)
        return false;
    if(size <= m_size)
        return true;
    void *nb = realloc(m_buf, size);
    if(nb == nullptr)
        return false;
    m_buf = nb;
    m_size = size;
    return true;
}
