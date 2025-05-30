# Team18 Real-Time Embedded Project
_**Below is a concise overview of the project. For more information, please refer to the detailed technical documentation here： [Technical Report](documentation/Technical_Report.md)**_

_**The related demo video can be found here:  [Video](video_demo.mp4)**_
## **Plant Protection Robot**

A smart agricultural robot based on **Raspberry Pi 5** and the **Adeept robot kit**, designed for **remote plant monitoring**, **picking the ripe fruits**, **pest deterrence**, **obstacle detection**, and **web-based control**. The system leverages **event-driven architecture**, efficient I/O scheduling, and multi-protocol hardware interfaces to achieve reliable and responsive operation.

![Robot Image](images/WechatIMG803.jpg)

---

### Key Features

- **Real-time Video Monitoring**  
  Uses the Raspberry Pi Camera v2 with the open-source library [libcamera2opencv](https://github.com/berndporr/libcamera2opencv) to capture frames and convert them directly into OpenCV cv::Mat format. These frames are then streamed as MJPEG via an embedded HTTP server.

- **Pest Deterrence and Picking fruits with Smart Arm**  
  Controls a 4-DOF robotic arm powered by **Waveshare serial bus servos**, communicating via **USB serial** for improved power stability and precise movements.

- **Sensor Data via Event-Driven Timers**  
  Periodic sensor updates driven by `TimerFD`, including:
  - Battery voltage monitoring via **ADS7830** ADC
  - Obstacle distance via **HC-SR04** ultrasonic sensor
  - System stats: CPU load, memory usage, and temperature

- **Tracked Chassis Drive System**  
  Uses **PCA9685 PWM controller** to generate signals for **DRV8833 motor drivers**, enabling forward, backward, and turning motions via DC motors.

- **Web-based Control Interface**  
A browser interface for real-time robot control and monitoring using **WebSocket**
---

### Hardware Overview

| Component | Description |
|----------|-------------|
| **Raspberry Pi 5** | Main control board, running logic and servers |
| **Pi Camera v2** | Captures live video stream |
| **HC-SR04** | Ultrasonic sensor for obstacle detection |
| **ADS7830** | ADC module for reading battery voltage |
| **PCA9685** | Multi-channel PWM signal generator |
| **DRV8833** | Dual H-bridge driver for DC motors |
| **Waveshare Serial Bus Servos** | Controlling the 4DOF robotic arm via USB |
| **Tracked Chassis** | Provides terrain adaptability and mobility |

> *Note: The Adeept HAT is not suitable for Raspberry Pi 5 due to insufficient power delivery when driving multiple servos. This project uses a separate power system and serial servos to ensure stability.*


---

### Software Architecture
```
 ┌──────────────────────────────────────────────────┐
 │              WebsiteServer (main)                │
 │  ┌──────────────────────────────────────┐        │
 │  │ Image Capture via libcamera2opencv   │        │
 │  │ MJPEG HTTP Streaming Server          │        │
 │  │ WebSocket Control & Status Push      │        │
 │  └──────────────────────────────────────┘        │
 │                                                  │
 │  Event-Driven Sensors (TimerFD based)            │
 │  ┌────────────────────────────────────────────┐  │
 │  │ Battery ADC │ Obstacle Distance │ Sys Info │  │
 │  └────────────────────────────────────────────┘  │
 │                                                  │
 │  GPIO: DC motors, PCA9685 PWM, DRV8833           │
 │  USB Serial: Waveshare Bus Servo Arm             │
 └──────────────────────────────────────────────────┘
```
---

### Project Structure

```bash
PlantProtectionRobot/
├── documentation/       # Technical Documents
├── images/              # Project images
├── include/             # The Driver Header and Declaration
├──── camera_server/
├──── driver/
├────── Camera/
├────── DCMotor/
├────── PCA9685/
├────── SerialMotor/
├────── SysInfo/
├────── Ultrasonic/
├────── WebArm/
├──── bms.h
├──── delay.h
├──── libcam2_node/
├──── libcamera2opencv-master/
├── main/                # Main Files
├── src/                             
├──── test/              # Test Files
├──── website_test/              # Test Files
├── website/             # Frontend HTML/CSS/JS
├── .clangd
├── .gitignore
├── CMakeLists.txt
├── LICENSE
└── README.md
```
---
### Setup & Usage

#### Requirements
- Raspberry Pi 5 (Raspberry Pi OS 64-bit recommended)
- libcamera enabled and configured
- Network access on the same LAN

#### Dependencies
``` bash
sudo apt update
sudo apt install libopencv-dev libgpiod-dev libwebsocketpp-dev libboost-all-dev libcamera-dev
```

``` bash
# Clone and build libcamera2opencv (required for video streaming)

git clone https://github.com/berndporr/libcamera2opencv.git
cd libcamera2opencv
mkdir build && cd build
cmake ..
make
sudo make install
```

#### Build & Run
``` bash
git clone https://github.com/RickyElE/Team18_Robot_Prj.git
cd Team18_robot_prj
mkdir build && cd build
cmake ../
make
./main
```
Then you need to open the website folder and execute the command:
``` bash
cd ../website
python3 -m http.server 8085
```

#### Access the Interface
Open your browser and go to:
- http://**your-raspberry-pi-ip**:8085

---

### License

This project is licensed under the **MIT License**.

---

### Group Member
- Leader: Zhihong Xu
- DCMotor, Sensors, System: Zhihong Xu
- Robot Arm, Servo Motor: Guankai Wang & Guining Zhang
- Website, Streaming: YUWEN WANG, JUIHSIN CHANG

Contributions welcome! Feel free to open issues or PRs.

---

### Social Media

See our robot in action on Instagram:
@[plantprotectionbot](https://www.instagram.com/plant_protection_robot?igsh=bW1sYWptamFkeGVs)

---

### Link
**Adeept Robot Kit**: https://www.adeept.com/rasptank_p0121.html

### License

This project is licensed under the GNU General Public License v2.0 (GPL-2.0).  
See the [LICENSE](LICENSE) file for full details.

The including components from projects licensed under GPL-2.0:

- [libcamera2opencv] - GPL-2.0 - https://github.com/berndporr/libcamera2opencv/tree/master

As a result, the combined work is subject to the terms of the GPL-2.0 license.  

# See You Next Time!
                       

