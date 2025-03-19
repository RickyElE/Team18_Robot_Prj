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
#include <cstdlib>
#include <ctime>

// 全局變量
std::mutex frameMutex;
std::vector<uint8_t> latestFrame;
std::atomic<bool> running{true};
const int PORT = 8081;
std::string webroot = "/home/Team18_Pi/website/Team18_Robot_Prj/website"; // 網頁文件所在的路徑
//std::string webroot = "/home/Team18_Pi/website/resi_website"; // 網頁文件所在的路徑

// 函數聲明
void cameraThread();
void handleClient(int clientSocket, struct sockaddr_in clientAddr);
std::string getContentType(const std::string& path);
bool fileExists(const std::string& path);

// 處理信號
void signalHandler(int signum) {
    std::cout << "Stopping server..." << std::endl;
    running = false;
    
    // 停止所有攝像頭進程
    system("sudo pkill -f libcamera");
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


void cameraThread() {
    std::cout << "Camera thread starting..." << std::endl;
    
    // 清理可能佔用攝像頭的進程
    system("sudo pkill -f libcamera");
    system("sudo pkill -f raspistill");
    std::this_thread::sleep_for(std::chrono::seconds(2));

    int frameCount = 0;    //計算拍攝的影像數量
    const std::string outputPath = "/tmp/camera_frame.jpg";    //影像暫存路徑
    
    while (running) {

        bool captureSuccess = false;
        
        // 首先嘗試使用 libcamera-still
        std::string cmd = "libcamera-still -n -o " + outputPath + " --width 320 --height 240 --immediate --timeout 1 2>/dev/null";  //將拍攝結果儲存到 /tmp/camera_frame.jpg
        int result = system(cmd.c_str());
        
        // 檢查是否成功捕獲
        struct stat fileStat;

        if (result == 0 && stat(outputPath.c_str(), &fileStat) == 0 && fileStat.st_size > 0) {   //如果讀取成功

            std::ifstream file(outputPath, std::ios::binary | std::ios::ate);

            if (file.is_open()) {
                std::streamsize size = file.tellg();
                file.seekg(0, std::ios::beg);
                
                std::vector<uint8_t> frame(size);
                if (file.read(reinterpret_cast<char*>(frame.data()), size)) {
                    // 更新最新幀
                    std::lock_guard<std::mutex> lock(frameMutex);
                    latestFrame = frame;                            //如果成功讀取，就把影像存入lastestFrame

                    if (frameCount % 5 == 0) {
                        std::cout << "Frame " << frameCount << " captured, size: " << size << " bytes" << std::endl;
                    }
                    captureSuccess = true;
                }
                file.close();
            }
        }
        
        // 如果使用 libcamera-still 失敗，嘗試 raspistill
        if (!captureSuccess) {
            cmd = "raspistill -n -o " + outputPath + " -w 320 -h 240 -t 100 2>/dev/null";
            result = system(cmd.c_str());
            
            if (result == 0 && stat(outputPath.c_str(), &fileStat) == 0 && fileStat.st_size > 0) {
                std::ifstream file(outputPath, std::ios::binary | std::ios::ate);
                if (file.is_open()) {
                    std::streamsize size = file.tellg();
                    file.seekg(0, std::ios::beg);
                    
                    std::vector<uint8_t> frame(size);
                    if (file.read(reinterpret_cast<char*>(frame.data()), size)) {
                        // 更新最新幀
                        std::lock_guard<std::mutex> lock(frameMutex);
                        latestFrame = frame;
                        if (frameCount % 5 == 0) {
                            std::cout << "Frame " << frameCount << " captured using raspistill, size: " << size << " bytes" << std::endl;
                        }
                        captureSuccess = true;
                    }
                    file.close();
                }
            }
        }
        
        // 如果仍然失敗，嘗試生成一個測試圖像
        if (!captureSuccess) {
            // 使用初始化的測試圖像
            if (latestFrame.empty()) {
                std::cout << "No frame captured and no test pattern available" << std::endl;
            } else if (frameCount % 5 == 0) {
                std::cout << "Using test pattern, size: " << latestFrame.size() << " bytes" << std::endl;
            }
        }
        
        frameCount++;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    
    std::cout << "Camera thread stopped" << std::endl;
}

// 處理客戶端連接
void handleClient(int clientSocket, struct sockaddr_in clientAddr) {

    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    
    // 接收 HTTP 請求
    ssize_t bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
    if (bytesRead <= 0) {
        close(clientSocket);
        return;
    }
    
    buffer[bytesRead] = '\0';
    std::string request(buffer);
    
    // 解析 HTTP 請求
    std::string path;
    if (request.find("GET") == 0) {
        size_t pathStart = request.find(" ") + 1;
        size_t pathEnd = request.find(" ", pathStart);
        std::string fullPath = request.substr(pathStart, pathEnd - pathStart);
        
        // 分離路徑和查詢參數
        size_t queryPos = fullPath.find('?');
        if (queryPos != std::string::npos) {
            path = fullPath.substr(0, queryPos);
        } else {
            path = fullPath;
        }
        
        // 輸出調試信息
        std::cout << "Received request for path: '" << path << "'" << std::endl;
        
        // 默認首頁
        if (path == "/") {
            path = "/website.html";  // 使用 website.html 作為默認首頁
        }
    }
    
    // 處理視頻流請求
    if (path == "/video_feed") {
        std::cout << "Video feed request received from " << inet_ntoa(clientAddr.sin_addr) << std::endl;
        
        // 獲取最新幀
        std::vector<uint8_t> frame;
        {
            std::lock_guard<std::mutex> lock(frameMutex);
            frame = latestFrame;
        }
        
        std::cout << "Sending frame with size: " << frame.size() << " bytes" << std::endl;
        
        // 構建 HTTP 回應 - 使用靜態 JPEG 而非 MJPEG 串流
        std::string header = "HTTP/1.1 200 OK\r\n"
                             "Content-Type: image/jpeg\r\n"
                             "Content-Length: " + std::to_string(frame.size()) + "\r\n"
                             "Cache-Control: no-cache, no-store, must-revalidate\r\n"
                             "Pragma: no-cache\r\n"
                             "Expires: 0\r\n"
                             "Access-Control-Allow-Origin: *\r\n"
                             "Connection: close\r\n"
                             "\r\n";
        
        // 發送回應
        send(clientSocket, header.c_str(), header.length(), 0);
        
        // 如果有幀，發送幀數據
        if (!frame.empty()) {
            send(clientSocket, frame.data(), frame.size(), 0);
            std::cout << "Frame sent successfully" << std::endl;
        } else {
            std::cout << "Warning: Empty frame, nothing sent" << std::endl;
        }
        
        // 關閉連接
        close(clientSocket);
        return;
    }
    
    // 處理控制請求
    else if (path.find("/control") == 0) {
        // 解析方向參數
        std::string fullPath;
        size_t pathStart = request.find(" ") + 1;
        size_t pathEnd = request.find(" ", pathStart);
        fullPath = request.substr(pathStart, pathEnd - pathStart);
        
        std::string dir;
        size_t dirPos = fullPath.find("dir=");
        if (dirPos != std::string::npos) {
            dir = fullPath.substr(dirPos + 4);
            // 如果URL有其他參數，截取到下一個&
            size_t ampPos = dir.find("&");
            if (ampPos != std::string::npos) {
                dir = dir.substr(0, ampPos);
            }
        }
        
        std::cout << "Control command received: " << dir << std::endl;
        
        // 這裡可以添加樹莓派的硬件控制代碼，例如使用GPIO控制電機
        
        // 返回成功響應
        std::string response = "HTTP/1.1 200 OK\r\n"
                              "Content-Type: application/json\r\n"
                              "Access-Control-Allow-Origin: *\r\n"
                              "Content-Length: 16\r\n"
                              "Connection: close\r\n"
                              "\r\n"
                              "{\"status\":\"ok\"}";
        
        send(clientSocket, response.c_str(), response.length(), 0);
        close(clientSocket);
        return;
    }
    // 處理靜態文件請求
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
                                        "Access-Control-Allow-Origin: *\r\n"
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
                                  "Access-Control-Allow-Origin: *\r\n"
                                  "Content-Length: 9\r\n"
                                  "\r\n"
                                  "Not Found";
            
            std::cout << "File not found: " << filepath << std::endl;
            send(clientSocket, notFound.c_str(), notFound.length(), 0);
        }
        
        close(clientSocket);
    }
}

int main() {
    try {
        std::cout << "Starting server initialization..." << std::endl;
        
        // 獲取當前進程ID，避免殺死自己
        pid_t current_pid = getpid();
        std::cout << "Current process ID: " << current_pid << std::endl;
        
        // 逐行執行和檢查每個初始化命令
        std::cout << "About to kill libcamera processes..." << std::endl;
        int libcam_result = system("sudo pkill -f libcamera");
        std::cout << "pkill libcamera returned: " << libcam_result << std::endl;
        
        // 跳過 camera_stream 進程的清理，避免自我終止
        std::cout << "Skipping camera_stream process cleanup (to avoid killing ourselves)" << std::endl;
        
        std::cout << "About to kill raspistill processes..." << std::endl;
        int rapi_result = system("sudo pkill -f raspistill");
        std::cout << "pkill raspistill returned: " << rapi_result << std::endl;
        
        std::cout << "Sleeping for 2 seconds..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(2));
        std::cout << "Sleep complete" << std::endl;
        
        // 檢查網頁目錄是否存在
        std::cout << "Checking if web root exists: " << webroot << std::endl;
        struct stat buffer;
        if (stat(webroot.c_str(), &buffer) != 0) {
            std::cerr << "Error: Web root directory does not exist: " << webroot << std::endl;
            std::cerr << "Error code: " << errno << " - " << strerror(errno) << std::endl;
            return 1;
        }
        
        std::cout << "Web root directory exists" << std::endl;
        
        // 設置信號處理
        std::cout << "Setting up signal handlers..." << std::endl;
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
        std::cout << "Signal handlers set" << std::endl;
        
        // 創建服務器套接字
        std::cout << "Creating server socket..." << std::endl;
        int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
        if (serverSocket < 0) {
            std::cerr << "Error creating socket: " << strerror(errno) << std::endl;
            return 1;
        }
        std::cout << "Server socket created successfully" << std::endl;
        
        // 設置套接字選項
        std::cout << "Setting socket options..." << std::endl;
        int opt = 1;
        if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
            std::cerr << "Error setting SO_REUSEADDR: " << strerror(errno) << std::endl;
            close(serverSocket);
            return 1;
        }
        
        // 添加 SO_REUSEPORT 選項以避免"地址已在使用"錯誤
        if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) < 0) {
            std::cerr << "Warning: Error setting SO_REUSEPORT option: " << strerror(errno) << " (continuing anyway)" << std::endl;
        }
        
        std::cout << "Socket options set successfully" << std::endl;
        
        // 設置服務器地址
        std::cout << "Setting up server address on port " << PORT << "..." << std::endl;
        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof(serverAddr));
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        serverAddr.sin_port = htons(PORT);
        
        // 綁定套接字
        std::cout << "Binding socket..." << std::endl;
        if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
            std::cerr << "Error binding socket: " << strerror(errno) << std::endl;
            // 檢查是否因為端口已被使用
            if (errno == EADDRINUSE) {
                std::cerr << "Port " << PORT << " is already in use. Try killing any process using it with: sudo fuser -k " << PORT << "/tcp" << std::endl;
            }
            close(serverSocket);
            return 1;
        }
        
        std::cout << "Socket bound to port " << PORT << " successfully" << std::endl;
        
        // 監聽連接
        std::cout << "Starting to listen for connections..." << std::endl;
        if (listen(serverSocket, 10) < 0) {
            std::cerr << "Error listening on socket: " << strerror(errno) << std::endl;
            close(serverSocket);
            return 1;
        }
        
        std::cout << "Server started at http://0.0.0.0:" << PORT << std::endl;
        std::cout << "Serving files from: " << webroot << std::endl;
        
        // 啟動攝像頭線程
        std::cout << "Starting camera thread..." << std::endl;
        std::thread camThread(cameraThread);
        std::cout << "Camera thread started" << std::endl;
        
        std::cout << "Main server loop starting..." << std::endl;
        
        // 接受和處理客戶端連接
        while (running) {
            struct sockaddr_in clientAddr;
            socklen_t clientAddrLen = sizeof(clientAddr);
            
            // 設置超時以允許檢查 running 標誌
            fd_set readSet;
            FD_ZERO(&readSet);
            FD_SET(serverSocket, &readSet);
            
            struct timeval timeout;
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;
            
            int ready = select(serverSocket + 1, &readSet, NULL, NULL, &timeout);
            if (ready <= 0) {
                // 超時或錯誤
                if (ready < 0) {
                    std::cerr << "Select error: " << strerror(errno) << std::endl;
                }
                continue;
            }
            
            std::cout << "Accepting new connection..." << std::endl;
            int clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
            if (clientSocket < 0) {
                std::cerr << "Error accepting connection: " << strerror(errno) << std::endl;
                continue;
            }
            
            std::cout << "New connection from " << inet_ntoa(clientAddr.sin_addr) << ":" << ntohs(clientAddr.sin_port) << std::endl;
            
            // 在單獨的線程中處理客戶端
            std::thread clientThread(handleClient, clientSocket, clientAddr);
            clientThread.detach();
        }
        
        // 清理
        std::cout << "Shutting down server..." << std::endl;
        close(serverSocket);
        
        std::cout << "Waiting for camera thread to finish..." << std::endl;
        if (camThread.joinable()) {
            camThread.join();
        }
        
        // 確保攝像頭進程停止
        std::cout << "Stopping any remaining camera processes..." << std::endl;
        system("sudo pkill -f libcamera-vid");
        
        std::cout << "Server shut down successfully" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Exception caught in main: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Unknown exception caught in main" << std::endl;
        return 1;
    }
}