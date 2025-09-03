#include <iostream>
#include <string>
#include <array>
#include <vector>
#include <boost/asio.hpp>
#include "center.pb.h"

using namespace std;
namespace net = boost::asio;
using tcp = net::ip::tcp;

class Client {
public:
	Client(net::io_context& io, const string& host, const string& port)
		: socket_(io), resolver_(io)
	{
		auto endpoints = resolver_.resolve(host, port);
		net::async_connect(socket_, endpoints,
			[this](boost::system::error_code ec, tcp::endpoint) {
				if (!ec) {
					cout << "Connected to server\n";
					sendLogin();   // 연결 후 로그인 패킷 보냄
					readHeader();  // 서버 응답 대기 시작
				}
			});
	}

	void sendLogin() {
		auth::C2S_Login login;
		login.set_user_id("test01");
		login.set_nickname("tell");
		login.set_password("1234");

		center::GamePacket pkt;
		pkt.mutable_c2s_login()->CopyFrom(login);

		sendPacket(pkt);
	}

	void sendPacket(const google::protobuf::Message& msg) {
		string out;
		if (!msg.SerializeToString(&out)) {
			cerr << "Serialize failed\n";
			return;
		}

		uint32_t len = (uint32_t)out.size();
		vector<uint8_t> buf(4 + len);
		buf[0] = len & 0xFF;
		buf[1] = (len >> 8) & 0xFF;
		buf[2] = (len >> 16) & 0xFF;
		buf[3] = (len >> 24) & 0xFF;
		memcpy(buf.data() + 4, out.data(), len);

		auto sendbuf = make_shared<vector<uint8_t>>(move(buf));
		net::async_write(socket_, net::buffer(*sendbuf),
			[sendbuf](boost::system::error_code ec, size_t /*n*/) {
				if (ec) cerr << "Send failed: " << ec.message() << endl;
			});
	}

private:
	void readHeader() {
		auto hdr = make_shared<array<uint8_t, 4>>();
		net::async_read(socket_, net::buffer(*hdr),
			[this, hdr](boost::system::error_code ec, size_t) {
				if (ec) {
					cerr << "Disconnected: " << ec.message() << endl;
					return;
				}
				uint32_t bodyLen = (*hdr)[0] | ((*hdr)[1] << 8) | ((*hdr)[2] << 16) | ((*hdr)[3] << 24);
				readBody(bodyLen);
			});
	}

	void readBody(size_t bodyLen) {
		auto body = make_shared<vector<uint8_t>>(bodyLen);
		net::async_read(socket_, net::buffer(*body),
			[this, body](boost::system::error_code ec, size_t) {
				if (ec) {
					cerr << "Read body failed: " << ec.message() << endl;
					return;
				}
				center::GamePacket pkt;
				if (pkt.ParseFromArray(body->data(), (int)body->size())) {
					handlePacket(pkt);
				}
				readHeader(); // 다음 패킷 대기
			});
	}

	void handlePacket(const center::GamePacket& pkt) {
		if (pkt.has_s2c_login()) {
			cout << "[LOGIN RES]" << "\n" << " success = " << pkt.s2c_login().success() << "\n" <<
				" userId = " << pkt.s2c_login().user_id() << "\n"
				<< " msg = " << pkt.s2c_login().message() << endl;
		}
		else if (pkt.has_s2c_game_start()) {
			cout << "[GAME START] " << pkt.s2c_game_start().message() << endl;
		}
		else if (pkt.has_s2c_player_move()) {
			cout << "[MOVE] " << pkt.s2c_player_move().user_id()
				<< " -> (" << pkt.s2c_player_move().x()
				<< "," << pkt.s2c_player_move().y() << ")" << endl;
		}
		else {
			cout << "[UNKNOWN PACKET]" << endl;
		}
	}

	tcp::socket socket_;
	tcp::resolver resolver_;
};

int main() {
	try {
		net::io_context io;
		Client client(io, "127.0.0.1", "12345");

		io.run(); // 이벤트 루프 (끊기 전까지 계속 유지)
	}
	catch (exception& e) {
		cerr << "Exception: " << e.what() << endl;
	}
}