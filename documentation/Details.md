# _**Team18_Robot_Prj**_ : Plant Protection Robot

## 1. Introduction
### 1.1 What is it?
The Plant Proetction Robot is a guarder which be used to check the status of the plants when people do not want to work outside. Then people can remote control it to check the plant and do something else, such as cutting branches and leaves. This robot is based on **Adeept's ADR013 robot kit**: https://www.adeept.com/rasptank_p0121.html, and the **Raspberry Pi 5** with **RaspberryPi OS**. We will use the robot kit and pi to develop our coding and logic, therefore finishing our target of Plant Proetction.

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
- Main.cpp launches WebSocket server on port 8084
- Browser connects to server using **WebSocket('ws://192.168.1.100:8084')**
- Bidirectional communication channel established for control and monitoring


2️⃣**Command Transmission**:
- User interactions captured by JavaScript event listeners
- Commands formatted as for example **JSON: {type: "command", action: "move_forward"}** 
action: Defines the specific control action.
- WebSocket transmits commands to robot controller

3️⃣**Feedback Loop**
- Robot hardware executes commands through control functions
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
In sensor reading and system info reading

## 4 Team 
### 4.1 Collaborator
Leader - Zhihong Xu

Collaborator - YUWEN WANG, Guining Zhang, Guankai Wang, JUIHSIN CHANG

### 4.2 The Arrangement
Chassis & System Structure - Zhihong Xu

Robotic Arm - Guankai Wang, Guining Zhang

Vision & Communication - YUWEN WANG, JUIHSIN CHANG

## 


                       
