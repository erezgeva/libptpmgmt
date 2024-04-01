#include <client/client_state.hpp>
#include <common/jcltypes.hpp>

using namespace JClkLibClient;

ClientState::ClientState()
{
	connected = false;
	sessionId = JClkLibCommon::InvalidSessionId;
}

bool ClientState::get_connected() {return connected;}

void ClientState::set_connected(bool new_state) {connected = new_state;}

ClientState JClkLibClient::state{};
