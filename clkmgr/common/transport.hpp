/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Common transport base class for Clkmgr.
 * It is extended for specific transports such as POSIX message queue.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#ifndef COMMON_TRANSPORT_HPP
#define COMMON_TRANSPORT_HPP

#include "common/util.hpp"
#include "common/message.hpp"

#include <cstddef>
#include <string>

#define MAX_BUFFER_LENGTH   (4096)

__CLKMGR_NAMESPACE_BEGIN

class TransportBuffer
{
  private:
    uint8_t buff[MAX_BUFFER_LENGTH];

  public:
    uint8_t *data() { return buff; }
    size_t max_size() { return MAX_BUFFER_LENGTH; }
};

class TransportContext
{
  private:
    size_t m_offset = 0;
    TransportBuffer m_buffer;

  public:
    TransportContext() = default;
    virtual ~TransportContext() = default;

    size_t get_offset() { return m_offset; }
    void set_offset(size_t offset) { m_offset = offset; }
    TransportBuffer &get_buffer() { return m_buffer; }

    void resetOffset() { m_offset = 0; }
    void addOffset(size_t offset) { m_offset += offset; }
};

class TransportTransmitterContext : public TransportContext
{
  public:
    virtual ~TransportTransmitterContext() = default;
    virtual bool sendBuffer() = 0;
};

class TransportListenerContext : public TransportContext
{
  public:
    virtual ~TransportListenerContext() = default;
};

class Transport
{
  public:
    static bool initTransport() { return true; }
    static bool stopTransport() { return true; }
    static bool finalizeTransport() { return true; }
    static bool init();
    static bool stop();
    static bool finalize();
};

#define PER_TRANSPORT_VARIADIC_TEMPLATE(x)                        \
    template <typename T>                                         \
    inline bool _##x()                                            \
    {                                                             \
        return T::x();                                            \
    }                                                             \
    template <typename T, typename... Types>                      \
    inline typename std::enable_if<sizeof...(Types) != 0, bool>::type    \
    _##x()                                                        \
    {                                                             \
        return _##x<T>() && _##x<Types...>();                     \
    }

PER_TRANSPORT_VARIADIC_TEMPLATE(initTransport)
PER_TRANSPORT_VARIADIC_TEMPLATE(stopTransport)
PER_TRANSPORT_VARIADIC_TEMPLATE(finalizeTransport)

__CLKMGR_NAMESPACE_END

#endif /* COMMON_TRANSPORT_HPP */
