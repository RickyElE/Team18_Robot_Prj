# How to Cross Compile the applications of Raspberry Pi in X86 or Apple Silicon

Due to the class project, I want to use my Macbook pro M1 and my Windows laptop to do the cross compile. Because it is unconvenient to do programming in Raspberry Pi with teammate. 

## Prerequisites

You need to make sure you can connect to raspberry pi by SSH. 
Then you need to install `rsync` and crosscompiler in raspberry pi.
```
sudo apt-get install rsync
sudo apt-get gcc-aarch64-linux-gnu
sudo apt-get g++-aarch64-linux-gnu
```
Then, you need to have an operation system of debian 12 in your computer, which is the basic of the raspberry pi os. I used the Vmware 16 (Win) or Vmware Fusion (Mac) to install the debian 12 in my laptop. 



## Usage examples

After installing the `rsync`, you can use it to transmit the sysrroot of raspberry pi os to the debian 12: 
```
rsync -avz Hostname@xxx.xxx.xxx.xxx:/lib /the/destination/folder/path
rsync -avz Hostname@xxx.xxx.xxx.xxx:/usr /the/destination/folder/path
```
The Hostname is your hostname of Raspberry Pi, and you need to set the path of file storage.

After transmission, you will see the folder of lib and usr. For example: 
```
-/home/user/raspi-sysroot
--/usr
--/lib
```

Then you can use these file to cross complie the application. 

### Building

To compile application of Raspberry Pi, you need to let compiler know where the file of header and library in. Here I would use CMakeLists.txt to tell it. For example:
```
cmake_minimum_required(VERSION 3.16)
project(GPIO_CXX_Demo CXX)

set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

# set crosscompiler
set(CMAKE_C_COMPILER /usr/bin/aarch64-linux-gnu-gcc)
set(CMAKE_CXX_COMPILER /usr/bin/aarch64-linux-gnu-g++)

# set sysroot of raspberry pi
set(CMAKE_SYSROOT /home/xzh/raspi-sysroot)

# set the path of header and library
include_directories(${CMAKE_SYSROOT}/usr/include)
link_directories(${CMAKE_SYSROOT}/usr/lib/aarch64-linux-gnu)

add_executable(gpio_test gpio_test.cpp)

target_link_libraries(gpio_test gpiod)
```

Here is the gpio test to make led work. 

After you make your own CMakeLists.txt, create a folder called `build`, and then 
```
cd build
cmake ..
make
```
Then you will get the executable file in the folder `build`. 
And then you use the command `scp` to transmit the executable file to Raspberry Pi: 
```
scp YOUR_EXECUTABLE_FILE Hostname@xxx.xxx.xxx.xxx:/home/xxx/
```
Then you can execute the file in your Raspberry Pi by `./YOUR_EXECUTABLE_FILE`


## Shortcoming
This method cannot have 100% to do compiling in your computer because of the technical reasons. For example, I am only able to compile the application with gpiod but cannot compile the applicatin with FastDDS. And I also find another article which teaches you to configure cross compile environment in Ubuntu: https://tttapa.github.io/Pages/Raspberry-Pi/C++-Development-RPiOS/Development-setup.html .

If you follow this article to configure the environment successfully, please tell me. Thanks.
