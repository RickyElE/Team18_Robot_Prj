#include "main.h"
#include <atomic>
#include <cmath>
#include <csignal>

std::atomic_bool keep_running = true;

void signal_handler(int signum) {
    std::cout << "\n[INFO] Signal received (" << signum << "), exiting..." << std::endl;
    keep_running = false;
}

int main(){
    WebsiteServer website_server;
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    if (!website_server.start()){
        std::cerr << "website cannot start!" << std::endl;
        return -1;
    }
    while(keep_running){
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    website_server.stop();
    return 0;
}