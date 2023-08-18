#include <map>

#ifndef PROXY_CLIENT
#define PROXY_CLIENT

#include <client/message.hpp>

namespace JClkLibProxy {
	class Client;
	typedef std::shared_ptr<JClkLibProxy::Client> ClientX;

	class Client
	{
	public:
		typedef std::pair<JClkLibCommon::sessionId_t,ClientX> SessionMapping_t;
	private:
		static JClkLibCommon::sessionId_t nextSession;
		static std::map<SessionMapping_t::first_type,SessionMapping_t::second_type> SessionMap;
	public:
		static JClkLibCommon::sessionId_t CreateClientSession();
		static ClientX GetClientSession(JClkLibCommon::sessionId_t sessionId);
	private:
		std::unique_ptr<JClkLibCommon::TransportTransmitterContext> transmitContext;
	public:
		void set_transmitContext(decltype(transmitContext)::pointer context)
		{ this->transmitContext.reset(context); }
		auto get_transmitContext() { return transmitContext.get(); }
	};
}

#endif/*PROXY_CLIENT*/
