import math

def Angle_calculate(is_horizontal=True):
    # 初始位置（每次函数调用都会重置成这几个初始值）
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
    # 计算从点 C 到目标 A 的距离
    len_AC = math.sqrt((target_A[0] - current_C[0])**2 + (target_A[1] - current_C[1])**2)
    
    # 按照余弦定理计算 theta2
    # 正确公式: theta2 = arccos((AB^2 + BC^2 - AC^2) / (2 * AB * BC))
    theta2 = math.acos((len_AB**2 + len_BC**2 - len_AC**2) / (2 * len_AB * len_BC))
    
    # theta1（BCB'）计算示例：
    # 利用 atan2 得到 target_A 向量的角度，再减去角 C（ACB）的角度
    theta1 = math.atan2(target_A[1], target_A[0]) - math.acos((len_BC**2 + len_AC**2 - len_AB**2) / (2 * len_BC * len_AC))
    
    # 将计算结果转换为角度并进行修正
    f_angle1 = 180 - math.degrees(theta1) 
    f_angle2 = 210 - math.degrees(theta2)

    return f_angle1, f_angle2

# 调用函数，并将返回值赋给变量
f_angle1, f_angle2 = Angle_calculate(is_horizontal=True)
print("计算得到的 angle2（角度）：", f_angle2)
print("计算得到的 angle1（角度）：", f_angle1)
