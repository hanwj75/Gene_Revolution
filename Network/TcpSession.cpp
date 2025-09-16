#include "TcpSession.h"
#include "TcpServer.h"
#include <iostream>

namespace net = boost::asio;
using tcp = net::ip::tcp;
using namespace std;
using errCode = boost::system::error_code;

TcpSession::TcpSession(tcp::socket socket, TcpServer* server)
	: _socket(move(socket)), _server(server) {
}

void TcpSession::start() {
	doReadHeader();
}

void TcpSession::send(const vector<uint8_t>& data) {
	bool writeInProgress = !_writeBuffer.empty();
	_writeBuffer.insert(_writeBuffer.end(), data.begin(), data.end());

	if (!writeInProgress) {
		doWrite();
	}
}

void TcpSession::close() {
	boost::system::error_code err;
	_socket.close(err);
	notifyDisconnect();
}

string TcpSession::remoteAddress() const {
	return _socket.remote_endpoint().address().to_string();
}

void TcpSession::doReadHeader() {
	auto self(shared_from_this());
	_readBuffer.resize(HEADER_SIZE);

	net::async_read(_socket, net::buffer(_readBuffer), [this, self](errCode err, size_t) {
		if (!err) {
			uint32_t bodyLength = 0;
			memcpy(&bodyLength, _readBuffer.data(), HEADER_SIZE);

			doReadBody(bodyLength);
		}
		else {
			notifyDisconnect();
		}
		});
}

void TcpSession::doReadBody(size_t bodyLength) {
	auto self(shared_from_this());
	_readBuffer.resize(bodyLength);

	net::async_read(_socket, net::buffer(_readBuffer), [this, self](errCode err, size_t) {
		if (!err) {
			//여기서 패킷 핸들러 호출

			doReadHeader(); //다음 패킷 기다림
		}
		else {
			notifyDisconnect();
		}
		});
}

void TcpSession::doWrite() {
	auto self(shared_from_this());
	net::async_write(_socket,
		net::buffer(_writeBuffer), [this, self](errCode err, size_t length) {
			if (!err) {
				_writeBuffer.erase(_writeBuffer.begin(), _writeBuffer.begin() + length);
				if (!_writeBuffer.empty())
					doWrite();
			}
			else {
				notifyDisconnect();
			}
		});
}

void TcpSession::notifyDisconnect() {
	if (_server) {
		_server->onDisconnect(shared_from_this());
	}
}