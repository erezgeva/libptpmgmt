#ifndef PROXY_CLIENT_STATE
#define PROXY_CLIENT_STATE

#include <common/jcltypes.hpp>
#include <common/util.hpp>

namespace JClkLibClient {
	class ClientState {
	private:
		bool connected;
		JClkLibCommon::sessionId_t sessionId;
	public:
		ClientState();
		DECLARE_ACCESSOR(connected);
		DECLARE_ACCESSOR(sessionId);
	};

	extern JClkLibClient::ClientState state;
}

#endif/*PROXY_CLIENT_STATE*/
