#include "libcam2web.h"
#include <iostream>
#include <string>
#include <csignal>

// 全局服務器實例用於信號處理
LibCam2Web* server = nullptr;

// 信號處理程序以優雅地關閉
void signalHandler(int signum) {
    std::cout << "接收到中斷信號 (" << signum << ")。\n";
    
    // 清理並關閉服務器
    if (server != nullptr) {
        server->stop();
    }
    
    exit(signum);
}

// 主程序
int main(int argc, char* argv[]) {
    // 註冊信號處理程序
    signal(SIGINT, signalHandler);
    
    // 解析命令行參數
    int port = 8080;  // 默認端口
    Libcam2OpenCVSettings settings;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--port" && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        } else if (arg == "--width" && i + 1 < argc) {
            settings.width = std::stoi(argv[++i]);
        } else if (arg == "--height" && i + 1 < argc) {
            settings.height = std::stoi(argv[++i]);
        } else if (arg == "--framerate" && i + 1 < argc) {
            settings.framerate = std::stoi(argv[++i]);
        } else if (arg == "--brightness" && i + 1 < argc) {
            settings.brightness = std::stof(argv[++i]);
        } else if (arg == "--contrast" && i + 1 < argc) {
            settings.contrast = std::stof(argv[++i]);
        } else if (arg == "--help") {
            std::cout << "使用方法: " << argv[0] << " [選項]\n"
                     << "選項:\n"
                     << "  --port PORT          網頁服務器端口 (默認: 8080)\n"
                     << "  --width WIDTH        攝像頭寬度 (默認: 自動)\n"
                     << "  --height HEIGHT      攝像頭高度 (默認: 自動)\n"
                     << "  --framerate FPS      攝像頭幀率 (默認: 自動)\n"
                     << "  --brightness VALUE   攝像頭亮度 (-1.0 到 1.0, 默認: 0.0)\n"
                     << "  --contrast VALUE     攝像頭對比度 (0.0 到 2.0, 默認: 1.0)\n"
                     << "  --help               顯示此幫助消息\n";
            return 0;
        }
    }
    
    // 創建並啟動服務器
    server = new LibCam2Web(port);
    
    try {
        std::cout << "啟動攝像頭，設置為:\n"
                 << "  寬度: " << (settings.width ? std::to_string(settings.width) : "自動") << "\n"
                 << "  高度: " << (settings.height ? std::to_string(settings.height) : "自動") << "\n"
                 << "  幀率: " << (settings.framerate ? std::to_string(settings.framerate) : "自動") << "\n"
                 << "  亮度: " << settings.brightness << "\n"
                 << "  對比度: " << settings.contrast << "\n";
        
        server->start(settings);
        
        // 等待用戶按Enter退出
        std::cout << "按Enter停止服務器..." << std::endl;
        std::cin.get();
        
        server->stop();
    } catch (const std::exception& e) {
        std::cerr << "錯誤: " << e.what() << std::endl;
    }
    
    delete server;
    return 0;
}