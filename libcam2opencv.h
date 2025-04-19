#ifndef __LIBCAM2OPENCV
#define __LIBCAM2OPENCV

#include <iomanip>
#include <iostream>
#include <memory>
#include <chrono>
#include <thread>
#include <map>
#include <vector>
#include <sys/mman.h>
#include <opencv4/opencv2/opencv.hpp>
#include <opencv4/opencv2/core.hpp>
#include <opencv4/opencv2/imgproc.hpp>
#include <opencv4/opencv2/highgui.hpp>

using cv::Mat;

// need to undefine QT defines here as libcamera uses the same expressions (!).
#undef signals
#undef slots
#undef emit
#undef foreach

#include <libcamera/libcamera/camera_manager.h>
#include <libcamera/libcamera/camera.h>
#include <libcamera/libcamera/controls.h>
#include <libcamera/libcamera/formats.h>
#include <libcamera/libcamera/stream.h>
#include <libcamera/libcamera/framebuffer.h>
#include <libcamera/libcamera/framebuffer_allocator.h>
#include <libcamera/libcamera/request.h>
#include <libcamera/libcamera.h>


// namespace libcamera {
//     namespace controls {
//         extern const ControlId Brightness;
//         extern const ControlId Contrast;
//         extern const ControlId FrameDurationLimits;
//     }
// }

/**
 * Video Capture Settings
 **/
struct Libcam2OpenCVSettings {
    /**
     * Width of the video capture. A zero lets libcamera decide the width.
     **/
    unsigned int width = 560;
    
    /**
     * Height of the video capture. A zero lets libcamera decide the height.
     **/
    unsigned int height = 360;

    /**
     * Framerate. A zero lets libcamera decide the framerate.
     **/
    unsigned int framerate = 0;

    /**
     * Brightness adjustment (-1.0 to 1.0)
     **/
    float brightness = 0.0;

    /**
     * Contrast adjustment (0.0 to 2.0)
     **/
    float contrast = 1.0;
};

class Libcam2OpenCV {
public:
    /**
     * Callback interface for frame processing
     */
    struct Callback {
        /**
         * Called when a new frame is available
         * @param frame OpenCV Mat containing the captured image
         * @param metadata Metadata associated with the frame
         */
        virtual void hasFrame(const cv::Mat &frame, const libcamera::ControlList &metadata) = 0;
        
        /**
         * Virtual destructor to ensure proper cleanup of derived classes
         */
        virtual ~Callback() = default;
    };

    /**
     * Register a callback to receive frames
     * @param cb Pointer to the callback object
     */
    void registerCallback(Callback* cb) {
        callback = cb;
    }

    /**
     * Start the camera capture with given settings
     * @param settings Camera and capture configuration
     */
    void start(Libcam2OpenCVSettings settings = Libcam2OpenCVSettings());

    /**
     * Stop the camera capture and release resources
     */
    void stop();

private:
    // Camera management objects
    std::unique_ptr<libcamera::CameraManager> cm;
    std::shared_ptr<libcamera::Camera> camera;
    
    // Buffer mapping and management
    std::map<libcamera::FrameBuffer*, std::vector<libcamera::Span<uint8_t>>> mapped_buffers;
    std::unique_ptr<libcamera::CameraConfiguration> config;
    
    // Frame processing
    cv::Mat frame;
    Callback* callback = nullptr;
    
    // Camera resources
    libcamera::FrameBufferAllocator* allocator = nullptr;
    libcamera::Stream* stream = nullptr;
    std::vector<std::unique_ptr<libcamera::Request>> requests;
    libcamera::ControlList controls;

    /**
     * Memory map a frame buffer
     * @param buffer Pointer to the frame buffer
     * @return Vector of memory spans for the buffer
     */
    std::vector<libcamera::Span<uint8_t>> Mmap(libcamera::FrameBuffer* buffer) const {
        auto item = mapped_buffers.find(buffer);
        return (item != mapped_buffers.end()) ? item->second : std::vector<libcamera::Span<uint8_t>>();
    }

    /**
     * Internal callback for request completion
     * @param request Completed camera request
     */
    void requestComplete(libcamera::Request* request);
};

#endif // __LIBCAM2OPENCV