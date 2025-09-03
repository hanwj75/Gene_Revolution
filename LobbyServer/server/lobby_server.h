#pragma once
#include <boost/asio.hpp>
namespace net = boost::asio;
class LobbyServer {
public:
	LobbyServer(net::io_context& io, unsigned short port);
private:
	void doAccept();
	net::ip::tcp::acceptor acceptor_;
};