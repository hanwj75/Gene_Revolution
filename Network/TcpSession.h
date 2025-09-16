#pragma once
#include <boost/asio.hpp>
#include <memory>
#include <vector>

class TcpServer;

class TcpSession : public std::enable_shared_from_this<TcpSession>
{
public:
	TcpSession(boost::asio::ip::tcp::socket socket, TcpServer* server);
	void start();
	void send(const std::vector<uint8_t>& data);
	void close();
	std::string remoteAddress() const;

private:
	void doReadHeader();
	void doReadBody(size_t bodyLength);
	void doWrite();
	void notifyDisconnect(); // 서버에게 끊김 알림

private:
	boost::asio::ip::tcp::socket _socket;
	TcpServer* _server; //소유 서버

	enum { HEADER_SIZE = 4 };

	std::vector<uint8_t> _readBuffer;
	std::vector<uint8_t> _writeBuffer;
};