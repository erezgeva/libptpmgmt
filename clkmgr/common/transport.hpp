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

#include <cstdint>
#include <functional>
#include <future>
#include <thread>
#include <vector>

#define MAX_BUFFER_LENGTH   (4096)

__CLKMGR_NAMESPACE_BEGIN

typedef std::array<uint8_t, MAX_BUFFER_LENGTH> TransportBuffer;
class Message;
class Transport;

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
#define SEND_BUFFER_TYPE(name)                \
    bool name()
    virtual SEND_BUFFER_TYPE(sendBuffer) = 0;
};

class TransportListenerContext : public TransportContext
{
    friend class Transport;
  protected:
#define LISTENER_CONTEXT_PROCESS_MESSAGE_TYPE(name)                    \
    bool name(Message *bmsg, TransportTransmitterContext *&txcontext)
    virtual LISTENER_CONTEXT_PROCESS_MESSAGE_TYPE(processMessage) { return false; }
  public:
    virtual ~TransportListenerContext() = default;
#define CREATE_TRANSMIT_CONTEXT_TYPE(name)                            \
    TransportTransmitterContext *name \
    (TransportClientId &clientId)
    virtual CREATE_TRANSMIT_CONTEXT_TYPE(CreateTransmitterContext) {
        return nullptr;
    }
};

class Transport
{
  public:
    typedef std::unique_ptr<TransportListenerContext> TransportWorkArg;
    typedef std::function<bool(TransportWorkArg::element_type *)>
    TransportWorkFunction;
    typedef std::pair<TransportWorkFunction, TransportWorkArg> TransportWork;
    typedef ptrdiff_t TransportWorkDesc;
    static const TransportWorkDesc InvalidTransportWorkDesc =
        (TransportWorkDesc)(-1);
  private:
    static void dispatchLoop(std::promise<bool>,
        std::shared_ptr<std::atomic<bool>> exitVal, TransportWork arg);
  public:
    static bool processMessage(TransportListenerContext &context);
    static bool initTransport() { return true; }
    static bool stopTransport() { return true; }
    static bool finalizeTransport() { return true; }
    static TransportWorkDesc registerWork(TransportWork work);
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
