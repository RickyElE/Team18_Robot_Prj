#include "motor.h"
#include "delay.h"
int main(){
    Motor motor = Motor();
    Delay delay = Delay();
    if (!motor.init()){
        return -1;
    }
    std::cout << "Forward!" << std::endl;
    motor.Forward();
    delay.delay_ms(2000);
    std::cout << "Backward!" << std::endl;
    motor.Backward();
    delay.delay_ms(2000);
    std::cout << "TurnLeeft!" << std::endl;
    motor.TurnLeft();
    delay.delay_ms(2000);
    std::cout << "TurnRight!" << std::endl;
    motor.TurnRight();
    delay.delay_ms(2000);
    std::cout << "Stop!" << std::endl;
    motor.Stop();
    return 0;

}