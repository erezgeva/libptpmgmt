#ifndef PROXY_CLIENT_STATE
#define PROXY_CLIENT_STATE

#include <common/jcltypes.hpp>
#include <common/util.hpp>
#include <common/transport.hpp>
#include <common/jclklib_import.hpp>
#include <atomic>
#include <string>

namespace JClkLibClient {
	class ClientState {
	private:
		std::atomic_bool connected;
		std::atomic_bool subscribed;
		JClkLibCommon::sessionId_t sessionId;
		JClkLibCommon::TransportClientId clientID;
		JClkLibCommon::jcl_state eventState;
		JClkLibCommon::jcl_state_event_count eventStateCount;
		JClkLibCommon::jcl_subscription eventSub;

	public:
		ClientState();
		bool get_connected();
		void set_connected(bool state);
		bool get_subscribed();
		void set_subscribed(bool subscriptionState);
		JClkLibCommon::TransportClientId get_clientID();
		void set_clientID(JClkLibCommon::TransportClientId cID);
		JClkLibCommon::jcl_state_event_count &get_eventStateCount();
		JClkLibCommon::jcl_state &get_eventState();
		void set_eventStateCount(JClkLibCommon::jcl_state_event_count eCount);
		void set_eventState(JClkLibCommon::jcl_state eState);
		std::string toString();
		JClkLibCommon::jcl_subscription &get_eventSub();
		//void get_clientID_memcpy(JClkLibCommon::TransportClientId cID);
		//void set_clientID_memcpy(JClkLibCommon::TransportClientId cID);
		DECLARE_ACCESSOR(sessionId);
	};

	extern JClkLibClient::ClientState state;
}

#endif/*PROXY_CLIENT_STATE*/
