#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <array>
#include <vector>
using namespace std;
namespace google { namespace protobuf { class Message; } }

//길이(4바이트) + 바디 프레이밍 기반 세션
class Session : public enable_shared_from_this<Session> {
public:
	explicit Session(boost::asio::ip::tcp::socket socket)

		: socket_(move(socket)) {
	}

	void start();

private:
	void readHeader();
	void readBody(size_t bodylen);
	void handlePacket(const vector<uint8_t>& body);

	//안전한 비동기 전송을 위한 헬퍼
	void sendPacket(const google::protobuf::Message& msg);

	//현재 세션 상태: 로그인 / 로비 / 인게임
	enum class State {
		Auth,
		Lobby,
		Ingame
	};
	State state_ = State::Auth;

	boost::asio::ip::tcp::socket socket_;
	array<uint8_t, 4> header_{};
	vector<uint8_t> body_;
};