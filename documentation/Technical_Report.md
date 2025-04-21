# _**Team18_Robot_Prj**_ : Plant Protection Robot

## 1. Introduction
### 1.1 What is it?
The Plant Proetction Robot is a guarder which be used to check the status of the plants when people do not want to work outside. Then people can remotely control it to **monitor** and **inspect** the plant conditions. They can also use the **robotic arm** to **drive away pests**, **cut off yellow leaves** and **pick fruits**. This robot is based on **Adeept's ADR013 robot kit**: https://www.adeept.com/rasptank_p0121.html, and the **Raspberry Pi 5** with **RaspberryPi OS**. We will use the robot kit and pi to develop our coding and logic, therefore finishing our target of Plant Proetction.

## 2. Robot Structure

### 2.1 Chassis

#### 2.1.1 Moving
- The chassis of the robot is tracked. We use the **IIC protocol** to write **PWM** generation instructions to the **PCA9685**. By adjusting the duty cycle of the **PWM** wave, we can change the output of the **DRV8833**, thereby controlling the forward or reverse rotation of the **DC motor**. So we can generate a wheel speed difference based on this to achieve the purposes of moving **forward**, **backward** and turning **left** and **right**.

#### 2.1.2 Obstacle Detection
- We installed the ultrasonic sensor of model **HC-SR04** right in front of the robot. It calculates the distance between the obstacle and the front of the robot by the time difference between the emission of **ultrasonic waves** and their reception. Therefore, the correct waveform must first be sent to the **trig port** to request the sensor to emit ultrasonic waves. Then, the timing starts when the **rising edge of the echo port** is triggered and ends when the **falling edge of the echo port** is triggered. The time in between can be used to calculate the distance. **Reference Link:** https://akizukidenshi.com/goodsaffix/hc-sr04_v20.pdf

#### 2.1.3 System Info
- Monitor the temperature and usage rate of the CPU, as well as the usage rate of memory and memory swap. To avoid overheat.
- The **ADS7830** collects **battery voltage** information and **power status** in real time to prevent the robot from having too low a battery.


### 2.2 Robotic Arm

#### 2.2.1 Algorithm Refinement

- **Version 1**  

We simplified the robotic arm as shown in figure below, treating the gripper as the end of link AB (i.e., point A). The entire motion of point A is achieved by controlling motors B and C.

<img src="https://github.com/RickyElE/Team18_Robot_Prj/raw/main/images/FigureA.jpg" alt="FigureA" width="500" height="300"/>

Initially, we planned to establish a 2D coordinate system with point C as the origin, aiming to achieve horizontal and vertical movements of point A within this coordinate system. Therefore, we developed the following algorithm: assuming the coordinates of point A are (xA, yA), the target position for horizontal movement is A' (xA + δx, yA), and for vertical movement, it is A' (xA, yA + δy).

<img src="https://github.com/RickyElE/Team18_Robot_Prj/raw/main/images/FigureB.jpg" alt="FigureB" width="500" height="300"/>


Therefore, the motion of the robotic arm is essentially a mathematical problem-solving process. Taking vertical movement as an example, the algorithm is illustrated as follows: as shown in the figure above, given the coordinates of A', the lengths of links AB and BC, and the distance A'C, we can use the Law of Cosines to calculate the angle ∠A'BC (denoted as θ₂). Since the coordinates of A' are known, angle ∠A'CB can be calculated geometrically. Then, by applying the Law of Cosines again, we can compute angle ∠A'CB'. The difference between ∠A'CB and ∠A'CB' gives angle ∠B'CB (denoted as θ₁). By rotating motors B and C to angles θ₁ and θ₂ respectively, point A can move from (xA, yA) to A' (xA, yA + δy). The implementation code can be found in Appendix 1 (note: the original code is not uploaded to GitHub, as it was later replaced with an improved algorithm).

- **Version 2**  

After implementing the previous version of the algorithm, we found through continuous testing that, although we achieved horizontal and vertical movements of the robotic arm's end-effector from a mathematical perspective, such movements did not hold much practical value for the task of branch pruning. In essence, the algorithm merely enabled simple horizontal or vertical displacement of point A, the end-effector, without contributing meaningfully to the actual task.As shown in the figure below, due to the mechanical characteristics of the robotic arm, if we attempt to lift the end-effector from point A to point A′ for pruning, the previous algorithm may result in a configuration like A′B′C. Therefore, the algorithm is not suitable for solving our practical problem. Based on this observation, we developed a second version of the algorithm.

<img src="https://github.com/RickyElE/Team18_Robot_Prj/raw/main/images/FigureC.jpg" alt="FigureC" width="400" height="300"/>

As shown in the figure below, our goal is to ensure that the angle between link AB and the horizontal direction remains constant, regardless of the movement of link BC. In practical operation, we first use “Forward” and “Backward” commands to control link BC to approach the target point. During this process, the angle between link AB and the horizontal remains unchanged. Once the arm is near the target point, we can adjust the angle between AB and the horizontal using the “HeadUp” or “HeadDown” commands to find an appropriate angle for branch pruning.

To achieve this type of movement, our algorithm is as follows: as shown in the figure, since θ₁ is the angle between link BC and the horizontal, and due to the equality of alternate interior angles between parallel lines, the angle marked in red is also equal to θ₁. Therefore, we obtain: θ₂ = θ₁ + Δθ. From this, we find that θ₂ - θ₁ = Δθ, where Δθ represents the angle between link AB and the horizontal. By implementing this mathematical relationship, we successfully completed the motion algorithm for the robotic arm.

<img src="https://github.com/RickyElE/Team18_Robot_Prj/raw/main/images/FigureD.jpg" alt="FigureD" width="500" height="300"/>

#### 2.2.2 Hardware Update

In the beginning, we used motors that only supported open-loop control, meaning there was no real-time position feedback. Combined with insufficient output torque, this often caused the robotic arm to fall or move in ways that did not match expectations. To address this, we replaced them with improved motors that provide position feedback. However, during implementation, we discovered that the feedback from these motors was highly inaccurate, making full closed-loop control still unfeasible. As a result, in our final version of the code, we adopted a semi-closed-loop control strategy, in which the position is read only at fixed intervals. We believe that with better hardware in the future, full closed-loop control can be achieved, leading to higher accuracy and improved performance.

### 2.3 Video Streaming 
The video streaming system utilizes LibCamera integrated with OpenCV and a web-based interfaced to deliver real-time visual feedback from the robot.

The video streaming is built on **Professor Bernd Porr's libcamera2opencv library**(https://github.com/berndporr/libcamera2opencv.git), which we've extended to support web-based streaming.

We developed a LibCam2Web class that bridges the gap between the camera and web-based streaming. This implementation uses:

- **LibCamera API**  
  For direct hardware access to Raspberry Pi camera module, offering better perfromance.

- **OpenCV**  
  For frame processing, format conversion (BGR to RGB color space), and JPEG compression.

- **HTTP Streaming**  
  A lightweight HTTP server that delivers MJPEG streams viewable in any modern web browse.


The streaming process follows these steps:
- The Libcam2OpenCV class captures raw frames from the camera module.
- Each captured frame is processed through a callback system.
- Frames are converted to JPEG format with configurable quality.
- The SimpleHttpServer delivers these frames to connected clients.
- Thread locks (mutexes) make sure the video frames stay complete when multiple parts of the program try to use them at the same time.

### 2.4 Website Server
The UI remains responsive and continuously updates via **WebSocket**, allowing two-way data exchange between the browser and the robot. This ensures low-latency control and live system monitoring without the need to refresh the page.

**Remote Control System Integration :** 
The remote control system integrates frontend JavaScript with backend C++ code through WebSocket communication, creating a complete control loop for the robot's operation.Movement commands are sent instantly when buttons are pressed, and sensor data updates are received in real time.

1️⃣**WebSocket Connection Establishment**: 
- Browser connects to server using **WebSocket('ws://192.168.1.100:8084')**
- Bidirectional communication channel established for control and monitoring


2️⃣**Command Transmission**
- User interactions captured by JavaScript event listeners
- Commands formatted as for example **JSON: {type: "command", action: "move_forward"}** 
action: Defines the specific control action.

3️⃣**Feedback**
- Sensor data collected and sent back via same WebSocket connection
- UI updates in real-time to display current robot status and camera feed

**💬USER INTERFACE:**
Accessible via any web browser, the UI displays:

- **Login System Interface**  : Users must log in with a username and password:  
  - Username/Password: `robot`  

- **Movement Controls** : Directional buttons are used to control the robot’s 

- **Mechanical Arm Operations** : 
    - Movement Tab:Controls vertical and forward/backward arm motion  
    - Clamp Tab: Controls the clamp for cutting, releasing, and rotation

- **Live Camera and Screenshots** : 
    
  - Real-time camera video stream  
  - Camera control buttons for up/down movement  
  - Screenshot capture with timestamps and a scrolling gallery

- **System Status**  :
  - Distance readings via ultrasonic sensor (gauge display)  
  - System resource stats: 
Battery information,CPU temperature/usage, RAM info/ Swap info

- **Live Camera and Screenshots**  
    
  - Real-time camera video stream  
  - Camera control buttons for up/down movement  
  - Screenshot capture with timestamps and a scrolling gallery

- **System Status**  
  - Distance readings via ultrasonic sensor (gauge display)  
  - System resource stats: 
Battery information,CPU temperature/usage, RAM info/ Swap info
## 3 Software Structure

### 3.1 Callback and Events
- We use **timerfd** to build a timer to enable the sensor to read information at regular intervals. Then, we register it through a **callback**. When the **timer event** is triggered, the callback function will be called to read the sensor information and save it for use by the get function to obtain the information.

### 3.2 Delay
- We use **timerfd** to implement the **delay** function. Compared with **usleep** and **std::this_thread::sleep_for**, this way, waiting can be achieved without blocking the thread, and it is **highly accurate**, **stable** and **anti-jitter**.

### 3.3 Avoiding Memory leaking
- Every time a thread, function or variable is created, we will release it, like:
``` bash
temp_thread = thread(function, this);

if (temp_thread.joinable()){
    temp_thread.join();
}
```
## 4 Current Limitations & Areas for Improvement
### 4.1 Software:
- **Limited autonomy**: The robot currently lacks autonomous navigation or mapping capabilities (e.g., SLAM), and relies on manual control via the web interface.
- **High CPU usage**: The current implementation of the main control program results in high CPU usage on the Raspberry Pi 5, which can affect overall system responsiveness.
- **MJPEG streaming performance**: The HTTP server responsible for video streaming occasionally experiences stuttering or latency during transmission, especially under load.
- **HTTP server complexity**: Starting the HTTP server and integrating it with the rest of the system currently requires multiple steps and lacks automation.
- **Lack of visual intelligence**: Currently, pest deterrence relies on manual control. Future versions should include image recognition to detect insects and respond autonomously.
- **Arm movement precision**: The robotic arm’s current control algorithm is basic. Improvements are needed for accurate positioning and targeted pest removal.
- **Unresolved bugs**: Several known issues remain in the codebase that may affect stability or edge-case behavior and need to be addressed in future iterations.
### 4.2 Hardware:
- **Power system upgrade planned**: It is necessary to replace the current HAT with a more advanced board that:
  - Supports USB Power Delivery (PD) input
  - Integrates voltage regulation circuitry
  - Charges rechargeable lithium batteries
  - Drives multiple servo motors without voltage drops

## 5 Team 
### 5.1 Collaborator
Leader - Zhihong Xu

Collaborator - YUWEN WANG, Guining Zhang, Guankai Wang, JUIHSIN CHANG

### 5.2 The Arrangement
Chassis & System Structure - Zhihong Xu

Robotic Arm - Guankai Wang, Guining Zhang

Vision & Communication - YUWEN WANG, JUIHSIN CHANG

## 6 Appendix
### Code Implementation of the Initial Version of the Robotic Arm Algorithm：
```cpp
#include <iostream>
#include <chrono>
#include <thread>
#include <cmath>
#include <iomanip>
#include <cstdint>
#include "PCA9685.h"
#include "delay.h"

// class roboarm encapsulates PCA9685 control and movement logic
class roboarm {
public:

    Delay delay;

    // Internal 2D point struct for storing key joint positions
    struct Point2D {
        double x;
        double y;
    };

    // Constructor with optional I2C address of PCA9685 (default: 0x5f)
    roboarm(uint8_t addr = 0x5f) : pca(addr) {
        // Set initial position (corresponding to init_position())
        initPosition();
        // current_C is fixed at the origin
        current_C = {0, 0};
    }

    ~roboarm() {
        // Destructor clears all channel signals to stop PWM output on exit
        clearAllChannels();
    }

    // Initialize PCA9685 and configure PWM frequency
    bool init() {
        if (!pca.init()) {
            std::cerr << "Failed to initialize PCA9685!" << std::endl;
            return false;
        }
        // Set PWM frequency to 50Hz, suitable for servos
        pca.setPWMFreq(50);
        // Clear all channels
        clearAllChannels();
        pca.wakeup();
        return true;
    }

    // Set servo angle on the specified channel (range: 0~180 degrees)
    void setAngle(int channel, double angle) {
        // Servo parameters (unit: microseconds)
        const int min_pulse = 500;
        const int max_pulse = 2400;
        const int actuation_range = 180; // Servo motion range

        // Calculate pulse width based on angle
        int pulse = min_pulse + (max_pulse - min_pulse) * angle / actuation_range;
        // PCA9685 resolution is 4096, at 50Hz the period is 20000 microseconds
        int ticks = static_cast<int>((pulse / 20000.0) * 4096);
        // Set PWM output for the channel (on=0, off=ticks)
        pca.setPWM(channel, 0, ticks);
    }

    // Clear all PWM signals on all channels (stop output)
    void clearAllChannels() {
        const int NUM_CHANNELS = 16;
        for (int channel = 0; channel < NUM_CHANNELS; channel++) {
            pca.setPWM(channel, 0, 0);
        }
    }

    // Initialize robotic arm to default position (startup pose)
    void initialRobarm() {
        setAngle(0, 180);
        delay.delay_ms(2000);
        setAngle(1, 180);
        delay.delay_ms(2000);
        setAngle(2, 90);
        delay.delay_ms(2000);
        setAngle(3, 100);
        delay.delay_ms(2000);
    }

    // Initialize joint positions of the robotic arm (corresponds to init_position() in Python)
    void initPosition() {
        current_A = { -29.64, 28.5 };
        current_B = { -79, 0 };
        current_C = { 0, 0 };
    }

    // Calculate target angles based on displacement and update internal state
    // Parameters deltaX and deltaY represent horizontal and vertical displacement
    // len_AB and len_BC are the link lengths of the robotic arm
    std::pair<double, double> angleCalculate(double deltaX, double deltaY, double len_AB = 57.0, double len_BC = 79.0) {
        // Calculate target position target_A
        Point2D target_A;
        target_A.x = current_A.x + deltaX;
        target_A.y = current_A.y + deltaY;
        // Calculate distance from current_C to target_A (length of AC)
        double len_AC = std::sqrt((target_A.x - current_C.x) * (target_A.x - current_C.x) +
                                  (target_A.y - current_C.y) * (target_A.y - current_C.y));
        // Calculate theta2 using the Law of Cosines (angle between AB and BC)
        double theta2 = safeAcos((len_AB * len_AB + len_BC * len_BC - len_AC * len_AC) / (2 * len_AB * len_BC));
        // Calculate theta1: use atan2 to get direction of target_A, then subtract Law of Cosines result
        double theta1 = std::atan2(target_A.y, target_A.x) - 
                        safeAcos((len_BC * len_BC + len_AC * len_AC - len_AB * len_AB) / (2 * len_BC * len_AC));
        // Adjust angles and convert to degrees
        double f_angle1 = 180.0 - (theta1 * 180.0 / M_PI);
        double f_angle2 = 210.0 - (theta2 * 180.0 / M_PI);
        
        // Calculate target_B based on theta1 (assume distance to current_C is len_BC)
        Point2D target_B;
        target_B.x = len_BC * std::cos(theta1);
        target_B.y = len_BC * std::sin(theta1);
        // Update internal state
        current_A = target_A;
        current_B = target_B;
        return std::make_pair(f_angle1, f_angle2);
    }

    // Move forward: increase x coordinate (deltaY = 0)
    void moveForward() {
        auto angles = angleCalculate(3, 0, 57.0, 79.0);
        std::cout << "[MoveForward] f_angle1: " << angles.first 
                  << "  f_angle2: " << angles.second << std::endl;
        setAngle(0, angles.first);
        delay.delay_ms(500);
        setAngle(1, angles.second);
    }

    // Move backward: decrease x coordinate (deltaY = 0)
    void moveBackward() {
        auto angles = angleCalculate(-3, 0, 57.0, 79.0);
        std::cout << "[MoveBackward] f_angle1: " << angles.first 
                  << "  f_angle2: " << angles.second << std::endl;
        setAngle(0, angles.first);
        delay.delay_ms(500);
        setAngle(1, angles.second);
    }

    // Move upward: increase y coordinate (deltaX = 0)
    void moveUp() {
        auto angles = angleCalculate(0, 30, 57.0, 79.0);
        std::cout << "[MoveUp] f_angle1: " << angles.first 
                  << "  f_angle2: " << angles.second << std::endl;
        // Adjust channel 1 first, then channel 0
        setAngle(1, angles.second);
        delay.delay_ms(500);
        setAngle(0, angles.first);
        delay.delay_ms(500);
    }

    // Move downward: decrease y coordinate (deltaX = 0)
    void moveDown() {
        auto angles = angleCalculate(0, -3, 57.0, 79.0);
        std::cout << "[MoveDown] f_angle1: " << angles.first 
                  << "  f_angle2: " << angles.second << std::endl;
        setAngle(0, angles.first);
        delay.delay_ms(500);
        setAngle(1, angles.second);
    }

    // Debug method: print current positions of key points
    void printStatus() {
        std::cout << "Current_A: (" << current_A.x << ", " << current_A.y << ")" << std::endl;
        std::cout << "Current_B: (" << current_B.x << ", " << current_B.y << ")" << std::endl;
        std::cout << "Current_C: (" << current_C.x << ", " << current_C.y << ")" << std::endl;
    }

private:
    // safeAcos ensures input stays within [-1, 1] to avoid floating-point errors
    double safeAcos(double x) {
        if (x > 1.0) x = 1.0;
        if (x < -1.0) x = -1.0;
        return std::acos(x);
    }

    // Internal variables to store joint positions
    Point2D current_A;
    Point2D current_B;
    Point2D current_C;

    // Internal PCA9685 and delay objects
    PCA9685 pca;
    
};