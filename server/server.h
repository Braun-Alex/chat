#pragma once

#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/SocketStream.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Thread.h>
#include <Poco/Mutex.h>

#include <iostream>
#include <deque>
#include <map>

const size_t PORT = 8080;

using namespace Poco;
using namespace Poco::Net;
using Poco::Mutex;

class ChatServer {
public:
    ChatServer();
    [[noreturn]] void run();
private:
    class ClientHandler: public Runnable {
    public:
        ClientHandler(StreamSocket& ss, std::map<std::string, StreamSocket>& clients, Mutex& mutex,
                      std::deque<std::string>& chatHistory);
        void run() override;
    private:
        StreamSocket _ss;
        std::map<std::string, StreamSocket>& _clients;
        Mutex& _mutex;
        std::deque<std::string>& _chatHistory;
        static const int MAX_HISTORY_SIZE = 50;

        void broadcastMessage(const std::string& msg);
        void sendPrivateMessage(const std::string& from, const std::string& target,
                                const std::string& msg);
        void addToHistory(const std::string& msg);
    };

    ServerSocket _serverSocket;
    std::map<std::string, StreamSocket> _clients;
    Mutex _mutex;
    std::deque<std::string> _chatHistory;
};