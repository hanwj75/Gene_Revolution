#include "TcpServer.h"
#include <iostream>
namespace net = boost::asio;
using tcp = net::ip::tcp;
using errCode = boost::system::error_code;
using namespace std;

TcpServer::TcpServer(net::io_context& io, int port)
	: _acceptor(io, tcp::endpoint(tcp::v4(), port)) {
}

void TcpServer::startAccept() {
	_acceptor.async_accept(
		[this](errCode err, tcp::socket socket) {
			if (!err) {
				auto session = make_shared<TcpSession>(move(socket), this);
				_sessions.push_back(session);

				onConnect(session);
				session->start();
			}

			startAccept(); //다음 클라 기다림
		});
}

void TcpServer::onDisconnect(shared_ptr<TcpSession> session) {
	//즉시 반응 : weak_ptr 만 남기고 실제 erase는 cleanup에서 함
	cout << "Client disconnected: " << session->remoteAddress() << endl;
}

void TcpServer::cleanupSessions() {
	//expired 된 세션 제거
	auto it = _sessions.begin();
	while (it != _sessions.end()) {
		if (it->expired()) {
			it = _sessions.erase(it);
		}
		else {
			++it;
		}
	}
}