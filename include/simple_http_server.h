#ifndef SIMPLE_HTTP_SERVER_H
#define SIMPLE_HTTP_SERVER_H

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <atomic>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

class SimpleHttpServer {
public:
    SimpleHttpServer(int port = 8081);
    ~SimpleHttpServer();

    // 啟動服務器
    bool start();
    
    // 停止服務器
    void stop();
    
    // 設置 MJPEG 流的回調函數
    void setImageProvider(std::function<std::vector<uint8_t>()> provider);

private:
    int port_;
    int server_fd_;
    std::string readHtmlFile(const std::string& path);
    std::string getHtmlPage();
    std::atomic<bool> running_;
    std::thread server_thread_;
    std::mutex mutex_;
    std::function<std::vector<uint8_t>()> image_provider_;
    
    
    // 處理客戶端連接
    void clientHandler(int client_fd);
    
    // 服務器主循環
    void serverLoop();
    
    // 發送 HTTP 回應
    void sendHttpResponse(int client_fd, int status_code, const std::string& content_type, const std::string& body);
    
    // 發送 MJPEG 流
    void sendMjpegStream(int client_fd);
    
};

#endif // SIMPLE_HTTP_SERVER_H
