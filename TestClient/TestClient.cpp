#include <boost/asio.hpp>
#include <iostream>
#include <string>
#include "center.pb.h"
#include "Packet.h"

namespace net = boost::asio;
using tcp = net::ip::tcp;

// 메시지 ID 정의 (서버와 동일하게)
static constexpr uint32_t MSGID_C2S_ENTER_LOBBY = 11;
static constexpr uint32_t MSGID_S2C_ENTER_LOBBY = 12;

int main()
{
	try
	{
		net::io_context io_context;

		// 서버 주소 + 포트
		tcp::resolver resolver(io_context);
		auto endpoints = resolver.resolve("127.0.0.1", "5000");

		tcp::socket socket(io_context);

		// 서버 접속 시도
		net::connect(socket, endpoints);

		std::cout << "✅ 서버 접속 성공!" << std::endl;

		// 1. 로비 입장 요청 패킷 생성
		center::GamePacket packet;
		auto* enterLobby = packet.mutable_c2s_enter_lobby();
		enterLobby->set_user_id("test_user");

		// 2. PacketUtil을 사용하여 직렬화 (메시지 ID 포함)
		std::vector<uint8_t> buffer = PacketUtil::serialize(MSGID_C2S_ENTER_LOBBY, packet);

		// 3. 패킷 전송
		net::write(socket, net::buffer(buffer));

		std::cout << "📤 로비 입장 요청 전송 완료 (메시지 ID: " << MSGID_C2S_ENTER_LOBBY << ")" << std::endl;

		// 4. 서버 응답 수신
		std::array<uint8_t, 4096> reply_buffer;
		size_t len = socket.read_some(net::buffer(reply_buffer));

		// 5. 수신된 데이터를 vector로 변환
		std::vector<uint8_t> received_data(reply_buffer.begin(), reply_buffer.begin() + len);

		// 6. 패킷 역직렬화 (서버와 동일한 방식)
		PacketHeader header;
		std::vector<uint8_t> body;
		if (PacketUtil::deserialize(received_data, header, body)) {
			// 7. 바디를 protobuf 메시지로 파싱
			center::GamePacket response_packet;
			if (response_packet.ParseFromArray(body.data(), static_cast<int>(body.size()))) {
				std::cout << "📥 서버 응답 수신 (메시지 ID: " << header.id << ")" << std::endl;

				// 8. 메시지 ID 기준 라우팅 (서버와 동일한 방식)
				if (header.id == MSGID_S2C_ENTER_LOBBY && response_packet.has_s2c_enter_lobby()) {
					// 로비 입장 응답 처리
					const auto& response = response_packet.s2c_enter_lobby();
					std::cout << "✅ 로비 입장 결과: " << (response.success() ? "성공" : "실패") << std::endl;
					std::cout << "📝 메시지: " << response.message() << std::endl;
					if (response.gold()) {
						std::cout << "💰 보유 골드: " << response.gold() << std::endl;
					}
				}
				else {
					std::cout << "❌ 예상치 못한 응답 메시지 ID: " << header.id << std::endl;
				}
			}
			else {
				std::cout << "❌ 패킷 파싱 실패" << std::endl;
			}
		}
		else {
			std::cout << "❌ 패킷 역직렬화 실패" << std::endl;
		}

		// 소켓 종료
		socket.close();
	}
	catch (std::exception& e)
	{
		std::cerr << "❌ 오류: " << e.what() << std::endl;
	}

	return 0;
}