/* SPDX-License-Identifier: LGPL-3.0-or-later */

/** @file
 * @brief allocate buffer
 *
 * @author Erez Geva <ErezGeva2@gmail.com>
 * @copyright 2021 Erez Geva
 *
 */

#include "buf.h"

void buffer::del()
{
    if(m_buf != nullptr) {
        free(m_buf);
        m_buf = nullptr;
        m_size = 0;
    }
}
bool buffer::alloc(size_t size)
{
    if(size == 0)
        return false;
    if(size <= m_size)
        return true;
    void *buf = realloc(m_buf, size);
    if(buf == nullptr)
        return false;
    m_buf = buf;
    m_size = size;
    return true;
}
