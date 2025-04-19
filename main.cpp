#include "main.h"
#include <iostream>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>

// 全局變量
std::atomic_bool keep_running = true;

// 信號處理函數
void signal_handler(int signum) {
    std::cout << "\n[INFO] 接收到中斷信號 (" << signum << ")，退出中..." << std::endl;
    keep_running = false;
}

int main(int argc, char* argv[]) {
    // 註冊信號處理
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    // 預設設定
    int port = 8081;
    

    try {
        // 初始化 WebSocket 服務器
        std::cout << "初始化服務器..." << std::endl;
        WebsiteServer website_server;

        // 設置攝像頭參數
        // website_server.setCameraSettings(port, settings);

        // 啟動所有服務
        if (!website_server.start()) {
            std::cerr << "服務器無法啟動！" << std::endl;
            return 1;
        }

        // 主循環，保持程序運行直到收到信號
        std::cout << "所有服務已啟動。按 Ctrl+C 停止..." << std::endl;
        while (keep_running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        // 程序即將結束，停止服務
        std::cout << "正在停止服務..." << std::endl;
        website_server.stop();

        std::cout << "所有服務已停止。" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "錯誤: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
