#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <set>
#include <vector>
#include "TcpSession.h"
namespace net = boost::asio;

class TcpServer {
public:
	TcpServer(net::io_context& io, int port);
	virtual ~TcpServer() {}

	void startAccept();

	//세션 이벤트
	virtual void onMessage(std::shared_ptr<TcpSession> session, const std::vector<uint8_t>& data) = 0;
	virtual void onConnect(std::shared_ptr<TcpSession> session) {}
	virtual void onDisconnect(std::shared_ptr<TcpSession> session);

	//안전망 : 세션 컨테이너 정리
	void cleanupSessions();

protected:
	net::ip::tcp::acceptor _acceptor;

	//weak_ptr 로 관리 (세션이 죽어도 dangling 방지)
	std::vector<std::weak_ptr<TcpSession>> _sessions;
};