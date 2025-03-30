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

#include "common/message.hpp"
#include "common/msgq_tport.hpp"
#include "common/null_tport.hpp"
#include "common/print.hpp"
#include "common/sighandler.hpp"
#include "common/transport.hpp"
#include "common/util.hpp"

#define EXIT_TIMEOUT    (200 /*ms*/)
#define START_TIMEOUT   (20 /*ms*/)

__CLKMGR_NAMESPACE_USE;

using namespace std;

class TransportWorkerState
{
  public:
    future<bool> retVal;
    shared_ptr<atomic<bool>> exitVal;
    unique_ptr<thread> _thread;
    TransportWorkerState(future<bool> retInit, bool exitInit) :
        retVal(std::move(retInit)), exitVal(make_shared<atomic<bool>>(exitInit)) {}
};
static vector<TransportWorkerState> workerList;

bool isFutureSet(future<bool> &f)
{
    return f.valid() &&
        f.wait_for(chrono::milliseconds(0)) == future_status::ready;
}
void Transport::dispatchLoop(promise<bool> _promise,
    shared_ptr<atomic<bool>> exitVal, TransportWork work)
{
    bool promiseVal = false;
    PrintDebug("Transport Thread started");
    for(; !exitVal->load();) {
        if(!work.first(work.second.get()))
            goto done;
        work.second.get()->_init = false;
    }
    promiseVal = true;
done:
    PrintDebug("Transport Thread exited");
    _promise.set_value_at_thread_exit(promiseVal);
    return;
}

Transport::TransportWorkDesc Transport::registerWork(TransportWork work)
{
    promise<bool> _promise;
    workerList.push_back(TransportWorkerState(_promise.get_future(), false));
    workerList.back()._thread = unique_ptr<thread>(
            new thread(MessageQueue::dispatchLoop, std::move(_promise),
                workerList.back().exitVal,
                TransportWork(work.first, std::move(work.second))));
    PrintDebug("Thread started");
    if(isFutureSet(workerList.back().retVal)) {
        workerList.back()._thread.get()->join();
        workerList.pop_back();
        PrintError("Thread exited early");
        return InvalidTransportWorkDesc;
    }
    /* Return the index of the element we just added */
    return (workerList.cend() - 1) - workerList.cbegin();
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
    for(const auto &it : workerList)
        it.exitVal->store(true);
    /* Do any transport specific stop */
    if(!_stopTransport<NullTransport, MessageQueue>())
        return false;
    return true;
}

bool Transport::finalize()
{
    for(auto &it : workerList) {
        if(it.retVal.wait_for(chrono::milliseconds(EXIT_TIMEOUT)) !=
            future_status::ready) {
            PrintError("Thread Join Timeout");
            return false;
        }
        it._thread.get()->join();
    }
    return _finalizeTransport<NullTransport, MessageQueue>();
}

bool Transport::InterruptWorker(TransportWorkDesc d)
{
    if(d == InvalidTransportWorkDesc)
        return false;
    /* Thread has exited, no need to interrupt */
    if(isFutureSet(workerList.back().retVal))
        return true;
    PrintDebug("Sending interrupt to MessageQueue worker");
    return SendSyscallInterruptSignal(*workerList[d]._thread.get());
}
