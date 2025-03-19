#include <iostream>
#include <string>
#include <thread>
#include <mutex>
#include <vector>
#include <atomic>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#include <cstdlib> // 用於 getenv

//new

// 全局變量
std::mutex frameMutex;
std::vector<uint8_t> latestFrame;
std::atomic<bool> running{true};
const int PORT = 8080;
std::string webroot = "/home/Team18_Pi/website/Team18_Robot_Prj/website"; // 網頁文件所在的路徑

// 函數聲明
void cameraThread();
void handleClient(int clientSocket);
std::string getContentType(const std::string& path);
bool fileExists(const std::string& path);

// 處理信號
void signalHandler(int signum) {   
    
    running = false;    //接收到ctrl+c指令時，中斷程序
}

// 獲取文件的 MIME 類型
std::string getContentType(const std::string& path) {
    if (path.find(".html") != std::string::npos) {
        return "text/html";
    } else if (path.find(".css") != std::string::npos) {
        return "text/css";
    } else if (path.find(".js") != std::string::npos) {
        return "application/javascript";
    } else if (path.find(".jpg") != std::string::npos || path.find(".jpeg") != std::string::npos) {
        return "image/jpeg";
    } else if (path.find(".png") != std::string::npos) {
        return "image/png";
    } else if (path.find(".gif") != std::string::npos) {
        return "image/gif";
    } else if (path.find(".ico") != std::string::npos) {
        return "image/x-icon";
    } else {
        return "application/octet-stream";
    }
}

// 檢查文件是否存在
bool fileExists(const std::string& path) {
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

// 使用 libcamera-still 捕獲圖像
void cameraThread() {   //生產者
    // 創建臨時文件路徑
    const char* tempJpegPath = "/tmp/pi_camera_stream.jpg";
    char command[256];
    

    //主循環: 只要有在運作就會持續執行
    while (running) {

        // 使用 libcamera-still 命令捕獲一張圖像
        snprintf(command, sizeof(command), 
                 "libcamera-still -n --immediate --width 320 --height 240 --output %s --timeout 1 --hflip --vflip --quality 70", 
                 tempJpegPath);
        
        // 執行命令
        int result = system(command);   //使用system()執行拍照命令


        if (result != 0) {    //如果拍照失敗   0:成功
            std::cerr << "Failed to capture image with libcamera-still" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));      //等待一秒後重試
            continue;  
        }
        
        // 讀取 JPEG 文件
        std::ifstream file(tempJpegPath, std::ios::binary | std::ios::ate);

        //檢查文件是否打開成功  
        if (!file.is_open()) {
            std::cerr << "Failed to open captured image file" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        
        // 獲取文件大小
        std::streamsize size = file.tellg();   //獲取文件末尾的位置，以獲取文件大小
        file.seekg(0, std::ios::beg);   //重新定位到文件開頭
        
        // 讀取文件內容
        std::vector<uint8_t> buffer(size);

        if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
            std::cerr << "Failed to read image file" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        file.close();
        
        // 更新最新幀
        std::lock_guard<std::mutex> lock(frameMutex);
        latestFrame = buffer;
        
        // 短暫延遲控制幀率
        std::this_thread::sleep_for(std::chrono::milliseconds(100));    //每幀之間暫停100毫秒
    }
}

// 處理客戶端連接
void handleClient(int clientSocket) {   //消費者

    //定義接收緩衝區大小
    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    
    // 接收 HTTP 請求
    ssize_t bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);   //recv():接收客戶端的http請求
    if (bytesRead <= 0) {     
        close(clientSocket);
        return;
    }
    
    buffer[bytesRead] = '\0';     //確保以null結尾
    std::string request(buffer);
    
    // 解析 HTTP 請求
    std::string path;
    if (request.find("GET") == 0) {

        //找到路徑的起始與結束位置
        size_t pathStart = request.find(" ") + 1;
        size_t pathEnd = request.find(" ", pathStart);
        path = request.substr(pathStart, pathEnd - pathStart);
        
        // 默認首頁
        if (path == "/") {
            path = "/website.html";
        }
    }
    
    // 處理視頻流請求
    if (path == "/video_feed") {     //處理影像
        // 發送 MJPEG 串流的 HTTP 頭
        // Motion JPEG : 傳送連續的JPEG 圖像形成影片
        const char* header = "HTTP/1.1 200 OK\r\n"
                            "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n"
                            "Cache-Control: no-cache\r\n"
                            "Connection: close\r\n"
                            "\r\n";
        
        send(clientSocket, header, strlen(header), 0);
        
        while (running) {
            // 獲取最新幀
            std::vector<uint8_t> frame;
            {
                std::lock_guard<std::mutex> lock(frameMutex);
                frame = latestFrame;
            }
            
            if (!frame.empty()) {
                // 創建 MJPEG 幀頭部
                std::string frameHeader = "--frame\r\n"
                                         "Content-Type: image/jpeg\r\n"
                                         "Content-Length: " + std::to_string(frame.size()) + "\r\n"
                                         "\r\n";
                
                // 發送幀頭部
                send(clientSocket, frameHeader.c_str(), frameHeader.length(), 0);
                
                // 發送幀數據
                send(clientSocket, frame.data(), frame.size(), 0);
                
                // 發送尾部換行
                const char* trailer = "\r\n";
                send(clientSocket, trailer, strlen(trailer), 0);
            }
            
            // 短暫延遲
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        
        close(clientSocket);
    }
    // 處理靜態文件請求 (ex:網頁等)
    else {
        // 構建本地文件路徑
        std::string filepath = webroot + path;
        
        // 檢查文件是否存在
        if (fileExists(filepath)) {

            // 獲取文件類型
            std::string contentType = getContentType(filepath);
            
            // 讀取文件
            std::ifstream file(filepath, std::ios::binary | std::ios::ate);

            if (file.is_open()) {
                // 獲取文件大小
                std::streamsize size = file.tellg();    
                file.seekg(0, std::ios::beg);
                
                // 讀取文件內容
                std::vector<char> content(size);
                if (file.read(content.data(), size)) {
                    // 創建 HTTP 響應頭
                    std::string header = "HTTP/1.1 200 OK\r\n"
                                        "Content-Type: " + contentType + "\r\n"
                                        "Content-Length: " + std::to_string(size) + "\r\n"
                                        "Connection: close\r\n"
                                        "\r\n";
                    
                    // 發送響應頭
                    send(clientSocket, header.c_str(), header.length(), 0);
                    
                    // 發送文件內容
                    send(clientSocket, content.data(), content.size(), 0);
                }
                file.close();
            }
        } else {
            // 文件不存在，返回 404
            std::string notFound = "HTTP/1.1 404 Not Found\r\n"
                                  "Content-Type: text/plain\r\n"
                                  "Content-Length: 9\r\n"
                                  "\r\n"
                                  "Not Found";
            
            send(clientSocket, notFound.c_str(), notFound.length(), 0);
        }
        
        close(clientSocket);
    }
}

int main() {
    // 設置信號處理 
    signal(SIGINT, signalHandler);     //ctrl+c 中止程序     //signalHandler 關閉服務器
    signal(SIGTERM, signalHandler);    //SIGTERM 程序中止信號
    
    // 創建服務器套接字
    // socket : 網路通訊的端點，讓兩個設備可以互相傳輸數據
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);    // AF_INET : IPv4 通訊協議   //SOCK_STREAM : 使用TCP(傳輸控制協議)
    if (serverSocket < 0) {    //如果socket創建失敗 
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }
    
    // 設置套接字選項
    int opt = 1;   //代表開啟 SO_REUSEADDR，允許 socket 快速重用端口
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error setting socket options" << std::endl;
        return 1;
    }
    
    // 設置服務器地址
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;     // 使用 IPv4 網路協議
    serverAddr.sin_addr.s_addr = INADDR_ANY;    // 允許任何 IP 連接
    serverAddr.sin_port = htons(PORT);     // 設置端口（轉換為網路字節序）
    
    // 綁定套接字
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {    //綁定伺服器的socket到指定的IP和端口
        std::cerr << "Error binding socket" << std::endl;
        return 1;
    }
    
    // 監聽連接
    if (listen(serverSocket, 10) < 0) {    //進入監聽模式，準備接收連線     //最多允許10個連線請求
        std::cerr << "Error listening on socket" << std::endl;
        return 1;
    }
    
    std::cout << "Server started at http://0.0.0.0:" << PORT << std::endl;
    std::cout << "Serving files from: " << webroot << std::endl;
    
    // 啟動攝像頭線程
    std::thread camThread(cameraThread);
    
    // 接受和處理客戶端連接
    while (running) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        
        // 設置超時以允許檢查 running 標誌
        fd_set readSet;     //存放要監聽的 socket 列表的資料結構
        FD_ZERO(&readSet);    //清空 fd_set（初始化）
        FD_SET(serverSocket, &readSet);      //檢查是否有新的連線
        
        struct timeval timeout;
        timeout.tv_sec = 1;     //設定超時時間為1秒
        timeout.tv_usec = 0;
        
        int ready = select(serverSocket + 1, &readSet, NULL, NULL, &timeout);
        if (ready <= 0) {
            // 超時或錯誤
            continue;
        }
        
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);    //接受新的客戶端連線
        if (clientSocket < 0) {
            std::cerr << "Error accepting connection" << std::endl;
            continue;
        }
        
        // 在單獨的線程中處理客戶端
        std::thread clientThread(handleClient, clientSocket);
        clientThread.detach();  
    }
    
    // 清理
    close(serverSocket);
    
    if (camThread.joinable()) {
        camThread.join();
    }
    
    std::cout << "Server shut down" << std::endl;
    return 0;
}