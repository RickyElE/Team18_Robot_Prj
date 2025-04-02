#include "simple_http_server.h"
#include <iostream>
#include <sstream>
#include <cstring>

SimpleHttpServer::SimpleHttpServer(int port) 
    : port_(port), server_fd_(-1), running_(false) {
}

SimpleHttpServer::~SimpleHttpServer() {
    stop();
}

bool SimpleHttpServer::start() {
    if (running_) return true;
    
    // 創建套接字
    server_fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd_ < 0) {
        std::cerr << "創建套接字失敗" << std::endl;
        return false;
    }
    
    // 設置端口重用
    int opt = 1;
    if (setsockopt(server_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "設置套接字選項失敗" << std::endl;
        close(server_fd_);
        return false;
    }
    
    // 綁定地址
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port_);
    
    if (bind(server_fd_, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "綁定地址失敗" << std::endl;
        close(server_fd_);
        return false;
    }
    
    // 監聽連接
    if (listen(server_fd_, 5) < 0) {
        std::cerr << "監聽失敗" << std::endl;
        close(server_fd_);
        return false;
    }
    
    running_ = true;
    
    // 在新線程中啟動服務器循環
    server_thread_ = std::thread(&SimpleHttpServer::serverLoop, this);
    
    std::cout << "HTTP 服務器已啟動，監聽端口 " << port_ << std::endl;
    return true;
}

void SimpleHttpServer::stop() {
    if (!running_) return;
    
    running_ = false;
    
    // 關閉服務器套接字
    if (server_fd_ >= 0) {
        close(server_fd_);
        server_fd_ = -1;
    }
    
    // 等待服務器線程結束
    if (server_thread_.joinable()) {
        server_thread_.join();
    }
    
    std::cout << "HTTP 服務器已停止" << std::endl;
}

void SimpleHttpServer::setImageProvider(std::function<std::vector<uint8_t>()> provider) {
    std::lock_guard<std::mutex> lock(mutex_);
    image_provider_ = provider;
}

void SimpleHttpServer::serverLoop() {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    
    // 設置超時時間
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
    
    fd_set readfds;
    
    while (running_) {
        FD_ZERO(&readfds);
        FD_SET(server_fd_, &readfds);
        
        // 使用select監聽套接字，帶超時
        int activity = select(server_fd_ + 1, &readfds, NULL, NULL, &timeout);
        
        if (activity < 0 && errno != EINTR) {
            std::cerr << "select 錯誤" << std::endl;
            break;
        }
        
        // 檢查是否有新的連接
        if (activity > 0 && FD_ISSET(server_fd_, &readfds)) {
            int client_fd = accept(server_fd_, (struct sockaddr *)&client_addr, &client_len);
            if (client_fd < 0) {
                std::cerr << "接受連接失敗" << std::endl;
                continue;
            }
            
            // 在新線程中處理客戶端
            std::thread client_thread(&SimpleHttpServer::clientHandler, this, client_fd);
            client_thread.detach();
        }
    }
}

void SimpleHttpServer::clientHandler(int client_fd) {
    // 接收 HTTP 請求
    char buffer[1024] = {0};
    int bytes_read = read(client_fd, buffer, sizeof(buffer) - 1);
    
    if (bytes_read <= 0) {
        close(client_fd);
        return;
    }
    
    // 解析請求
    std::string request(buffer);
    std::string path;
    
    if (request.find("GET ") == 0) {
        size_t path_start = 4; // 跳過 "GET "
        size_t path_end = request.find(" HTTP/", path_start);
        if (path_end != std::string::npos) {
            path = request.substr(path_start, path_end - path_start);
        }
    }
    
    // 根據路徑處理請求
    if (path == "/" || path.empty()) {
        // 返回 HTML 頁面
        sendHttpResponse(client_fd, 200, "text/html", getHtmlPage());
    } else if (path == "/stream") {
        // 發送 MJPEG 流
        sendMjpegStream(client_fd);
    } else {
        // 返回 404
        sendHttpResponse(client_fd, 404, "text/plain", "Not Found");
    }
    
    close(client_fd);
}

void SimpleHttpServer::sendHttpResponse(int client_fd, int status_code, const std::string& content_type, const std::string& body) {
    std::string status_text;
    switch (status_code) {
        case 200: status_text = "OK"; break;
        case 404: status_text = "Not Found"; break;
        default: status_text = "Unknown";
    }
    
    std::ostringstream response;
    response << "HTTP/1.1 " << status_code << " " << status_text << "\r\n";
    response << "Content-Type: " << content_type << "\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;
    
    write(client_fd, response.str().c_str(), response.str().length());
}

void SimpleHttpServer::sendMjpegStream(int client_fd) {
    // 發送 MJPEG 流頭部
    std::string header = "HTTP/1.1 200 OK\r\n"
                         "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n"
                         "Cache-Control: no-cache, no-store, must-revalidate\r\n"
                         "Pragma: no-cache\r\n"
                         "Expires: 0\r\n"
                         "Connection: close\r\n"
                         "\r\n";
    
    write(client_fd, header.c_str(), header.length());
    
    // 持續發送圖像
    while (running_) {
        std::vector<uint8_t> jpeg_data;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (image_provider_) {
                jpeg_data = image_provider_();
            }
        }
        
        if (!jpeg_data.empty()) {
            std::ostringstream frame_header;
            frame_header << "--frame\r\n";
            frame_header << "Content-Type: image/jpeg\r\n";
            frame_header << "Content-Length: " << jpeg_data.size() << "\r\n";
            frame_header << "\r\n";
            
            write(client_fd, frame_header.str().c_str(), frame_header.str().length());
            write(client_fd, jpeg_data.data(), jpeg_data.size());
            write(client_fd, "\r\n", 2);
        }
        
        // 睡眠一小段時間
        usleep(30000);  // 30ms
    }
}

std::string SimpleHttpServer::getHtmlPage() {
    return R"(
<!DOCTYPE html>
<html>
<head>
    <title>LibCam2Web</title>
    <style>
        body {
            font-family: Arial, sans-serif;
            text-align: center;
            background-color: #f0f0f0;
            margin: 0;
            padding: 20px;
        }
        .container {
            max-width: 800px;
            margin: 0 auto;
            background-color: white;
            border-radius: 10px;
            padding: 20px;
            box-shadow: 0 0 10px rgba(0, 0, 0, 0.1);
        }
        h1 {
            color: #333;
        }
        img {
            max-width: 100%;
            border: 1px solid #ddd;
            border-radius: 5px;
        }
        .brightness-indicator {
            width: 100%;
            height: 30px;
            margin-top: 20px;
            background: linear-gradient(to right, #000000, #ffffff);
            border-radius: 5px;
            position: relative;
        }
        .indicator {
            position: absolute;
            top: -10px;
            width: 10px;
            height: 50px;
            background-color: red;
            transform: translateX(-50%);
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>LibCam2Web</h1>
        <img src="/stream" alt="攝像頭畫面">
        <div class="brightness-indicator">
            <div class="indicator" id="brightness-indicator" style="left: 50%;"></div>
        </div>
    </div>

    <script>
        function updateBrightnessIndicator() {
            const randomBrightness = Math.random() * 100;
            document.getElementById('brightness-indicator').style.left = randomBrightness + '%';
            
            setTimeout(updateBrightnessIndicator, 500);
        }
        
        updateBrightnessIndicator();
    </script>
</body>
</html>
    )";
}
