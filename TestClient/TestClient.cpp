#include <boost/asio.hpp>
#include <iostream>
#include <string>

namespace net = boost::asio;
using tcp = net::ip::tcp;

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

		// 간단히 메시지 전송
		std::string msg = "Hello LobbyServer!";
		net::write(socket, net::buffer(msg));

		std::cout << "📤 메시지 전송 완료: " << msg << std::endl;

		// 서버 응답 수신 (없으면 블록됨, 테스트용으로 1초 타임아웃 주는 게 안전)
		char reply[1024];
		size_t len = socket.read_some(net::buffer(reply));
		std::cout << "📥 서버 응답: " << std::string(reply, len) << std::endl;
	}
	catch (std::exception& e)
	{
		std::cerr << "❌ 오류: " << e.what() << std::endl;
	}

	return 0;
}