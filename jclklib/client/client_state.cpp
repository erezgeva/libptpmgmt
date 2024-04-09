#include <client/msgq_tport.hpp>
#include <client/client_state.hpp>
#include <common/jcltypes.hpp>
#include <common/transport.hpp>
#include <iostream>
#include <cstring>

using namespace std;
using namespace JClkLibClient;
using namespace JClkLibCommon;

ClientState::ClientState()
{
	connected = false;
	sessionId = JClkLibCommon::InvalidSessionId;
	fill(begin(clientID ), end( clientID ), 0 );
}

bool ClientState::get_connected() {return connected;}

void ClientState::set_connected(bool new_state) {connected = new_state;}

TransportClientId ClientState::get_clientID() {
	return clientID;
}

void ClientState::set_clientID(TransportClientId new_cID) {
	strcpy((char *)clientID.data(), (char *)new_cID.data());
}

ClientState JClkLibClient::state{};
