import math

def init_position():
    """
    初始化初始位置，返回三个二维坐标点：
      - current_A: 如 [-29.64, 28.5]
      - current_B: 如 [-79, 0]
      - current_C: 如 [0, 0]
    """
    current_A = [-29.64, 28.5]
    current_B = [-79, 0]
    current_C = [0, 0]
    return current_A, current_B, current_C

# 初始化全局位置状态（只执行一次）
current_A, current_B, current_C = init_position()

def Angle_calculate(is_horizontal=True, deltaX=10, deltaY=10, len_AB=57.0, len_BC=79.0):
    """
    根据运动类型和参数计算机械臂的角度，
    同时更新全局状态：每次调用后，将 target_A 和 target_B 分别赋值给 current_A 和 current_B。
    
    参数：
      is_horizontal (bool): 如果为 True，则表示水平移动；否则表示竖直移动。
      deltaX, deltaY (float): 水平或竖直方向的位移量。
      len_AB, len_BC (float): 连杆 AB 和 BC 的固定长度。

    返回：
      f_angle1, f_angle2：修正后的两个角度（单位：度），按照各自几何关系进行偏移调整。
    """
    global current_A, current_B, current_C

    # 根据运动类型计算目标位置 target_A
    target_A = [current_A[0] + deltaX, current_A[1] + deltaY]


    # 计算从 current_C 到 target_A 的距离（边 AC 的长度）
    len_AC = math.sqrt((target_A[0] - current_C[0])**2 + (target_A[1] - current_C[1])**2)

    # 计算 theta2：利用余弦定理计算 AB 与 BC 之间的角度
    theta2 = math.acos((len_AB**2 + len_BC**2 - len_AC**2) / (2 * len_AB * len_BC))
    
    # 计算 theta1：先用 atan2 得到 target_A 的方向，再减去由余弦定理计算得到的角（即点 C 内角）
    theta1 = math.atan2(target_A[1], target_A[0]) - math.acos((len_BC**2 + len_AC**2 - len_AB**2) / (2 * len_BC * len_AC))
    
    # 根据几何关系对角度进行修正，并转换为度
    f_angle1 = 180 - math.degrees(theta1)
    f_angle2 = 210 - math.degrees(theta2)
    
    # 根据 theta1 计算 target_B，假设 target_B 距离 current_C 为 len_BC
    target_B = [len_BC * math.cos(theta1), len_BC * math.sin(theta1)]
    
    # 更新全局状态：将 target_A 和 target_B 分别赋值给 current_A 和 current_B
    current_A = target_A
    current_B = target_B

    return f_angle1, f_angle2

def MoveForward():
    """
    向前运动：水平运动中 x 坐标增加
    """
    f_angle1, f_angle2 = Angle_calculate(is_horizontal=True, deltaX=10, deltaY=0)
    print(f"[MoveForward] f_angle1: {f_angle1:.2f}  f_angle2: {f_angle2:.2f}")
    #set_angle(0, f_angle1)
    #set_angle(1, f_angle2)

def MoveBackward():
    """
    向后运动：水平运动中 x 坐标减少
    """
    f_angle1, f_angle2 = Angle_calculate(is_horizontal=True, deltaX=-10, deltaY=0)
    print(f"[MoveBackward] f_angle1: {f_angle1:.2f}  f_angle2: {f_angle2:.2f}")
    #set_angle(0, f_angle1)
    #set_angle(1, f_angle2)

def MoveUp():
    """
    向上运动：竖直运动中 y 坐标增加
    """
    f_angle1, f_angle2 = Angle_calculate(is_horizontal=False, deltaX=0, deltaY=10)
    print(f"[MoveUp] f_angle1: {f_angle1:.2f}  f_angle2: {f_angle2:.2f}")
    #set_angle(0, f_angle1)
    #set_angle(1, f_angle2)

def MoveDown():
    """
    向下运动：竖直运动中 y 坐标减少
    """
    f_angle1, f_angle2 = Angle_calculate(is_horizontal=False, deltaX=0, deltaY=-10)
    print(f"[MoveDown] f_angle1: {f_angle1:.2f}  f_angle2: {f_angle2:.2f}")
    #set_angle(0, f_angle1)
    #set_angle(1, f_angle2)


print("初始状态:")
print("current_A:", current_A)
print("current_B:", current_B)


print("\n向前移动:")
MoveForward()
print("更新后的 current_A:", current_A)
print("更新后的 current_B:", current_B)

print("\n向后移动:")
MoveBackward()
print("更新后的 current_A:", current_A)
print("更新后的 current_B:", current_B)

print("\n向上移动:")
MoveUp()
print("更新后的 current_A:", current_A)
print("更新后的 current_B:", current_B)

print("\n向下移动:")
MoveDown()
print("更新后的 current_A:", current_A)
print("更新后的 current_B:", current_B)
