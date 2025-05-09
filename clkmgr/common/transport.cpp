/* SPDX-License-Identifier: BSD-3-Clause
   SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. */

/** @file
 * @brief Common transport base class implementation for Clkmgr.
 * It provides common functionality for derived transport classes.
 *
 * @author Christopher Hall <christopher.s.hall@@intel.com>
 * @copyright © 2024 Intel Corporation.
 *
 */

#include "common/transport.hpp"
#include "common/msgq_tport.hpp"
#include "common/null_tport.hpp"
#include "common/sighandler.hpp"
#include "common/print.hpp"

#include <future>
#include <vector>

#define EXIT_TIMEOUT    (200 /*ms*/)
#if 0
#define START_TIMEOUT   (20 /*ms*/)
#endif

using namespace std;

__CLKMGR_NAMESPACE_BEGIN;

class TransportWorkerState
{
  private:
    promise<bool> m_promise;
    future<bool> m_retVal;
    TransportWorkFunction m_func; // Callback
    unique_ptr<TransportListenerContext> m_arg;
    atomic_bool m_exitVal;
    thread m_thread;
  public:
    TransportWorkerState(const TransportWorkFunction &func,
        TransportListenerContext *context) :
        m_retVal(m_promise.get_future()),
        m_func(func),
        m_arg(context),
        m_exitVal(false) {}
    bool init();
    void dispatchLoop();
    bool finalize();
    bool isFutureSet();
    void stopSignal() { m_exitVal.store(true); }
    thread &getThread() { return m_thread; }
};

__CLKMGR_NAMESPACE_END;

__CLKMGR_NAMESPACE_USE;

static vector<unique_ptr<TransportWorkerState>> workerList;

bool TransportWorkerState::isFutureSet()
{
    return m_retVal.valid() &&
        m_retVal.wait_for(chrono::milliseconds(0)) == future_status::ready;
}

void TransportWorkerState::dispatchLoop()
{
    bool promiseVal = false;
    PrintDebug("Transport Thread started");
    while(!m_exitVal.load()) {
        if(!m_func(m_arg.get()))
            goto done;
        // Set TransportListenerContext::_init
        m_arg.get()->_init = false;
    }
    promiseVal = true;
done:
    PrintDebug("Transport Thread exited");
    m_promise.set_value_at_thread_exit(promiseVal);
    return;
}

static void staticDispatchLoop(TransportWorkerState *me)
{
    me->dispatchLoop();
}

bool TransportWorkerState::init()
{
    m_thread = thread(staticDispatchLoop, this);
    PrintDebug("Thread started");
    if(isFutureSet()) {
        m_thread.join();
        PrintError("Thread exited early");
        return false;
    }
    return true;
}

bool TransportWorkerState::finalize()
{
    if(m_retVal.wait_for(chrono::milliseconds(EXIT_TIMEOUT)) !=
        future_status::ready) {
        PrintError("Thread Join Timeout");
        return false;
    }
    m_thread.join();
    return true;
}

TransportWorkDesc Transport::registerWork(const TransportWorkFunction &func,
    TransportListenerContext *context)
{
    if(context == nullptr) {
        PrintError("Transport::registerWork null context");
        return InvalidTransportWorkDesc;
    }
    TransportWorkerState *row = new TransportWorkerState(func, context);
    if(row != nullptr) {
        if(row->init()) {
            workerList.push_back(unique_ptr<TransportWorkerState>(row));
            // We add at the end, so the index is the last element
            return workerList.size() - 1;
        }
        PrintError("TransportWorkerState::init fail");
        delete row;
    } else
        PrintError("Transport::registerWork fail allocating");
    delete context;
    return InvalidTransportWorkDesc;
}

bool Transport::processMessage(TransportListenerContext &context)
{
    Message *msg;
    TransportTransmitterContext *txcontext;
    context.resetOffset();
    if(!Message::buildMessage(msg, context))
        return false;
    PrintDebug("Received message " + msg->toString());
    if(!context.processMessage(msg, txcontext))
        return false;
    /* Echo the message back with ACK disposition */
    if(msg->get_msgAck() != ACK_NONE)
        return msg->transmitMessage(*txcontext);
    return true;
}

bool Transport::init()
{
    return _initTransport<NullTransport, MessageQueue>();
}

bool Transport::stop()
{
    /* Send stop signal to all of the threads */
    for(auto &it : workerList)
        it->stopSignal();
    /* Do any transport specific stop */
    if(!_stopTransport<NullTransport, MessageQueue>())
        return false;
    return true;
}

bool Transport::finalize()
{
    for(auto &it : workerList) {
        if(!it->finalize())
            return false;
    }
    return _finalizeTransport<NullTransport, MessageQueue>();
}

bool Transport::InterruptWorker(TransportWorkDesc d)
{
    if(d == InvalidTransportWorkDesc)
        return false;
    /* Thread has exited, no need to interrupt */
    if(workerList.back()->isFutureSet())
        return true;
    PrintDebug("Sending interrupt to MessageQueue worker");
    return SendSyscallInterruptSignal(workerList[d]->getThread());
}
