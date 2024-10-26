#include "server.h"

Server::Server(boost::asio::io_context& io_context, int port)
    : acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
    start_accept();
}

void Server::start_accept() {
    tcp::socket* socket = new tcp::socket(acceptor_.get_executor());
    acceptor_.async_accept(*socket,
                           boost::bind(&Server::handle_accept,
                                   this, socket, boost::asio::placeholders::error));
}

void Server::handle_accept(tcp::socket* socket, const boost::system::error_code& error) {
    if (!error) {
        std::thread(&Server::handle_client, this, socket).detach();
    }
    start_accept();
}

void Server::handle_client(tcp::socket* socket) {
    std::thread([this, socket]() {
        try {
            while (true) {
                char buffer[512];
                boost::system::error_code error;
                size_t length = socket->read_some(boost::asio::buffer(buffer), error);
                if (!error) {
                    std::string command(buffer, length);
                    command.erase(command.find_last_not_of(" \n\r\t")+1);

                    if (command == "pos") {
                        std::string response = robot.current_position() + "\n";
                        boost::asio::write(*socket, boost::asio::buffer(response));
                    } else if (command.substr(0,4) == "turn") {
                        std::istringstream iss(command);
                        std::string turn_cmd, joint_arg, angle_arg;
                        iss >> turn_cmd >> joint_arg >> angle_arg;
                        int joint_number = std::stoi(joint_arg.substr(8)) - 1;
                        double angle = std::stod(angle_arg.substr(8));
                        robot.turn(joint_number, angle);
                        std::string response = robot.current_position() + "\n";
                        boost::asio::write(*socket, boost::asio::buffer(response));
                    } else {
                        std::string response = "Unknown command\n";
                        boost::asio::write(*socket, boost::asio::buffer(response), error);
                    }
                }
            }
        } catch (std::exception& e) {
            std::cerr << "Exception in thread: " << e.what() << "\n";
        }
        socket->close();
        delete socket;
    }).detach();
}