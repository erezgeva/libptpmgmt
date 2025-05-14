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
#include <cstdint>
#include <string>
#include <functional>

#define MAX_BUFFER_LENGTH   (4096)

__CLKMGR_NAMESPACE_BEGIN

typedef std::array<uint8_t, MAX_BUFFER_LENGTH> TransportBuffer;
class Message;
class Transport;
class TransportWorkerState;

class TransportContext
{
  protected:
    bool _init;
    size_t offset;
    TransportBuffer buffer;
  public:
    bool init() { return _init; }
    TransportContext() : _init(true), offset(0) {}
    virtual ~TransportContext() = default;

    const size_t &getc_offset() { return offset; }
    size_t &get_offset() { return offset; }
    void set_offset(const size_t &offset) { this->offset = offset; }
    size_t c_get_val_offset() const { return offset; }

    const TransportBuffer &getc_buffer() { return buffer; }
    TransportBuffer &get_buffer() { return buffer; }
    void set_buffer(const TransportBuffer &buffer) { this->buffer = buffer; }
    TransportBuffer c_get_val_buffer() const { return buffer; }

    void resetOffset() { set_offset(0); }
    void addOffset(size_t offset) { this->offset += offset; }
};

class TransportTransmitterContext : public TransportContext
{
  public:
    virtual ~TransportTransmitterContext() = default;
    virtual bool sendBuffer() = 0;
};

class TransportListenerContext : public TransportContext
{
    friend class Transport;
    friend class TransportWorkerState;
  protected:

    virtual bool processMessage(Message *bmsg,
        TransportTransmitterContext *&txcontext) { return false; }
  public:
    virtual ~TransportListenerContext() = default;

    virtual TransportTransmitterContext *CreateTransmitterContext(
        TransportClientId &clientId) {
        return nullptr;
    }
};

typedef std::function<bool(TransportListenerContext *)> TransportWorkFunction;
typedef ptrdiff_t TransportWorkDesc;
const TransportWorkDesc InvalidTransportWorkDesc = (TransportWorkDesc)(-1);

class Transport
{
  public:
    static bool processMessage(TransportListenerContext &context);
    static bool initTransport() { return true; }
    static bool stopTransport() { return true; }
    static bool finalizeTransport() { return true; }
    static TransportWorkDesc registerWork(TransportWorkFunction func,
        TransportListenerContext *context);
    static bool init();
    static bool stop();
    static bool finalize();
    static bool InterruptWorker(TransportWorkDesc d);
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
