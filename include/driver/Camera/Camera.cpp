#include "Camera.h"


int main(){
    Camera test;
    if(!test.start()){
        std::cerr << "Failed to start Camera node" << std::endl;
        return -1;
    }
    return 0;
}