#include <client/client_state.hpp>

using namespace JClkLibClient;

ClientState::ClientState()
{
	connected = false;
}

ClientState JClkLibClient::state{};
