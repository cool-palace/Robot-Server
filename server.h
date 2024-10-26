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
#include "Robot.h"

using boost::asio::ip::tcp;

class Server {
public:
    Server(boost::asio::io_context& io_context, int port);
private:
    void start_accept();
    void handle_accept(tcp::socket* socket, const boost::system::error_code& error);
    void handle_client(tcp::socket* socket);

    tcp::acceptor acceptor_;
    Robot robot;
};


#endif //SERVER_SERVER_H
