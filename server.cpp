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
                        turn_robot(socket, command);
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

void Server::turn_robot(tcp::socket* socket_, const std::string& command_) {
    {
        std::unique_lock<std::mutex> lock(robot_mutex);
        command_queue.push(std::make_pair(socket_, command_));

        if (is_moving) {
            std::string busy_message = "The robot is currently moving, please wait...\n";
            boost::asio::write(*socket_, boost::asio::buffer(busy_message));
            command_condition.wait(lock, [this] { return !is_moving; });
        }
        is_moving = true;
    }

    auto command_pair = command_queue.front();
    tcp::socket* socket = command_pair.first;
    std::string command = command_pair.second;
    command_queue.pop();

    std::istringstream iss(command);
    std::string turn_cmd, joint_arg, angle_arg, speed_arg;
    iss >> turn_cmd >> joint_arg >> angle_arg >> speed_arg;

    int joint_number = std::stoi(joint_arg.substr(8)) - 1;
    double angle = std::stod(angle_arg.substr(8));
    double speed = speed_arg.empty() ? 180 : std::stod(speed_arg.substr(8));
    std::string moving_message = "Turning joint " + std::to_string(joint_number + 1) + "...\n";
    boost::asio::write(*socket, boost::asio::buffer(moving_message));

    double time_to_turn = abs(angle) / speed;
    std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(time_to_turn * 1000)));

    robot.turn(joint_number, angle, speed);
    std::string response = robot.current_position() + "\n";
    boost::asio::write(*socket, boost::asio::buffer(response));
    {
        std::unique_lock<std::mutex> lock(robot_mutex);
        is_moving = false;
        command_condition.notify_one();
    }

}

std::mutex Server::robot_mutex;