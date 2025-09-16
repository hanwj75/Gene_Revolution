#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <array>
#include <vector>
#include "center.pb.h"
#include "Packet.h"

namespace net = boost::asio;
using tcp = net::ip::tcp;

class LobbySession : public std::enable_shared_from_this<LobbySession> {
public:
	LobbySession(tcp::socket socket);
	void start();

private:
	tcp::socket socket_;
	std::array<uint8_t, 1024> buffer_;

	void doRead();
	void handlePacket(const std::vector<uint8_t>& data);

	// msgId를 함께 넘겨서 직렬화 수행
	void sendPacket(const center::GamePacket& packet, uint32_t msgId);
};

class LobbyServer {
public:
	LobbyServer(net::io_context& io_context, short port);

private:
	void doAccept();
	tcp::acceptor acceptor_;
};
