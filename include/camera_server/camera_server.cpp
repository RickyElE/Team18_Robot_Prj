#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <linux/i2c.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include "PCA9685.h"


// PCA9685 registers/addresses
#define PCA9685_MODE1 0x00
#define PCA9685_PRESCALE 0xFE
#define LED0_ON_L 0x06
#define LED0_ON_H 0x07
#define LED0_OFF_L 0x08
#define LED0_OFF_H 0x09

// 當前角度和運行狀態
std::atomic<int> current_angle(90); // 初始角度設為中間值
std::atomic<bool> running(true);

// PCA9685地址
const int PCA9685_ADDR = 0x5f; // 默認為0x40
int i2c_fd; // I2C文件描述符

// I2C寫入字節
void i2c_write_byte(int file, uint8_t reg, uint8_t value) {
    uint8_t buf[2];
    buf[0] = reg;
    buf[1] = value;
    if (write(file, buf, 2) != 2) {
        std::cerr << "無法寫入I2C設備" << std::endl;
        exit(1);
    }
}

// I2C讀取字節
uint8_t i2c_read_byte(int file, uint8_t reg) {
    uint8_t buf[1] = {reg};
    if (write(file, buf, 1) != 1) {
        std::cerr << "無法設置要讀取的寄存器" << std::endl;
        exit(1);
    }
    
    if (read(file, buf, 1) != 1) {
        std::cerr << "無法讀取I2C設備" << std::endl;
        exit(1);
    }
    
    return buf[0];
}

// 設置PCA9685
void setup_pca9685() {
    // 打開I2C設備
    const char* i2c_device = "/dev/i2c-1"; // 在大多數樹莓派上，I2C總線1
    i2c_fd = open(i2c_device, O_RDWR);
    if (i2c_fd < 0) {
        std::cerr << "無法開啟I2C設備: " << i2c_device << std::endl;
        exit(1);
    }
    
    // 設置I2C設備地址
    if (ioctl(i2c_fd, I2C_SLAVE, PCA9685_ADDR) < 0) {
        std::cerr << "無法設置I2C地址" << std::endl;
        close(i2c_fd);
        exit(1);
    }
    
    // 重置PCA9685
    i2c_write_byte(i2c_fd, PCA9685_MODE1, 0x00);
    
    // 設置PWM頻率為50Hz
    int prescale = 25000000.0 / (4096 * 50.0) - 1; // 計算預分頻器值
    int mode1 = i2c_read_byte(i2c_fd, PCA9685_MODE1);
    i2c_write_byte(i2c_fd, PCA9685_MODE1, (mode1 & 0x7F) | 0x10); // 睡眠模式
    i2c_write_byte(i2c_fd, PCA9685_PRESCALE, prescale); // 設置預分頻器
    i2c_write_byte(i2c_fd, PCA9685_MODE1, mode1); // 喚醒
    usleep(5000);
    i2c_write_byte(i2c_fd, PCA9685_MODE1, mode1 | 0xa1); // 自動增量開啟
}

// 設置伺服角度
void set_angle(int channel, int angle) {
    // 將角度轉換為脈衝長度
    // 針對標準伺服：0度 = 500us, 180度 = 2400us
    int pulse = 500 + (angle * (2400 - 500) / 180);
    
    // 將脈衝時間轉換為PWM值 (0-4095)
    int value = pulse * 4096 / 20000; // 20000us為週期(50Hz)
    
    // 通道offset
    int channel_offset = 4 * channel;
    
    // 設置PWM控制寄存器
    i2c_write_byte(i2c_fd, LED0_ON_L + channel_offset, 0);
    i2c_write_byte(i2c_fd, LED0_ON_H + channel_offset, 0);
    i2c_write_byte(i2c_fd, LED0_OFF_L + channel_offset, value & 0xFF);
    i2c_write_byte(i2c_fd, LED0_OFF_H + channel_offset, value >> 8);
}

// 獲取按鍵輸入
char get_key() {
    struct termios oldt, newt;
    char ch;
    
    // 獲取當前終端屬性
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    
    // 設置無緩衝、無回顯模式
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    
    // 讀取一個字符
    ch = getchar();
    
    // 恢復終端屬性
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    
    return ch;
}

// 鍵盤監聽線程
void keyboard_thread_func() {
    std::cout << "控制馬達：" << std::endl;
    std::cout << "  q - 向上移動" << std::endl;
    std::cout << "  w - 向下移動" << std::endl;
    std::cout << "  x - 退出程式" << std::endl;
    
    while (running) {
        char key = get_key();
        
        if (key == 'q') {
            // 向上移動 (減少角度)
            int new_angle = std::max(0, current_angle.load() - 10);
            current_angle.store(new_angle);
            std::cout << "向上移動，當前角度: " << new_angle << std::endl;
        } else if (key == 'w') {
            // 向下移動 (增加角度)
            int new_angle = std::min(180, current_angle.load() + 10);
            current_angle.store(new_angle);
            std::cout << "向下移動，當前角度: " << new_angle << std::endl;
        } else if (key == 'x') {
            std::cout << "退出程式" << std::endl;
            running.store(false);
        }
        
        // 小延遲以減少 CPU 使用率
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

// 網絡監聽線程 - 新增
void network_thread_func() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
    
    // 創建套接字
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        std::cerr << "套接字創建失敗" << std::endl;
        return;
    }
    
    // 設置套接字選項，允許重用地址
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        std::cerr << "setsockopt 失敗" << std::endl;
        return;
    }
    
    // 設置地址
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8888);  // 使用8888端口
    
    // 綁定套接字
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        std::cerr << "綁定失敗" << std::endl;
        return;
    }
    
    // 監聽
    if (listen(server_fd, 3) < 0) {
        std::cerr << "監聽失敗" << std::endl;
        return;
    }
    
    std::cout << "網絡伺服器啟動，監聽端口 8888" << std::endl;
    
    // 設置非阻塞模式
    int flags = fcntl(server_fd, F_GETFL, 0);
    fcntl(server_fd, F_SETFL, flags | O_NONBLOCK);
    
    while (running) {
        // 接受連接
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
        
        if (new_socket >= 0) {
            // 讀取數據
            int valread = read(new_socket, buffer, 1024);
            if (valread > 0) {
                // 處理命令
                if (buffer[0] == 'q') {
                    // 向上移動 (減少角度)
                    int new_angle = std::max(0, current_angle.load() - 10);
                    current_angle.store(new_angle);
                    std::cout << "網絡命令：向上移動，當前角度: " << new_angle << std::endl;
                } else if (buffer[0] == 'w') {
                    // 向下移動 (增加角度)
                    int new_angle = std::min(180, current_angle.load() + 10);
                    current_angle.store(new_angle);
                    std::cout << "網絡命令：向下移動，當前角度: " << new_angle << std::endl;
                }
            }
            
            // 關閉連接
            close(new_socket);
        }
        
        // 小延遲以減少 CPU 使用率
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    close(server_fd);
}

// 信號處理
void signal_handler(int signum) {
    running.store(false);
}


void sleep(){
    uint8_t awake = i2c_read_byte(i2c_fd,MODE_1_ADDR);
    uint8_t sleep = awake | MODE_1_SLEEP;
    i2c_write_byte(i2c_fd,MODE_1_ADDR, sleep);
    // delay.delay_ms(5);
}

void wakeup(){
    uint8_t sleep = i2c_read_byte(i2c_fd,MODE_1_ADDR);
    uint8_t wakeup = sleep & ~MODE_1_SLEEP;
    i2c_write_byte(i2c_fd,MODE_1_ADDR, wakeup);
}

int main() {
    // 設置信號處理
    signal(SIGINT, signal_handler);
    
    try {
        // 初始化PCA9685
        setup_pca9685();
        
        // 伺服通道
        int channel = 4;
        wakeup();
        // 啟動鍵盤監聽線程
        std::thread kb_thread(keyboard_thread_func);
        
        // 啟動網絡監聽線程 - 新增
        std::thread net_thread(network_thread_func);
        
        // 主循環負責更新伺服位置
        while (running) {
            set_angle(channel, current_angle);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // 等待線程結束
        if (kb_thread.joinable()) {
            kb_thread.join();
        }
        
        // 等待網絡線程結束 - 新增
        if (net_thread.joinable()) {
            net_thread.join();
        }
    } catch (const std::exception& e) {
        std::cerr << "發生錯誤: " << e.what() << std::endl;
    }
    
    // 在退出前將伺服置于中間位置
    set_angle(4, 90);
    std::cout << "程式結束" << std::endl;
    
    sleep();
    // 關閉I2C
    close(i2c_fd);
    return 0;
}