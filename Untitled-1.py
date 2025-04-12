import math


def Angle_calculate():
    #初始位置
    current_A=[-29.64,28.5]
    current_B=[-79,0]
    current_C=[0,0]
    #变化量
    deltaX = 10
    deltaY = 10
    
    #目标位置
    
    #如果是平移运动：
    target_A=[0,0]
    target_A[0] = current_A[0] + deltaX
    target_A[1] = current_A[1]   
    

    #如果是竖直运动：
    target_A[0] = current_A[0] 
    target_A[1] = current_A[1] + deltaY
    
    #计算达到目标点所需的坐标：
    len_BC = 79.0  #目标边BC的长度
    len_AC = math.sqrt((target_A[0]-current_C[0])**2 + (target_A[1]-current_C[1])**2) #目标边AC的长度
    len_AB = 57.0  #目标边AB的长度

    #余弦定理计算theta2:
    #cosB = a2 + c2 - b2 / 2ac
    theta2 = math.acos( (len_BC**2 + len_AB**2 - len_AC**2) / (2 * len_BC * len_AB) )
