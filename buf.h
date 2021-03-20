/* SPDX-License-Identifier: LGPL-3.0-or-later */

/** @file
 * @brief allocate buffer
 *
 * @author Erez Geva <ErezGeva2@gmail.com>
 * @copyright 2021 Erez Geva
 *
 * @details
 *  Provide class to allocate memory buffer for a message object.
 *  The class is mostly usefull in scripts language like Perl.
 *  As C++ application can simple allocate the buffer.
 */

#ifndef __BUF_H
#define __BUF_H

#include <string>
#include <stdint.h>

/**
 * @brief hold a buffer
 * @details
 *  Allocate a buffer to be used with a message object.
 */
class buffer
{
  private:
    void *m_buf;
    size_t m_size;
    void del();

  public:
    buffer() : m_buf(nullptr), m_size(0) {}
    ~buffer() { del(); }
    /**
     * Get buffer pointer
     * @return a pointer to buffer
     */
    void *getBuf() { return m_buf; }
    /**
     * Get buffer size
     * @return the buffer size
     */
    size_t getSize() { return m_size; }
    /**
     * Allocate or enlarge the buffer
     * @param[in] size allocated buffer size
     * @return true on buffer allocation success
     */
    bool alloc(size_t size);
};

/**
 * @brief hold a binary octets
 * @note so std::string will hold only readable strings
 */
class binary
{
  private:
    std::string str;
  public:
    /**
     * Copy constructor
     * @param[in] other object
     */
    binary(binary &other) { str = other.str; }
    /**
     * Constructor from buffer
     * @param[in] buf pointer to binary octets
     * @param[in] length
     */
    binary(uint8_t *buf, size_t length) {str = std::string((char *)buf, length);}
    /**
     * Get binary length
     * @return binary length
     */
    size_t length() {return str.length();}
    /**
     * Get pointer to binary
     * @return binary pointer
     */
    const uint8_t *get() {return (const uint8_t *)str.c_str();}
};

#endif /*__BUF_H*/
