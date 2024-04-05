/*! \file msgq_tport.cpp
    \brief Client POSIX message queue transport class.

    (C) Copyright Intel Corporation 2023. All rights reserved. Intel Confidential.
    Author: Christopher Hall <christopher.s.hall@intel.com>
*/

#include <client/msgq_tport.hpp>
#include <client/connect_msg.hpp>

#include <common/sighandler.hpp>
#include <common/util.hpp>
#include <common/print.hpp>

#include <thread>
#include <cstring>

#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>

#include <errno.h>

using namespace JClkLibClient;
using namespace JClkLibCommon;
using namespace std;

DECLARE_STATIC(ClientMessageQueue::mqListenerName,"");
DECLARE_STATIC(ClientMessageQueue::mqNativeClientTransmitterDesc,-1);
DECLARE_STATIC(ClientMessageQueue::txContext);

LISTENER_CONTEXT_PROCESS_MESSAGE_TYPE(ClientMessageQueueListenerContext::processMessage)
{
        ClientMessage *msg = dynamic_cast<decltype(msg)>(bmsg);

        PrintDebug("Processing received client message");
        if (msg == NULL) {
                PrintError("Wrong message type");
                return false;
        }

        return msg->processMessage(*this,txcontext);
}

bool ClientMessageQueue::initTransport()
{
	struct mq_attr mq_attr;

	mq_attr.mq_flags = 0;
	mq_attr.mq_maxmsg = 2; // Two outstanding messages per client
	mq_attr.mq_msgsize = (decltype(mq_attr.mq_msgsize)) std::tuple_size<TransportBuffer>::value;

	PrintInfo("Initializing Message Queue Client Transport...");
	mqListenerName += mqProxyName + ".";
	mqListenerName += to_string(getpid());
	mqNativeListenerDesc = mq_open(mqListenerName.c_str(), RX_QUEUE_FLAGS, RX_QUEUE_MODE, &mq_attr);
	if (mqNativeListenerDesc == -1) {
		PrintError("Failed to open listener queue");
		return false;
	}

	if (InvalidTransportWorkDesc ==
	    (mqListenerDesc = registerWork
	     (move((TransportWork){MqListenerWork,
				   TransportWorkArg(new ClientMessageQueueListenerContext(mqNativeListenerDesc))})) )) {
		PrintError("Listener Thread Unexpectedly Exited");
		return false;
	}

	mqNativeClientTransmitterDesc = mq_open(mqProxyName.c_str(), TX_QUEUE_FLAGS);
	if (mqNativeClientTransmitterDesc == -1) {
		PrintErrorCode("Failed to open transmitter queue: " + mqProxyName);
		return false;
	}
	txContext.reset(new ClientMessageQueueTransmitterContext(mqNativeClientTransmitterDesc));

	PrintDebug("Client Message queue opened");

	return true;
}

bool ClientMessageQueue::stopTransport()
{
	PrintInfo("Stopping Message Queue Client Transport");
	PrintDebug("mqListenerName = " + mqListenerName);
	if (mq_unlink(mqListenerName.c_str()) == -1)
		PrintErrorCode("unlink failed");
	if (mqListenerDesc != InvalidTransportWorkDesc && !InterruptWorker(mqListenerDesc))
		PrintError("Interrupt worker failed");

	return true;
}

bool ClientMessageQueue::finalizeTransport()
{
	PrintDebug("mqNativeListenerDesc = " + to_string(mqNativeListenerDesc));
	PrintDebug("mqNativeClientTransmitterDesc = " + to_string(mqNativeClientTransmitterDesc));
	return mq_close(mqNativeListenerDesc) != -1 && mq_close(mqNativeClientTransmitterDesc) != -1;
}

bool ClientMessageQueue::writeTransportClientId(Message *msg)
{
	ClientConnectMessage *cmsg = dynamic_cast<decltype(cmsg)>(msg);
	if (cmsg == NULL) {
		PrintErrorCode("[ClientMessageQueue] cmsg is NULL !!\n");
		return false;
	}
	strcpy((char *)cmsg->getClientId().data(), mqListenerName.c_str());
	printf ("[ClientMessageQueue] cmsg->getClientId().data(): %s", (char *)cmsg->getClientId().data());
	return true;
}

SEND_CLIENT_MESSAGE(ClientMessageQueue::sendMessage)
{
	auto context = txContext.get();

	msg->presendMessage(context);
	if (mq_send(mqNativeClientTransmitterDesc, (char *)context->getc_buffer().data(), context->getc_offset(), 0) == -1) {
		PrintErrorCode("Failed to transmit client message");
		return false;
	}
	PrintDebug("[Azu] ClientMessageQueue::sendMessage successful ");
	return true;
}
