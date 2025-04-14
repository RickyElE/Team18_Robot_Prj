#include "ppr_chassis_node.h"
#include <iostream>

#include <thread>



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
