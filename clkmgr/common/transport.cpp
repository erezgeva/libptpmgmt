/*
 * SPDX-License-Identifier: BSD-3-Clause
 * SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation.
 */

/** @file transport.cpp
 * @brief Common transport base class implementation for Clkmgr.
 * It provides common functionality for derived transport classes.
 *
 * @author Christopher Hall <christopher.s.hall@intel.com>
 * @copyright © 2024 Intel Corporation.
 * @license BSD-3-Clause
 *
 */

#include <common/message.hpp>
#include <common/msgq_tport.hpp>
#include <common/null_tport.hpp>
#include <common/print.hpp>
#include <common/sighandler.hpp>
#include <common/transport.hpp>
#include <common/util.hpp>

#define EXIT_TIMEOUT    (200 /*ms*/)
#define START_TIMEOUT   (20 /*ms*/)

__CLKMGR_NAMESPACE_USE

using namespace std;

DECLARE_STATIC(Transport::workerList);

void Transport::dispatchLoop(
    promise<FUTURE_TYPEOF(TransportWorkerState::retVal)> promise,
    shared_ptr<atomic<bool>> exitVal, TransportWork work)
{
    FUTURE_TYPEOF(TransportWorkerState::retVal) promiseVal = false;
    PrintDebug("Transport Thread started");
    for(; !exitVal->load();) {
        if(!work.first(work.second.get()))
            goto done;
        work.second.get()->_init = false;
    }
    promiseVal = true;
done:
    PrintDebug("Transport Thread exited");
    promise.set_value_at_thread_exit(promiseVal);
    return;
}

Transport::TransportWorkerState::TransportWorkerState(future<bool> retInit,
    bool exitInit)
{
    exitVal = make_shared<atomic<bool>>(exitInit);
    retVal = move(retInit);
    thread = nullptr;
}

Transport::TransportWorkDesc Transport::registerWork(TransportWork work)
{
    promise<FUTURE_TYPEOF(TransportWorkerState::retVal)> promise;
    workerList.push_back(TransportWorkerState(promise.get_future(), false));
    workerList.back().thread = make_unique<thread>
        (MessageQueue::dispatchLoop, move(promise),
            workerList.back().exitVal,
            TransportWork(work.first, move(work.second)));
    PrintDebug("Thread started");
    if(isFutureSet<FUTURE_TYPEOF(TransportWorkerState::retVal)>
        (workerList.back().retVal)) {
        workerList.back().thread.get()->join();
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
    for(decltype(workerList)::iterator it = workerList.begin();
        it != workerList.end(); ++it)
        it->exitVal->store(true);
    /* Do any transport specific stop */
    if(!_stopTransport<NullTransport, MessageQueue>())
        return false;
    return true;
}

bool Transport::finalize()
{
    bool retVal = false;
    for(auto it = workerList.begin();
        it != workerList.end(); ++it) {
        if(it->retVal.wait_for(chrono::milliseconds(EXIT_TIMEOUT)) !=
            future_status::ready) {
            PrintError("Thread Join Timeout");
            goto done;
        }
        it->thread.get()->join();
        retVal &= it->retVal.get();
    }
    if(!_finalizeTransport<NullTransport, MessageQueue>())
        goto done;
    retVal = true;
done:
    return retVal;
}

bool Transport::InterruptWorker(TransportWorkDesc d)
{
    if(d == InvalidTransportWorkDesc)
        return false;
    /* Thread has exited, no need to interrupt */
    if(isFutureSet<FUTURE_TYPEOF(TransportWorkerState::retVal)>
        (workerList.back().retVal))
        return true;
    PrintDebug("Sending interrupt to MessageQueue worker");
    return SendSyscallInterruptSignal(*workerList[d].thread.get());
}
