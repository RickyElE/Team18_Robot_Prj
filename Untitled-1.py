import math

import math

def Angle_calculate(is_horizontal=True):
    # 初始位置
    current_A = [-29.64, 28.5]
    current_B = [-79, 0]
    current_C = [0, 0]

    # 变化量
    deltaX = 10
    deltaY = 10

    # 根据运动类型计算目标位置
    if is_horizontal:  # 平移运动
        target_A = [current_A[0] + deltaX, current_A[1]]
    else:  # 竖直运动
        target_A = [current_A[0], current_A[1] + deltaY]
    
    # 计算目标边的长度
    len_BC = 79.0  # 边 BC 的长度
    len_AB = 57.0  # 边 AB 的长度
    # 假设计算的是从点 C 到目标 A 的距离（可根据需求调整）
    len_AC = math.sqrt((target_A[0] - current_C[0])**2 + (target_A[1] - current_C[1])**2)
    
    # 按照余弦定理计算 theta2
    # 正确公式: theta2 = arccos((AB^2 + BC^2 - AC^2) / (2 * AB * BC))
    theta2 = math.acos((len_AB**2 + len_BC**2 - len_AC**2) / (2 * len_AB * len_BC))
    
    return theta2

# 示例调用
result = Angle_calculate(is_horizontal=True)
print("计算得到的theta2（弧度）：", result)





