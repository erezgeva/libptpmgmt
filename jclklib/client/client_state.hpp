#ifndef PROXY_CLIENT_STATE
#define PROXY_CLIENT_STATE

#include <common/jcltypes.hpp>
#include <common/util.hpp>
#include <common/transport.hpp>
#include <atomic>

namespace JClkLibClient {
	class ClientState {
	private:
		std::atomic_bool connected;
		JClkLibCommon::sessionId_t sessionId;
		JClkLibCommon::TransportClientId clientID;
	public:
		ClientState();
		bool get_connected();
		void set_connected(bool state);
		JClkLibCommon::TransportClientId get_clientID();
		void set_clientID(JClkLibCommon::TransportClientId cID);
		void get_clientID_memcpy(JClkLibCommon::TransportClientId cID);
		void set_clientID_memcpy(JClkLibCommon::TransportClientId cID);
		DECLARE_ACCESSOR(sessionId);
	};

	extern JClkLibClient::ClientState state;
}

#endif/*PROXY_CLIENT_STATE*/
