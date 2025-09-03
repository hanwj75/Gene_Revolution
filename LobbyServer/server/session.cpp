#include "session.h"
#include <iostream>
#include <cstring>
#include "../proto/center.pb.h"
#include "google/protobuf/message.h"
#include <memory>

using namespace std;
namespace net = boost::asio;
using tcp = net::ip::tcp;

static uint32_t le32(const uint8_t h[4]) {
	return (uint32_t)h[0] | ((uint32_t)h[1] << 8) | ((uint32_t)h[2] << 16) | ((uint32_t)h[3] << 24);
}

void Session::start() {
	readHeader();
}

void Session::readHeader() {
	auto self = shared_from_this();
	net::async_read(socket_, net::buffer(header_),
		[this, self](boost::system::error_code err, size_t /*n*/) {
			if (err) {
				cerr << "readHeader Error: " << err.message() << endl;
				return;
			}
			uint32_t len = le32(header_.data());
			if (len == 0 || len > 1'000'000) {
				cerr << "Invalid Length: " << len << endl;
				return;//간단한 방어
			}

			readBody(len);
		}
	);
}

void Session::readBody(size_t bodyLen) {
	auto self = shared_from_this();
	body_.resize(bodyLen);
	net::async_read(socket_, net::buffer(body_),
		[this, self](boost::system::error_code err, size_t/*n*/) {
			if (err) {
				cerr << "readBody Error: " << err.message() << endl;
				return;
			}
			//패킷 처리 (동기 호출, body_는 멤버라 안전)
			handlePacket(body_);
			//다음 패킷 수신을 위해 헤더 읽기 재개
			readHeader();
		}
	);
}
//안전한 비동기 전송 헬퍼
void Session::sendPacket(const google::protobuf::Message& msg) {
	//메세지 직렬화 -> shared_ptr<string> 에 저장
	auto body = make_shared<string>();
	if (!msg.SerializeToString(body.get())) {
		cerr << "SerializeToString Failed\n";
		return;
	}

	//헤더(4바이트 리틀엔디안) + 바디를 하나의 버퍼로 합침 (shared_ptr<vector<uint8_t>>)
	uint32_t len = static_cast<uint32_t>(body->size());
	auto sendbuf = make_shared<vector<uint8_t>>(4 + len);
	(*sendbuf)[0] = static_cast<uint8_t>(len & 0xFF);
	(*sendbuf)[1] = static_cast<uint8_t>((len >> 8) & 0xFF);
	(*sendbuf)[2] = static_cast<uint8_t>((len >> 16) & 0xFF);
	(*sendbuf)[3] = static_cast<uint8_t>((len >> 24) & 0xFF);
	memcpy(sendbuf->data() + 4, body->data(), len);

	//async_write에 sendbuf(shared_ptr)를 캡처해서 버퍼 생명 보장
	auto self = shared_from_this();
	net::async_write(socket_, net::buffer(*sendbuf), [this, self, sendbuf](boost::system::error_code err, size_t/*n*/) {
		if (err) {
			cerr << "SendPacket write failed: " << err.message() << endl;
			return;
		}
		});
}

void Session::handlePacket(const vector<uint8_t>& body) {
	center::GamePacket pkt;
	if (!pkt.ParseFromArray(body.data(), (int)body.size())) {
		cerr << "Invalid Game Packet\n";
		return;
	}

	//어떤 메시지가 들어왔는지 확인
	switch (pkt.payload_case()) {
		//[로그인 서버 ] 로그인
	case center::GamePacket::kC2SLogin: {
		const auto& login = pkt.c2s_login();
		cout << "[ LOGIN ] Name = " << login.nickname() << "\n";

		//응답 생성
		center::GamePacket outMsg;
		auto* res = outMsg.mutable_s2c_login();
		res->set_success(true);
		res->set_user_id(login.user_id());
		res->set_message("Login Success [ " + login.nickname() + " ]");

		//안전 전송
		sendPacket(outMsg);
		break;
	}

	case center::GamePacket::kC2SEnterLobby: {
		//[로비서버] 로비 입장
		const auto& enter = pkt.c2s_enter_lobby();
		cout << "[ LOBBY ] EnterUser = " << enter.user_id() << endl;

		//응답 생성
		center::GamePacket outMsg;
		auto* res = outMsg.mutable_s2c_enter_lobby();
		res->set_success(true);
		res->set_message(" Enter Lobby! ");
	}
	case center::GamePacket::kC2SGameStart: {
		//[로비서버] 게임 시작
		const auto& req = pkt.c2s_game_start();
		cout << "[ LOBBY ] GameStart from user: " << req.user_id() << "\n";

		//응답 생성
		center::GamePacket outMsg;
		auto* res = outMsg.mutable_s2c_game_start();
		res->set_success(true);
		res->set_message(" Game Start! ");

		sendPacket(outMsg);
		break;
	}

	case center::GamePacket::kC2SPlayerMove: {
		//[인게임서버] 플레이어 이동
		const auto& move = pkt.c2s_player_move();
		cout << "[ INGAME ] Move " << move.user_id() << " -> (" << move.x() << "," << move.y() << ")\n";

		//브로드캐스트/응답
		center::GamePacket outMsg;
		auto* res = outMsg.mutable_s2c_player_move();
		res->set_user_id(move.user_id());
		res->set_x(move.x());
		res->set_y(move.y());
		res->set_dir(move.dir());

		sendPacket(outMsg);
		break;
	}

	case center::GamePacket::kC2SPlayerAttack: {
		//[인게임서버] 플레이어 공격
		const auto& atk = pkt.c2s_player_attack();
		cout << "[ INGAME ] Attack Player: " << atk.user_id() << "Attack Type: [" << atk.attack_type() << "]" << endl;

		//공격 이벤트 브로드캐스트/응답
		center::GamePacket outMsg;
		auto* res = outMsg.mutable_s2c_player_attack();
		res->set_user_id(atk.user_id());
		res->set_attack_type(atk.attack_type());
		res->set_hit()
	}
	default:
		cerr << "Unknown Packet\n";
		break;
	}
}