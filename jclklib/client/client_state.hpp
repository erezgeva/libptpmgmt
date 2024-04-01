#ifndef PROXY_CLIENT_STATE
#define PROXY_CLIENT_STATE

#include <common/jcltypes.hpp>
#include <common/util.hpp>
#include <atomic>

namespace JClkLibClient {
	class ClientState {
	private:
		std::atomic_bool connected;
		JClkLibCommon::sessionId_t sessionId;
	public:
		ClientState();
		bool get_connected();
		void set_connected(bool state);
		DECLARE_ACCESSOR(sessionId);
	};

	extern JClkLibClient::ClientState state;
}

#endif/*PROXY_CLIENT_STATE*/
