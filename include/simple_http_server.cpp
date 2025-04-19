#include "simple_http_server.h"
#include <iostream>
#include <sstream>
#include <cstring>
#include <fstream>

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
    
    // 添加 CORS 頭
    std::string cors_headers = 
        "Access-Control-Allow-Origin: *\r\n"
        "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
        "Access-Control-Allow-Headers: Content-Type\r\n";

    // 添加新的 /capture 路由
    if (path == "/capture") {
        std::vector<uint8_t> jpeg_data;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (image_provider_) {
                jpeg_data = image_provider_();
            }
        }
        
        if (!jpeg_data.empty()) {
            std::ostringstream response;
            response << "HTTP/1.1 200 OK\r\n";
            response << cors_headers;  // 添加 CORS 頭
            response << "Content-Type: image/jpeg\r\n";
            response << "Content-Length: " << jpeg_data.size() << "\r\n";
            response << "\r\n";
            
            write(client_fd, response.str().c_str(), response.str().length());
            write(client_fd, jpeg_data.data(), jpeg_data.size());
        } else {
            // 如果沒有可用的幀
            std::string not_found_body = "No frame available";
            std::ostringstream response;
            response << "HTTP/1.1 404 Not Found\r\n";
            response << cors_headers;  // 添加 CORS 頭
            response << "Content-Type: text/plain\r\n";
            response << "Content-Length: " << not_found_body.length() << "\r\n";
            response << "\r\n";
            response << not_found_body;
            
            write(client_fd, response.str().c_str(), response.str().length());
        }
    } 
    // 原有的路由保持不變
    else if (path == "/" || path.empty() || path == "/website.html") {
        std::string html = readHtmlFile("/home/Team18_Pi/website/resi_website/website.html");
        
        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << cors_headers;  // 添加 CORS 頭
        response << "Content-Type: text/html\r\n";
        response << "Content-Length: " << html.length() << "\r\n";
        response << "\r\n";
        response << html;
        
        write(client_fd, response.str().c_str(), response.str().length());
    } else if (path == "/stream") {
        // 發送 MJPEG 流
        sendMjpegStream(client_fd);
    } else {
        // 返回 404
        std::string not_found_body = "Not Found";
        std::ostringstream response;
        response << "HTTP/1.1 404 Not Found\r\n";
        response << cors_headers;  // 添加 CORS 頭
        response << "Content-Type: text/plain\r\n";
        response << "Content-Length: " << not_found_body.length() << "\r\n";
        response << "\r\n";
        response << not_found_body;
        
        write(client_fd, response.str().c_str(), response.str().length());
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
                         "Access-Control-Allow-Origin: *\r\n"
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

// 添加這個函數來讀取 HTML 檔案
std::string SimpleHttpServer::readHtmlFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "無法打開 HTML 文件: " << path << std::endl;
        // 直接返回一個簡單的備用 HTML，而不是調用 getHtmlPage()
        return R"(
<!DOCTYPE html>
<html>
<head>
    <title>LibCam2Web</title>
    <style>
        body { font-family: Arial; text-align: center; padding: 20px; }
    </style>
</head>
<body>
    <h1>LibCam2Web</h1>
    <p></p>
    <img src="/stream" alt="攝像頭畫面">
</body>
</html>
        )";
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
    return content;
}

