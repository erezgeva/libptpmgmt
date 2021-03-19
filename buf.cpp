/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* buf.cpp allocate buffer
 *
 * Authors: Erez Geva <ErezGeva2@gmail.com>
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
    if(size <= 0)
        return false;
    if(size <= m_size)
        return true;
    del();
    void *buf = malloc(size);
    if(buf == nullptr)
        return false;
    m_buf = buf;
    m_size = size;
    return true;
}
