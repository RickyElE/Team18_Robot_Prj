#include "ppr_chassis_node.h"
#include <iostream>

#include <thread>


// 修改 `ppr_chassis_node::run()`，讓它發送 WebSocket 訊息
// void ppr_chassis_node::run() {
//     // std::thread ws_thread(start_websocket_server); // 啟動 WebSocket 伺服器
//     // ws_thread.detach(); // 分離執行緒，避免阻塞

//     std::cout << "ppr_chassis_node is running..." << std::endl;

//     while (running_) {
//         float voltage = bms.getVoltage();
//         float percentage = bms.getPercentage();
        
//         // 發佈數據到 Fast DDS
//         publish_battery_status(voltage, percentage);
        
//         // 發送數據到 WebSocket
//         // send_battery_status(voltage, percentage);

//         // std::this_thread::sleep_for(std::chrono::seconds(1));
//     }
// }

// 主程式啟動 `ppr_chassis_node`
int main() {
    std::cout << "Starting ppr_chassis_node." << std::endl;

    ppr_chassis_node mynode;
    if (!mynode.init()) {
        std::cerr << "Failed to initialize ppr_chassis_node." << std::endl;
        return 1;
    }

    mynode.start();
    std::cout << "Press Enter to stop...\n";
    std::cin.get();
    mynode.stop();

    return 0;
}
