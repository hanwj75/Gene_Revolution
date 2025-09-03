#include "lobby_server.h"
#include "session.h"
namespace net = boost::asio;
using tcp = net::ip::tcp;

LobbyServer::LobbyServer(net::io_context& io, unsigned short port)
    : acceptor_(io, tcp::endpoint(tcp::v4(), port)) {
    doAccept();
}

void LobbyServer::doAccept() {
    acceptor_.async_accept([this](boost::system::error_code ec, tcp::socket socket) {
        if (!ec) {
            std::make_shared<Session>(std::move(socket))->start();
        }
        doAccept();
        });
}
