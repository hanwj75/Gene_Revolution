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

	//���� �̺�Ʈ
	virtual void onMessage(std::shared_ptr<TcpSession> session, const std::vector<uint8_t>& data) = 0;
	virtual void onConnect(std::shared_ptr<TcpSession> session) {}
	virtual void onDisconnect(std::shared_ptr<TcpSession> session);

	//������ : ���� �����̳� ����
	void cleanupSessions();

protected:
	net::ip::tcp::acceptor _acceptor;

	//weak_ptr �� ���� (������ �׾ dangling ����)
	std::vector<std::weak_ptr<TcpSession>> _sessions;
};