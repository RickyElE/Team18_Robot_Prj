#pragma once

#include "SCSCL.h"
#include <utility>

/**
 * @brief 初始化机械臂到默认位置
 *
 * @param sc SCSCL 控制器实例的引用
 */
void initial_roboarm(SCSCL &sc);

/**
 * @brief 根据舵机1的当前位置计算 theta1 偏移量
 *
 * @param sc SCSCL 控制器实例的引用
 * @return 计算得到的 theta1 偏移量（单位与算法中一致）
 */
double update_math_equ(SCSCL &sc);

/**
 * @brief 根据舵机0的增量运动计算两个舵机的目标位置
 *
 * @param sc SCSCL 控制器实例的引用
 * @param delta_pos0 舵机0的位置变化量
 * @return 包含 {target_pos0, target_pos1} 的整数对
 */
std::pair<int, int> angleCalculate(SCSCL &sc, int delta_pos0);

/**
 * @brief 机械臂前进操作（在舵机0上执行固定增量运动）
 *
 * @param sc SCSCL 控制器实例的引用
 */
void MoveForward(SCSCL &sc);

/**
 * @brief 机械臂后退操作（在舵机0上执行固定增量运动）
 *
 * @param sc SCSCL 控制器实例的引用
 */
void MoveBackward(SCSCL &sc);

/**
 * @brief 以指定速度抬头（舵机1向上运动）
 *
 * @param sc SCSCL 控制器实例的引用
 * @param speed 运动速度
 */
void HeadUp(SCSCL &sc, int speed);

/**
 * @brief 以指定速度低头（舵机1向下运动）
 *
 * @param sc SCSCL 控制器实例的引用
 * @param speed 运动速度
 */
void HeadDown(SCSCL &sc, int speed);
