#ifndef SERVER_SERVER_H
#define SERVER_SERVER_H
#include <iostream>
#include <fstream>
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <thread>
#include <mutex>
#include <string>
#include <chrono>
#include <queue>
#include "Robot.h"

using boost::asio::ip::tcp;

class Server {
public:
    Server(boost::asio::io_context& io_context, int port);
private:
    void start_accept();
    void handle_accept(tcp::socket* socket, const boost::system::error_code& error);
    void handle_client(tcp::socket* socket);
    void turn_robot(tcp::socket* socket, const std::string& command);

    tcp::acceptor acceptor_;
    Robot robot;
    static std::mutex robot_mutex;
    std::queue<std::pair<tcp::socket*, std::string>> command_queue;
    std::condition_variable command_condition;
    bool is_moving = false;
};


#endif //SERVER_SERVER_H
