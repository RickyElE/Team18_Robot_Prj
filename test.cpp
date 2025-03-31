#include <iostream>
#include <chrono>
#include <thread>
#include "PCA9685.h"  

// 设置 PCA9685 有效通道数
const int NUM_CHANNELS = 16;

// 设置指定通道伺服角度的函数
// 参数：pca    - PCA9685 对象
//       channel - 伺服所在的通道（0-15）
//       angle   - 角度（0~180）
void set_angle(PCA9685 &pca, int channel, int angle) {
    // 伺服参数（单位：微秒）
    const int min_pulse = 500;
    const int max_pulse = 2400;
    const int actuation_range = 180;  // 伺服运动范围

    // 根据角度计算脉宽
    int pulse = min_pulse + (max_pulse - min_pulse) * angle / actuation_range;
    
    // PCA9685 分辨率为 4096，50Hz 时周期为20000微秒，
    // 将脉宽转换为计数值
    int ticks = static_cast<int>((pulse / 20000.0) * 4096);
    
    // 设置对应通道的 PWM 输出（on=0, off=ticks）
    pca.setPWM(channel, 0, ticks);
}

// 清零所有通道的 PWM 信号（停止输出）
void clear_all_channels(PCA9685 &pca) {
    for (int channel = 0; channel < NUM_CHANNELS; channel++) {
        pca.setPWM(channel, 0, 0);
    }
}


// 假设 set_angle 函数已经在其他地方定义，类似如下形式：
// void set_angle(PCA9685 &pca, int channel, int angle);

//初始化机械臂各个位置
void initial_robarm(PCA9685 &pca) {
    set_angle(pca, 0, 180);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    set_angle(pca, 1, 180);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    set_angle(pca, 2, 90);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    set_angle(pca, 3, 100);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
}


int main() {
    // 初始化 PCA9685 对象，指定 I2C 地址为 0x5f（默认地址通常为 0x40）
    PCA9685 pca(0x5f);

    // 使用 init() 初始化 I2C 总线并配置 PCA9685
    if (!pca.init()) {
        std::cerr << "初始化 PCA9685 失败！" << std::endl;
        return 1;
    }

    // 设置 PWM 频率为 50Hz，适合伺服电机
    pca.setPWMFreq(50);

    // 手动清零所有通道，确保开始前PWM信号停止发送
    clear_all_channels(pca);
    pca.wakeup();

    // 示例：控制通道 0 的伺服运动到 150°
    set_angle(pca, 0, 100);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    set_angle(pca, 1, 100);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    set_angle(pca, 2, 150);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    set_angle(pca, 3, 160);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    initial_robarm(pca);
    // 如果需要测试所有通道的运动，可以调用 test() 函数
    // test(pca);

    // 清零所有通道，停止 PWM 输出
    clear_all_channels(pca);

    // 此处不需要显式调用 close()，析构函数会自动释放资源
    return 0;
}
