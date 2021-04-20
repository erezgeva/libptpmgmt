/* SPDX-License-Identifier: LGPL-3.0-or-later */

/** @file
 * @brief buffer for send, receive, build, and parse
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright 2021 Erez Geva
 * @copyright GNU Lesser General Public License 3.0 or later
 *
 */

#ifndef __PMC_BUF_H
#define __PMC_BUF_H

#include <cstdint>
#include <cstddef>

/**
 * Create memory buffer for use in send, receive, build and parse
 * @note This class is for scripts language.
 *  C++ code can allocate the buffer directly.
 */
class Buf
{
  private:
    void *m_buf;
    size_t m_size;
  public:
    Buf(): m_buf(nullptr), m_size(0) {}
    /**
     * Create a new memory buffer
     * @param[in] size new buffer size
     */
    Buf(const size_t size): m_buf(nullptr), m_size(0) {alloc(size);}
    ~Buf();
    /**
     * Create a new memory buffer or resize existing one
     * @param[in] size buffer new size
     * @return true if new buffer allocation success
     */
    bool alloc(const size_t size);
    /**
     * Get memory buffer pointer
     * @return memory buffer pointer or null if buffer does not exist
     */
    void *get() {return m_buf;}
    /**
     * Get memory buffer pointer
     * @return memory buffer pointer or null if buffer does not exist
     */
    void *operator()() {return m_buf;}
    /**
     * Get memory buffer size
     * @return memory buffer size or 0 if buffer does not exist
     */
    size_t size() const {return m_size;}
    /**
     * Is memory buffer allocated?
     * @return true if memory buffer is allocated
     */
    bool isAlloc() const {return m_size > 0 && m_buf != nullptr;}
};

#endif /*__PMC_BUF_H*/
