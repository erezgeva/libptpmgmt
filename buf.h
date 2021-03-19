/* SPDX-License-Identifier: LGPL-3.0-or-later */

/* buf.h allocate buffer
 *
 * Authors: Erez Geva <ErezGeva2@gmail.com>
 *
 */

#ifndef __BUF_H
#define __BUF_H

#include <string>
#include <stdint.h>

class buffer
{
  private:
    void *m_buf;
    size_t m_size;
    void del();

  public:
    buffer() : m_buf(nullptr), m_size(0) {}
    ~buffer() { del(); }
    void *getBuf() { return m_buf; }
    size_t getSize() { return m_size; }
    bool alloc(size_t size);
};

#endif /*__BUF_H*/
