#include "ppr_remote_node.h"

int main(){
    std::cout << "Starting ppr_master_node." << std::endl;

    ppr_remote_node mynode;
    mynode.start();

    return 0;
}