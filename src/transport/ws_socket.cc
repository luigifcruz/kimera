#include "kimera/transport.hpp"

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;
using websocketpp::lib::thread;

namespace Kimera {

WebsocketServer::WebsocketServer(unsigned int port) : port(port) {
    m_server.init_asio();

    m_server.set_open_handler(bind(&WebsocketServer::on_open, this, ::_1));
    m_server.set_close_handler(bind(&WebsocketServer::on_close, this, ::_1));
}

void WebsocketServer::on_open(connection_hdl hdl) {
    m_connections.insert(hdl);
}

void WebsocketServer::on_close(connection_hdl hdl) {
    m_connections.erase(hdl);
}

void WebsocketServer::send_message(const void* buf, size_t len) {
    for (auto it : m_connections) {
        m_server.send(it, buf, len, websocketpp::frame::opcode::binary);
    }
}

void WebsocketServer::run() {
    m_server.listen(port);
    m_server.start_accept();
    m_server.run();
}


bool Socket::OpenWSClient() {
    std::cerr << "[WS] Websocket client not implemented yet." << std::endl;
    return false;
}

bool Socket::OpenWSServer() {
    ws_server = std::make_shared<WebsocketServer>(state.port);
    std::thread t(bind(&WebsocketServer::run, ws_server.get()));
    t.detach();

    interf = Interfaces::WS;
    return true;
}

void Socket::CloseWS() {
}

int Socket::SendWS(const void* buf, size_t len) {
    ws_server->send_message(buf, len);
    return len;
}

int Socket::RecvWS(void* buf, size_t len) {
}

} // namespace Kimera
