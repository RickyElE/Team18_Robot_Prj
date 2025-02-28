#include <iostream>   //標準輸入輸出
#include <string>  
#include <thread>     //多線程
#include <mutex>      //線程同步
#include <vector>
#include <atomic>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <fstream>     //文件輸入輸出

// 全局變量
std::mutex frameMutex;     //確保同一時間只有一個可以存取frame
std::vector<uint8_t> latestFrame;
std::atomic<bool> running{true};    //boolean初始為true初始為true
const int PORT = 8080;        //服務器監聽窗口

// 函數聲明
void cameraThread();        //攝像頭捕獲線程
void handleClient(int clientSocket);     //客戶端連接
std::string createHTTPResponse(const std::string& content, const std::string& contentType);    //http響應

// HTML內容
const char* indexHTML = R"(
<!DOCTYPE html>
<html>
<head>
    <title>Raspberry Pi Camera Stream</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; }
        .container { margin-top: 30px; }
        img { max-width: 100%; border: 1px solid #ddd; }
    </style>
</head>
<body>
    <div class="container">
        <h1>Raspberry Pi Camera Stream</h1>
        <img src="/video_feed" alt="Video Stream">
    </div>
</body>
</html>
)";

// 使用libcamera-still捕獲圖像
void cameraThread() {
    // 創建臨時文件路徑
    const char* tempJpegPath = "/tmp/pi_camera_stream.jpg";    //定義臨時JPEG文件路徑
    char command[256];     //將命令陣列大小固定為256
    
    while (running) {
        // 使用libcamera-still命令捕獲一張圖像
        snprintf(command, sizeof(command), 
                 "libcamera-still -n --immediate --width 640 --height 480 --output %s --timeout 1", 
                 tempJpegPath);                //解析度為640*480    //immediate: 立即拍照    //timeout 1 : 設定曝光時間為1豪秒
                                               //libcamera-still 是raspberry pi 上的攝像機控制工具
        
        // 執行命令
        int result = system(command);
        if (result != 0) {       //如果執行失敗
            std::cerr << "Failed to capture image with libcamera-still" << std::endl;     //錯誤提示
            std::this_thread::sleep_for(std::chrono::seconds(1));        //讓目前執行的thread暫停一秒
            continue;  
        }
        
        // 讀取JPEG文件
        std::ifstream file(tempJpegPath, std::ios::binary | std::ios::ate);    
        //temJpegPath檔案路徑      //用二進位的方式開啟檔案    //從檔案末尾開始讀取，以獲取檔案大小

        if (!file.is_open()) {    //如果沒有打開
            std::cerr << "Failed to open captured image file" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        
        // 獲取文件大小
        std::streamsize size = file.tellg();    //取得目前讀取位置，即前面設定的末尾
        file.seekg(0, std::ios::beg);      //讀取位置重置回開頭
        
        // 讀取文件內容
        std::vector<uint8_t> buffer(size);     //創建足夠大的緩衝區
        if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {    
            std::cerr << "Failed to read image file" << std::endl;
            std::this_thread::sleep_for(std::chrono::seconds(1));
            continue;
        }
        file.close();
        
        // 更新最新幀
        std::lock_guard<std::mutex> lock(frameMutex);     //加索保護共享數據
        latestFrame = buffer;     //將讀取到的圖像更新為最新幀
        
        // 短暫延遲控制幀率
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

// 創建HTTP響應
std::string createHTTPResponse(const std::string& content, const std::string& contentType) {
    std::string response = "HTTP/1.1 200 OK\r\n";     //使用http 1.1 協議   //回應狀態碼200，表示成功
    response += "Content-Type: " + contentType + "\r\n";      //回應客戶端的形式
    response += "Content-Length: " + std::to_string(content.length()) + "\r\n";    //回應內容長度
    response += "Connection: keep-alive\r\n";       //一直保持連線
    response += "\r\n";      //http協定，要把header跟body分開
    response += content;     //body
    
    return response;
}

// 處理客戶端連接
void handleClient(int clientSocket) {
    const int BUFFER_SIZE = 1024;     //定義緩衝區大小
    char buffer[BUFFER_SIZE];        //創建接收緩衝區
    
    // 接收HTTP請求
    ssize_t bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);    //從客戶端讀取數據
    if (bytesRead <= 0) {        //檢查是否成功讀取
        close(clientSocket);
        return;
    }
    
    buffer[bytesRead] = '\0';
    std::string request(buffer);
    
    // 解析請求以確定端點
    if (request.find("GET / HTTP") != std::string::npos) {      //檢查是否為跟路徑請求
        // 提供index.html
        std::string response = createHTTPResponse(indexHTML, "text/html");     //創建html響應
        send(clientSocket, response.c_str(), response.length(), 0);    //發送響應
        close(clientSocket);     //關閉客戶端連接
    }
    else if (request.find("GET /video_feed HTTP") != std::string::npos) {     //檢查是否為視頻流請求
        // 視頻串流端點
        const char* header = "HTTP/1.1 200 OK\r\n"
                            "Content-Type: multipart/x-mixed-replace; boundary=frame\r\n"
                            "Cache-Control: no-cache\r\n"
                            "Connection: close\r\n"
                            "\r\n";
        
        send(clientSocket, header, strlen(header), 0);    //發送MJPEG流頭部
        
        while (running) {
            // 獲取最新幀
            std::vector<uint8_t> frame;
            {
                std::lock_guard<std::mutex> lock(frameMutex);
                frame = latestFrame;    //複製最新帧
            }
            
            if (!frame.empty()) {
                // 創建MJPEG幀頭部
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
            std::this_thread::sleep_for(std::chrono::milliseconds(100));   //控制發送頻率
        }
        
        close(clientSocket);
    }
    else {
        // 處理404 Not Found
        std::string notFound = "HTTP/1.1 404 Not Found\r\n"
                              "Content-Type: text/plain\r\n"
                              "Content-Length: 9\r\n"
                              "\r\n"
                              "Not Found";
        
        send(clientSocket, notFound.c_str(), notFound.length(), 0);
        close(clientSocket);
    }
}

// 處理信號
void signalHandler(int signum) {
    running = false;     //停止程序
}

int main() {
    // 設置信號處理
    signal(SIGINT, signalHandler);     //設定按下ctrl+c時停止程序
    signal(SIGTERM, signalHandler);    //設定按下SIGTERM時停止程序
    
    // 創建服務器套接字     //socket : 網路電話線，可以讓程式間互相通訊
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);     //使用IPv4網路協議  //tcp(sock_stream)資料傳輸
    if (serverSocket < 0) {      //創建失敗
        std::cerr << "Error creating socket" << std::endl;
        return 1;
    }
    
    // 設置套接字選項
    int opt = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        std::cerr << "Error setting socket options" << std::endl;
        return 1;
    }
    
    // 設置服務器地址
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);
    
    // 綁定套接字
    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Error binding socket" << std::endl;
        return 1;
    }
    
    // 監聽連接
    if (listen(serverSocket, 10) < 0) {
        std::cerr << "Error listening on socket" << std::endl;
        return 1;
    }
    
    std::cout << "Server started at http://0.0.0.0:" << PORT << std::endl;
    
    // 啟動攝像頭線程
    std::thread camThread(cameraThread);
    
    // 接受和處理客戶端連接
    while (running) {
        struct sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        
        // 設置超時以允許檢查running標誌
        fd_set readSet;
        FD_ZERO(&readSet);
        FD_SET(serverSocket, &readSet);
        
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        
        int ready = select(serverSocket + 1, &readSet, NULL, NULL, &timeout);
        if (ready <= 0) {
            // 超時或錯誤
            continue;
        }
        
        int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
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