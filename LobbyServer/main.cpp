#include "LobbyServer.h"

int main() {
	try {
		int port = 5000;
		boost::asio::io_context io_context;
		LobbyServer server(io_context, port);

		std::cout << "Lobby Server Start PORT : " << port << std::endl;
		io_context.run();
	}
	catch (std::exception& err) {
		std::cerr << "Exception: " << err.what() << std::endl;
	}
}