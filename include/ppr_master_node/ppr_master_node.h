#ifndef _PPR_MASTER_NODE_H_
#define _PPR_MASTER_NODE_H_


#include "ChassisSubscriber.h"

#include <thread>
#include <chrono>

class ppr_master_node
{
public:
    ChassisSubscriber* chassisSubscriber = new ChassisSubscriber();

    ppr_master_node(){};
    virtual ~ppr_master_node(){};
    void run(ppr_master_node &mynode);

    bool init();
    void subscribe(std::function<void()> callback);


};

bool ppr_master_node::init(){
    std::cout << "Initializing ppr_master_node" << std::endl;
    if (!chassisSubscriber->init())
    {
        std::cout << "Failed to initialize subscriber" << std::endl;
        return false;
    }
    return true;
}

void ppr_master_node::subscribe(std::function<void()> callback){
    callback();
}

void ppr_master_node::run(ppr_master_node &mynode){
    std::cout << "Running ppr_master_node" << std::endl;
    
    while(true){
        mynode.subscribe(std::bind(&ChassisSubscriber::subscribe, mynode.chassisSubscriber));
    }
}
#endif