#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <array>
#include <vector>
using namespace std;
namespace google { namespace protobuf { class Message; } }

//����(4����Ʈ) + �ٵ� �����̹� ��� ����
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

	//������ �񵿱� ������ ���� ����
	void sendPacket(const google::protobuf::Message& msg);

	//���� ���� ����: �α��� / �κ� / �ΰ���
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