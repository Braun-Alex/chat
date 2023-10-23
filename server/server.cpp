#include "server.h"

using namespace Poco;
using namespace Poco::Net;
using Poco::Mutex;

ChatServer::ChatServer(): _serverSocket(PORT) {}

void ChatServer::run() {
    std::cerr << "Chat server started on port " << PORT << "\n";
    while (true) {
        StreamSocket ss = _serverSocket.acceptConnection();
        auto* thread = new Thread;
        thread->start(new ClientHandler(ss, _clients, _mutex, _chatHistory));
    }
}

ChatServer::ClientHandler::ClientHandler(StreamSocket& ss, std::map<std::string, StreamSocket>& clients,
                                         Mutex& mutex, std::deque<std::string>& chatHistory): _ss(ss),
                                         _clients(clients), _mutex(mutex), _chatHistory(chatHistory) {}

void ChatServer::ClientHandler::run() {
    char buffer[256] = {0};
    int received;
    std::string clientName;
    try {
        while (true) {
            received = _ss.receiveBytes(buffer, sizeof(buffer) - 1);
            if (received > 0) {
                if (buffer[received-1] != '\0') {
                    buffer[received] = '\0';
                }
                std::string message(buffer);
                std::cerr << message << "\n";
                if (message.find("CONNECT: ") == 0) {
                    clientName = message.substr(9);
                    {
                        ScopedLock lock(_mutex);
                        _clients[clientName] = _ss;
                        for (const auto& historyMessage: _chatHistory) {
                            _ss.sendBytes(historyMessage.data(),
                                          static_cast<int>(historyMessage.size()));
                        }
                    }
                    broadcastMessage(clientName + " приєднався до чату.");
                } else if (message.find("DISCONNECT: ") == 0) {
                    ScopedLock lock(_mutex);
                    _clients.erase(clientName);
                    broadcastMessage(clientName + " залишив чат.");
                    return;
                } else if (message.find("PUBLIC MESSAGE: ") == 0) {
                    std::string msg = clientName + ": " + message.substr(16);
                    broadcastMessage(msg);
                } else if (message.find("PRIVATE MESSAGE: ") == 0) {
                    std::string targetName = message.substr(17, message.find(':', 17) - 17);
                    std::string msg = message.substr(message.find(':', 17) + 2);
                    sendPrivateMessage(clientName, targetName, msg);
                }
            }
        }
    } catch (const Poco::Exception& error) {
        std::cerr << "Error: " << error.displayText() << "\n";
    }
}

void ChatServer::ClientHandler::broadcastMessage(const std::string& msg) {
    ScopedLock lock(_mutex);
    std::string messageToSend = msg + '\0';
    for (auto &[name, socket]: _clients) {
        socket.sendBytes(messageToSend.data(), static_cast<int>(messageToSend.size()));
    }
    addToHistory(msg);
}

void ChatServer::ClientHandler::sendPrivateMessage(const std::string& from, const std::string& target,
                                                   const std::string& msg) {
    ScopedLock lock(_mutex);
    if (_clients.find(target) != _clients.end()) {
        std::string messageFrom = "[Приватно від " + from + "] " + msg + '\0',
        messageTo = "[Приватно до " + target + "] " + msg + '\0';
        _clients[target].sendBytes(messageFrom.data(), static_cast<int>(messageFrom.size()));
        _ss.sendBytes(messageTo.data(), static_cast<int>(messageTo.size()));
    } else {
        std::string errorMessage = "Користувача \"" + target + "\" не знайдено" + '\0';
        _ss.sendBytes(errorMessage.data(), static_cast<int>(errorMessage.size()));
    }
}

void ChatServer::ClientHandler::addToHistory(const std::string& msg) {
    std::string messageToStore = msg + '\0';
    _chatHistory.push_back(messageToStore);
    if (_chatHistory.size() > MAX_HISTORY_SIZE) {
        _chatHistory.pop_front();
    }
}