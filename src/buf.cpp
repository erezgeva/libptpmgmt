/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Buffer for send, receive, build, and parse
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 */

#include "buf.h"

__PTPMGMT_NAMESPACE_BEGIN

Buf::Buf(size_t size) { alloc(size); }
Buf::~Buf() { free(m_buf); }
bool Buf::alloc(size_t size)
{
    if(size == 0)
        return false;
    if(size <= m_size)
        return true;
    void *l_buf = realloc(m_buf, size);
    if(l_buf == nullptr)
        return false;
    m_buf = l_buf;
    m_size = size;
    return true;
}
void *Buf::get() const { return m_buf; }
size_t Buf::size() const { return m_size; }
bool Buf::isAlloc() const { return m_size > 0; }

__PTPMGMT_NAMESPACE_END
