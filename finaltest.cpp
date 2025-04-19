#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>       // 新增：支持数学运算
#include "PCA9685.h"  

// 设置 PCA9685 有效通道数
const int NUM_CHANNELS = 16;

//-------------------------
// 原有的 PCA9685 控制函数
//-------------------------

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

// 初始化机械臂各个位置
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

//------------------------------------------------------
// 新增：将 Python 中运动控制逻辑转换后的代码
//------------------------------------------------------

// 定义二维坐标结构体
struct Point2D {
    double x;
    double y;
};

// 全局变量：机械臂各个关键点位置
Point2D current_A;
Point2D current_B;
Point2D current_C;

// 初始化初始位置，返回三个二维坐标点
// current_A: 如 (-29.64, 28.5)
// current_B: 如 (-79, 0)
// current_C: 如 (0, 0)
void init_position() {
    current_A = { -29.64, 28.5 };
    current_B = { -79, 0 };
    current_C = { 0, 0 };
}

// 根据运动参数计算角度，并更新全局位置
// 参数 is_horizontal: 表示是否为水平运动（目前没有单独处理，仅用于区分调用时语义）
// deltaX, deltaY: 位移量
// len_AB, len_BC: 机械臂连杆长度（固定值）
std::pair<double, double> Angle_calculate(bool is_horizontal = true, double deltaX = 10, double deltaY = 10, double len_AB = 57.0, double len_BC = 79.0) {
    // 根据运动类型计算目标位置 target_A
    Point2D target_A;
    target_A.x = current_A.x + deltaX;
    target_A.y = current_A.y + deltaY;
    
    // 计算从 current_C 到 target_A 的距离（边 AC 的长度）
    double len_AC = std::sqrt((target_A.x - current_C.x) * (target_A.x - current_C.x) +
                              (target_A.y - current_C.y) * (target_A.y - current_C.y));
    
    // 利用余弦定理计算 theta2：连杆 AB 与 BC 之间的夹角
    double theta2 = std::acos((len_AB * len_AB + len_BC * len_BC - len_AC * len_AC) / (2 * len_AB * len_BC));
    
    // 计算 theta1：利用 atan2 得到 target_A 的方向，并减去余弦定理算出的值
    double theta1 = std::atan2(target_A.y, target_A.x) - 
                    std::acos((len_BC * len_BC + len_AC * len_AC - len_AB * len_AB) / (2 * len_BC * len_AC));
    
    // 根据几何关系对角度进行修正，并转换为度
    double f_angle1 = 180.0 - (theta1 * 180.0 / M_PI);
    double f_angle2 = 210.0 - (theta2 * 180.0 / M_PI);
    
    // 根据 theta1 计算 target_B，假设 target_B 距离 current_C 为 len_BC
    Point2D target_B;
    target_B.x = len_BC * std::cos(theta1);
    target_B.y = len_BC * std::sin(theta1);
    
    // 更新全局状态
    current_A = target_A;
    current_B = target_B;
    
    return std::make_pair(f_angle1, f_angle2);
}

// 向前运动：水平运动中 x 坐标增加
void MoveForward(PCA9685 &pca) {
    auto angles = Angle_calculate(true, 10, 0, 57.0, 79.0);
    std::cout << "[MoveForward] f_angle1: " << angles.first << "  f_angle2: " << angles.second << std::endl;
    // 根据计算结果驱动伺服
    set_angle(pca, 0, static_cast<int>(angles.first));
    set_angle(pca, 1, static_cast<int>(angles.second));
}

// 向后运动：水平运动中 x 坐标减少
void MoveBackward(PCA9685 &pca) {
    auto angles = Angle_calculate(true, -10, 0, 57.0, 79.0);
    std::cout << "[MoveBackward] f_angle1: " << angles.first << "  f_angle2: " << angles.second << std::endl;
    set_angle(pca, 0, static_cast<int>(angles.first));
    set_angle(pca, 1, static_cast<int>(angles.second));
}

// 向上运动：竖直运动中 y 坐标增加
void MoveUp(PCA9685 &pca) {
    auto angles = Angle_calculate(false, 0, 10, 57.0, 79.0);
    std::cout << "[MoveUp] f_angle1: " << angles.first << "  f_angle2: " << angles.second << std::endl;
    set_angle(pca, 0, static_cast<int>(angles.first));
    set_angle(pca, 1, static_cast<int>(angles.second));
}

// 向下运动：竖直运动中 y 坐标减少
void MoveDown(PCA9685 &pca) {
    auto angles = Angle_calculate(false, 0, -10, 57.0, 79.0);
    std::cout << "[MoveDown] f_angle1: " << angles.first << "  f_angle2: " << angles.second << std::endl;
    set_angle(pca, 0, static_cast<int>(angles.first));
    set_angle(pca, 1, static_cast<int>(angles.second));
}

//------------------------------------------------------
// main() 函数：整合 PCA9685 与机械臂运动控制逻辑
//------------------------------------------------------
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

    // 示例：控制通道 0-3 的伺服运动初始测试
    set_angle(pca, 0, 100);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    set_angle(pca, 1, 100);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    set_angle(pca, 2, 150);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    set_angle(pca, 3, 160);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    // 调用初始机械臂动作
    initial_robarm(pca);

    //--------- 新增：机械臂运动控制逻辑演示 ---------
    // 初始化机械臂位置状态（类似于 Python 中的 init_position()）
    init_position();

    // 打印初始状态
    std::cout << "初始状态:" << std::endl;
    std::cout << "current_A: (" << current_A.x << ", " << current_A.y << ")" << std::endl;
    std::cout << "current_B: (" << current_B.x << ", " << current_B.y << ")" << std::endl;

    // 向前移动
    std::cout << "\n向前移动:" << std::endl;
    MoveForward(pca);
    std::cout << "更新后的 current_A: (" << current_A.x << ", " << current_A.y << ")" << std::endl;
    std::cout << "更新后的 current_B: (" << current_B.x << ", " << current_B.y << ")" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    // 向后移动
    std::cout << "\n向后移动:" << std::endl;
    MoveBackward(pca);
    std::cout << "更新后的 current_A: (" << current_A.x << ", " << current_A.y << ")" << std::endl;
    std::cout << "更新后的 current_B: (" << current_B.x << ", " << current_B.y << ")" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    // 向上移动
    std::cout << "\n向上移动:" << std::endl;
    MoveUp(pca);
    std::cout << "更新后的 current_A: (" << current_A.x << ", " << current_A.y << ")" << std::endl;
    std::cout << "更新后的 current_B: (" << current_B.x << ", " << current_B.y << ")" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    // 向下移动
    std::cout << "\n向下移动:" << std::endl;
    MoveDown(pca);
    std::cout << "更新后的 current_A: (" << current_A.x << ", " << current_A.y << ")" << std::endl;
    std::cout << "更新后的 current_B: (" << current_B.x << ", " << current_B.y << ")" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(2000));

    // 清零所有通道，停止 PWM 输出
    clear_all_channels(pca);

    return 0;
}
