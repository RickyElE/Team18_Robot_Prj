#include <iostream>
#include <string>
#include <boost/asio.hpp>


const std::string MOTOR_HOST = "127.0.0.1"; // 如果在同一台樹莓派上運行
const int MOTOR_PORT = 8888;               // C++馬達程式的監聽端口

bool send_to_motor(const std::string& command) {
    try {
        boost::asio::io_service io_service;
        boost::asio::ip::tcp::socket socket(io_service);
        boost::asio::ip::tcp::endpoint endpoint(
            boost::asio::ip::address::from_string(MOTOR_HOST), 
            MOTOR_PORT
        );

        // 連接到馬達控制程式
        socket.connect(endpoint);

        // 發送命令
        boost::asio::write(socket, boost::asio::buffer(command));
        
        // 正常關閉連接
        socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both);
        socket.close();
        
        std::cout << "成功發送命令到馬達控制程式: " << command << std::endl;
        return true;
    }
    catch (std::exception& e) {
        std::cerr << "發送命令到馬達控制程式時出錯: " << e.what() << std::endl;
        return false;
    }
}

bool camera_move_up() {
    return send_to_motor("q");  // 發送 'q' 命令
}


bool camera_move_down() {
    return send_to_motor("w");  // 發送 'w' 命令
}