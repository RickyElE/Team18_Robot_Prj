import time
from board import SCL, SDA
import busio
from adafruit_motor import servo
from adafruit_pca9685 import PCA9685
import sys
import tty
import termios
import threading

# 設置 I2C
i2c = busio.I2C(SCL, SDA)
# 創建 PCA9685 實例
pca = PCA9685(i2c, address=0x5f)  # 默認值為 0x40
pca.frequency = 50

# 當前角度
current_angle = 90  # 初始角度設為中間值
running = True

def set_angle(ID, angle):
    servo_angle = servo.Servo(pca.channels[ID], min_pulse=500, max_pulse=2400, actuation_range=180)
    servo_angle.angle = angle

def get_key():
    # 獲取單個按鍵輸入，無需按回車
    fd = sys.stdin.fileno()
    old_settings = termios.tcgetattr(fd)
    try:
        tty.setraw(sys.stdin.fileno())
        ch = sys.stdin.read(1)
    finally:
        termios.tcsetattr(fd, termios.TCSADRAIN, old_settings)
    return ch

def keyboard_thread():
    global current_angle, running
    
    print("控制馬達：")
    print("  q - 向上移動")
    print("  w - 向下移動")
    print("  x - 退出程式")
    
    while running:
        key = get_key()
        
        if key == 'q':
            # 向上移動 (減少角度)
            current_angle = max(0, current_angle - 10)
            print(f"向上移動，當前角度: {current_angle}")
        elif key == 'w':
            # 向下移動 (增加角度)
            current_angle = min(180, current_angle + 10)
            print(f"向下移動，當前角度: {current_angle}")
        elif key == 'x':
            print("退出程式")
            running = False
        
        # 小延遲以減少 CPU 使用率
        time.sleep(0.1)

if __name__ == "__main__":
    channel = 4  # 使用通道 4
    
    try:
        # 啟動鍵盤監聽線程
        kb_thread = threading.Thread(target=keyboard_thread)
        kb_thread.daemon = True
        kb_thread.start()
        
        # 主循環負責更新伺服位置
        while running:
            set_angle(channel, current_angle)
            time.sleep(0.1)
            
    except KeyboardInterrupt:
        print("程式被使用者中斷")
    finally:
        # 在退出前將伺服置于中間位置
        set_angle(channel, 90)
        print("程式結束")