#include "libcam2opencv.h"

// using namespace libcamera;

void Libcam2OpenCV::requestComplete(libcamera::Request *request) {
    if (nullptr == request) return;
    if (request->status() == libcamera::Request::RequestCancelled)
	return;

    const libcamera::ControlList &requestMetadata = request->metadata();
    
    const libcamera::Request::BufferMap &buffers = request->buffers();
    for (auto bufferPair : buffers) {
	libcamera::FrameBuffer *buffer = bufferPair.second;
	libcamera::StreamConfiguration &streamConfig = config->at(0);
	unsigned int vw = streamConfig.size.width;
	unsigned int vh = streamConfig.size.height;
	unsigned int vstr = streamConfig.stride;
	auto mem = Mmap(buffer);
	frame.create(vh,vw,CV_8UC3);
	uint ls = vw*3;
	uint8_t *ptr = mem[0].data();
	for (unsigned int i = 0; i < vh; i++, ptr += vstr) {
	    memcpy(frame.ptr(i),ptr,ls);
	}
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
	if (nullptr != callback) {
	    callback->hasFrame(frame, requestMetadata);
	}
    }

    if (nullptr == request) return;
    if (request->status() == libcamera::Request::RequestCancelled)
	return;
    /* Re-queue the Request to the camera. */
    request->reuse(libcamera::Request::ReuseBuffers);
    camera->queueRequest(request);
}

void Libcam2OpenCV::start(Libcam2OpenCVSettings settings) {

    cm  = std::make_unique<libcamera::CameraManager>();
    cm->start();

    for (auto const &camera : cm->cameras())
	std::cerr << " - " << camera.get()->id() << std::endl;

    if (cm->cameras().empty()) {
	std::cerr << "No cameras were identified on the system."
		  << std::endl;
	cm->stop();
	return;
    }
	
    std::string cameraId = cm->cameras()[0]->id();
    camera = cm->get(cameraId);
    camera->acquire();


    config = camera->generateConfiguration( { libcamera::StreamRole::Viewfinder } );

    libcamera::StreamConfiguration &streamConfig = config->at(0);
	
    if ((settings.width > 0) && (settings.height > 0)) {
	streamConfig.size.width = settings.width;
	streamConfig.size.height = settings.height;
	int ret = camera->configure(config.get());
	if (ret) {
	    std::cerr << "CONFIGURATION FAILED!" << std::endl;
	    return;
	}
    }

    // opencv compatible format
    streamConfig.pixelFormat = libcamera::formats::BGR888;

    config->validate();

    camera->configure(config.get());

    allocator = new libcamera::FrameBufferAllocator(camera);

    for (libcamera::StreamConfiguration &cfg : *config) {
	int ret = allocator->allocate(cfg.stream());
	if (ret < 0) {
	    std::cerr << "Can't allocate buffers" << std::endl;
	    return;
	}
	    
	for (const std::unique_ptr<libcamera::FrameBuffer> &buffer : allocator->buffers(cfg.stream()))
	    {
		size_t buffer_size = 0;
		for (unsigned i = 0; i < buffer->planes().size(); i++)
		    {
			const libcamera::FrameBuffer::Plane &plane = buffer->planes()[i];
			buffer_size += plane.length;
			if (i == buffer->planes().size() - 1 || plane.fd.get() != buffer->planes()[i + 1].fd.get())
			    {
				void *memory = mmap(NULL, buffer_size, PROT_READ | PROT_WRITE, MAP_SHARED, plane.fd.get(), 0);
				mapped_buffers[buffer.get()].push_back(
								       libcamera::Span<uint8_t>(static_cast<uint8_t *>(memory), buffer_size));
				buffer_size = 0;
			    }
		    }
	    }
    }

    stream = streamConfig.stream();
    const std::vector<std::unique_ptr<libcamera::FrameBuffer>> &buffers = allocator->buffers(stream);
    for (unsigned int i = 0; i < buffers.size(); ++i) {
	std::unique_ptr<libcamera::Request> request = camera->createRequest();
	if (!request)
	    {
		std::cerr << "Can't create request" << std::endl;
		return;
	    }

	const std::unique_ptr<libcamera::FrameBuffer> &buffer = buffers[i];
	int ret = request->addBuffer(stream, buffer.get());
	if (ret < 0)
	    {
		std::cerr << "Can't set buffer for request"
			  << std::endl;
		return;
	    }

	requests.push_back(std::move(request));
    }

	camera->requestCompleted.connect(this,&Libcam2OpenCV::requestComplete);

	if (settings.framerate > 0) {
		int64_t frame_time = 1000000 / settings.framerate; // in us
		controls.set(libcamera::controls::FrameDurationLimits, libcamera::Span<const int64_t, 2>({ frame_time, frame_time }));
	}
	controls.set(libcamera::controls::Brightness, static_cast<float>(settings.brightness));
	controls.set(libcamera::controls::Contrast, static_cast<float>(settings.contrast));
	
	camera->start(&controls);
	for (std::unique_ptr<libcamera::Request> &request : requests)
		camera->queueRequest(request.get());
}

void Libcam2OpenCV::stop() {
    try {
        // 立即停止所有操作
        if (camera) {
            // 斷開所有信號連接
            camera->requestCompleted.disconnect(this);
            
            // 強制停止
            camera->stop();
            
            // 釋放所有請求
            for (auto& request : requests) {
                request->reuse(libcamera::Request::ReuseBuffers);
            }
            
            // 等待一段時間
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            
            // 釋放相機
            camera->release();
        }
        
        // 清理管理器
        if (cm) {
            cm->stop();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "停止相機時發生錯誤: " << e.what() << std::endl;
    }
}