#include "ppr_chassis_node.h"


int main(){
    std::cout << "Starting ppr_chassis_node." << std::endl;

    ppr_chassis_node mynode;
    if(!mynode.init())
    {
        return 1;
    }
    mynode.run(mynode);

    return 0;
}