<!-- SPDX-License-Identifier: GFDL-1.3-no-invariants-or-later
     SPDX-FileCopyrightText: Copyright © 2024 Intel Corporation. -->
# JCLKLIB codeflow

Jclklib is a 2-part implementation for C/C++ application to obtain
ptp4l events via pub/sub method using a library api call.

It provides a library jcklib.so (aka client-runtime library) and a daemon
jclklib_proxy.

In short we have :
          FRONT-END           MIDDLE-PIPE               BACK-END
c/c++ app<-------->jclklib.so<---------->jclklib_proxy<-------->ptp4l
        (library call)       (using msq)      |        (via UDS)
                                              |
                                        libptpmgmt.so

**if the diagram is not lisible, please change to text mode.**

* **c++ app** - Customer application that will be linking to jclklib.so library. Header file and sample test cpp file will be provided.

* **jclklib.so** - A dynamic library that provides a set of APIs to customer application : connect/disconnect, subscribe to proxy which in turn connect to ptp4l service.
This library is written in C++. It will have a permissive licence. It communicates with jclkib_proxy using message queues.
This library is also referred to as client-runtime library.

* **jclklib_proxy** - A daemon that is using libptpmgmt api to establish connection and subscribe towards ptp4l events. The communication is established using ptp4l UDS (/var/run/ptp4l as we now assumed it is always installed in the same local machine as the ptp4l)

## Compilation
Currently, we need to run 'make' in client, proxy folder.
It will definitely change in the future. Stay tuned.

## Connect message info

### Code Example : client/test.cpp , client/run_test.sh
Please adjust the LD_LIBRARY_PATH accordingly to include the correct jclklib.so
The placement will be changed in the future.

### Connect message flow (simplified code walkthrough)

Scenario : client application uses the jcklkib.so API to call `connect()`.
This will establish connection to the proxy.

** Client point of view **

1. **client/init.cpp/JClkLibClient::connect()**
        - Creation of new ClientConnectMessage object for CONNECT_MSG

1.1. **client/message.cpp/ClientMessage::init()**
        - We have this recursive template.
          `"template JClkLibCommon::_initMessage<ClientNullMessage,ClientConnectMessage>()`
          This will execute the `ClientConnectMessage::initMessage()`

          1.1.1 **client/connect_msg.cpp/ClientConnectMessage::initMessage()**
          - This will add the pair of <CONNECT_MSG , buildMessage function>
          - When an object of the type of ClientConnectMessage is received
          by the client transporter layer, it will know how to translate the object
          to messagequeue buffer.

1.2. **client/init.cpp/ClientTransport::init()**
        - Same concept as above but using initTransport recursive template.
        This will execute the `ClientMessageQueue::initTransport()`

        1.2.1 **client/msgq_tport.cpp/initTransport()**
        - Creation of listening and transmitting message queue.
        - Creation of **ClientMessageQueueListenerContext** and
        **ClientMessageQueueTransmitterContext**

        For each Listener and Transmitter context a specific message queue is defined.

        - Start *MqListenerWork* listener thread to listen to proxy reply.

1.3. **client/msgq_tport.cpp/ClientMessageQueue::writeTransportClientId(connectMsg.get());**
- Write the Client Listener msgq as the Transport Client ID.
This will allow the proxy to later connect to the client listener msgq to send ack msg.

1.4. **ClientMessageQueue::sendMessage(connectMsg.get());**
- Send the connect message to proxy. The object will be passed to transport layer.
Transport will then transform the client connect message object to buffer that
will then be sent thru the client transmitter msgq.

NOTE: Acknowledgement from proxy is the echo of the same message with ACK field added.
1.5 **client/connect_msg.cpp/ClientConnectMessage::processMessage**
- Client Transport layer will call this function to process from ClientConnectMessage reply received.
It is identified based on the unique msgID for CONNECT_MSG.
Currently we only printout the sessionID given by the proxy.

**[TODO]**: Return the sessionID to user calling the api connect.

** Proxy point of view **
Jclklib_proxy runs like a service daemon waiting for messages from potential client app.
The main loop function is in main.cpp.

How to run : LD_CONFIG_PATH=<path to libptpmgmt.so> jcklib_proxy -lptpmgmt &

1. **proxy/transport.cpp/ProxyTransport::init()**
- Same as for Client, the ProxyTransport layer is initialized.
- Set up listening message queue : `proxy/msg_tport.cpp/ProxyMessageQueue::initTransport()`

1. **proxy/message.cpp/ProxyMessage::init()**
- Same as for Client, the ProxyMessage layer is initialized.
This will call the `ProxyConnectMessage::initMessage()`

1.1 **proxy/connect_message.cpp/ProxyConnectMessage::initMessage()**
- Initialize the map with the first pair being CONNECT_MSG and its proxy buildMessage function.

Upon receiving the CONNECT_MSG in the proxy listener message queue (msgq) buffer, it will be received by `common/Transport::processMessage()`
which then trigger the `ProxyConnectMessage::buildMessage`.
And it will the message to `ProxyConnectMessage:processMessage`.

2. **proxy/connect_message.cpp/ProxyConnectMessage:processMessage**
- A new client session is created. A transmitter context is created for the client.
A transmitter context will have a dedicated tx msgq buffer for the client.
- The msg_ack is put to ACK_SUCCESS. This will then be taken back in
common/Transport::processMessage() and the echo reply be sent back to client.

## Subscription message info
To be added soon.

## Notification message info
To be added soon.
