#include "ppr_master_node.h"

int main(int, char**){
    std::cout << "Starting ppr_master_node." << std::endl;

    ppr_master_node mynode;
    if(!mynode.init())
    {
        return 1;
    }
    mynode.run(mynode);

    return 0;
}