#include "Channel/channel.hpp"
#include <asio.hpp>
#include <format>
#include <iostream>
#include <string>
#include <thread>
#include <unordered_map>
#include <variant>

using SocketPtr = std::shared_ptr<asio::ip::tcp::socket>;

struct NewConnection {
    SocketPtr socket;
};

struct NewMessage {
    asio::ip::tcp::endpoint from;
    std::string message;
};

struct Disconnection {
    asio::ip::tcp::endpoint endpoint;
};

using Message = std::variant<NewConnection, NewMessage, Disconnection>;
using Clients = std::unordered_map<asio::ip::tcp::endpoint, SocketPtr>;

class MessageHandler {
public:
    void operator()(NewConnection new_connection) {
        if (const auto& remote_endpoint =
                new_connection.socket->remote_endpoint();
            clients.find(remote_endpoint) == std::end(clients)) {
            new_connection.socket->write_some(
                asio::buffer("Hello, you are now connected to server...\n"));
            clients.insert({remote_endpoint, new_connection.socket});
            std::cout << std::format(
                "New client {}:{} connected to server...\n",
                remote_endpoint.address().to_string(), remote_endpoint.port());
        }
    }

    void operator()(NewMessage new_msg) {
        for (const auto& client : clients) {
            const auto& [endpoint, socket] = client;
            if (endpoint != new_msg.from) {
                socket->write_some(asio::buffer(new_msg.message));
            }
        }
    }

    void operator()(Disconnection disconnection) {
        if (const auto client_it = clients.find(disconnection.endpoint);
            client_it != std::end(clients)) {
            std::cout << std::format(
                "Client {}:{} left the server...",
                disconnection.endpoint.address().to_string(),
                disconnection.endpoint.port());
            std::cout << std::flush;
            clients.erase(client_it);
        }
    }

private:
    Clients clients;
};

void client(SocketPtr socket, Sender<Message> sender) {
    const auto& endpoint = socket->remote_endpoint();

    sender.send(NewConnection{socket});

    std::array<uint8_t, 1024> buffer;
    asio::error_code ec;
    while (true) {
        auto n = socket->read_some(asio::buffer(buffer), ec);
        if (ec == asio::error::eof) {
            sender.send(Disconnection{endpoint});
            return;
        } else {
            sender.send(
                NewMessage{endpoint, std::string{std::begin(buffer),
                                                 std::begin(buffer) + n}});
        }
    }
}

void server(Receiver<Message> receiver) {
    MessageHandler msg_handler{};

    while (true) {
        auto data = receiver.recv();
        if (data) {
            std::visit(msg_handler, *data);

        } else {
        }
    }
}

int main() {
    asio::io_context io_context{};
    asio::ip::tcp::endpoint endpoint{asio::ip::tcp::v4(), 6969};
    asio::ip::tcp::acceptor acceptor{io_context, endpoint};

    auto [sender, receiver] = make_channel<Message>();

    std::thread(server, std::move(receiver)).detach();

    while (true) {
        asio::ip::tcp::socket socket{acceptor.accept()};

        std::thread{client,
                    std::make_shared<asio::ip::tcp::socket>(std::move(socket)),
                    sender}
            .detach();
    }

    return 0;
}
