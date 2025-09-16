#include "LobbyServer.h"
#include <iostream>

using errCode = boost::system::error_code;
using namespace std;

// 메시지 ID (center.proto에 정의된 숫자와 일치시킴)
static constexpr uint32_t MSGID_C2S_ENTER_LOBBY = 11;
static constexpr uint32_t MSGID_S2C_ENTER_LOBBY = 12;

LobbySession::LobbySession(tcp::socket socket)
	: socket_(std::move(socket)) {
}

void LobbySession::start() {
	doRead();
}

void LobbySession::doRead() {
	auto self(shared_from_this());
	socket_.async_read_some(
		boost::asio::buffer(buffer_),
		[this, self](errCode err, size_t length) {
			if (!err) {
				vector<uint8_t> data(buffer_.begin(), buffer_.begin() + length);
				handlePacket(data);
				doRead();
			}
			else {
				cerr << "[LobbySession] Client disconnected or read error: " << err.message() << endl;
			}
		});
}

void LobbySession::handlePacket(const vector<uint8_t>& data) {
	// 1) PacketUtil::deserialize 를 통해 헤더+바디 분리
	PacketHeader header;
	vector<uint8_t> body;
	if (!PacketUtil::deserialize(data, header, body)) {
		cerr << "[LobbySession] Invalid packet (deserialize failed)" << endl;
		return;
	}

	// 2) 바디를 protobuf 메시지로 파싱
	center::GamePacket packet;
	if (!packet.ParseFromArray(body.data(), static_cast<int>(body.size()))) {
		cerr << "[LobbySession] Protobuf parse failed" << endl;
		return;
	}

	// 3) 메시지 ID 기준 라우팅 (추가 패킷은 여기서 확장)
	if (header.id == MSGID_C2S_ENTER_LOBBY && packet.has_c2s_enter_lobby()) {
		cout << "[Lobby] Player entered lobby!" << endl;

		// 간단 응답 생성
		center::GamePacket res;
		auto* enter = res.mutable_s2c_enter_lobby();
		enter->set_success(true);
		enter->set_message("Welcome To The Lobby!");

		// 메시지 ID를 지정해서 직렬화·전송
		sendPacket(res, MSGID_S2C_ENTER_LOBBY);
	}
	else {
		cout << "[LobbySession] Unhandled msgId=" << header.id << endl;
	}
}

void LobbySession::sendPacket(const center::GamePacket& packet, uint32_t msgId) {
	auto self(shared_from_this());
	// PacketUtil::serialize(msgId, msg) 시그니처를 사용
	vector<uint8_t> buffer = PacketUtil::serialize(msgId, packet);

	boost::asio::async_write(
		socket_, boost::asio::buffer(buffer),
		[this, self](errCode err, size_t /*length*/) {
			if (err) {
				cerr << "[LobbySession] Send Failed: " << err.message() << endl;
			}
		});
}

// ---------------- LobbyServer ----------------
LobbyServer::LobbyServer(net::io_context& io_context, short port)
	: acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
	doAccept();
}

void LobbyServer::doAccept() {
	acceptor_.async_accept(
		[this](errCode err, tcp::socket socket) {
			if (!err) {
				cout << "[LobbyServer] New client connected." << endl;
				std::make_shared<LobbySession>(std::move(socket))->start();
			}
			else {
				cerr << "[LobbyServer] Accept failed: " << err.message() << endl;
			}
			doAccept();
		});
}