/* SPDX-License-Identifier: LGPL-3.0-or-later
   SPDX-FileCopyrightText: Copyright © 2021 Erez Geva <ErezGeva2@gmail.com> */

/** @file
 * @brief Buffer for send, receive, build, and parse
 *
 * @author Erez Geva <ErezGeva2@@gmail.com>
 * @copyright © 2021 Erez Geva
 *
 */

#ifndef __PTPMGMT_BUF_H
#define __PTPMGMT_BUF_H

#ifdef __cplusplus
#include "name.h"

__PTPMGMT_NAMESPACE_BEGIN

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
    Buf(size_t size): m_buf(nullptr), m_size(0) {alloc(size);}
    ~Buf();
    /**
     * Create a new memory buffer or resize existing one
     * @param[in] size buffer new size
     * @return true if new buffer allocation success
     */
    bool alloc(size_t size);
    /**
     * Get memory buffer pointer
     * @return memory buffer pointer or null if buffer does not exist
     */
    void *get() const {return m_buf;}
    /**
     * Get memory buffer pointer
     * @return memory buffer pointer or null if buffer does not exist
     */
    void *operator()() const {return m_buf;}
    /**
     * Get memory buffer size
     * @return memory buffer size or 0 if buffer does not exist
     */
    size_t size() const {return m_size;}
    /**
     * Is memory buffer allocated?
     * @return true if memory buffer is allocated
     */
    bool isAlloc() const {return m_size > 0;}
};

__PTPMGMT_NAMESPACE_END
#else /* __cplusplus */
#error "this header is for C++ and scripts use only, NOT for C"
#endif /* __cplusplus */

#endif /* __PTPMGMT_BUF_H */
