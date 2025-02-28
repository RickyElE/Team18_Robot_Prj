// camera.h - Camera interface
#ifndef CAMERA_H
#define CAMERA_H

#include <vector>
#include <cstdint>
#include <cstddef>

class Camera {
public:
    Camera();
    ~Camera();
    
    // Get a frame from the camera as JPEG data
    std::vector<uint8_t> getFrame();
    
private:
    // Camera device handle
    int cameraFd;
    
    // Buffers for camera data
    void* buffer;
    
    // Initialize the camera
    bool initCamera();
    
    // Release camera resources
    void releaseCamera();
};

#endif // CAMERA_H