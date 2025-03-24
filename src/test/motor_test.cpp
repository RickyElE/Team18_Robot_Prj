#include "motor.h"
#include "delay.h"
int main(){
    Motor motor = Motor();
    Delay delay = Delay();
    if (!motor.init()){
        return -1;
    }

    motor.Forward();
    delay.delay_ms(2000);
    motor.Backward();
    delay.delay_ms(2000);
    motor.TurnLeft();
    delay.delay_ms(2000);
    motor.TurnRight();
    delay.delay_ms(2000);
    motor.Stop();
    return 0;

}