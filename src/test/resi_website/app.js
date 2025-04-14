// Check login status
if (!localStorage.getItem('isLoggedIn')) {
    window.location.href = 'login.html';
}

// Set user information
const username = localStorage.getItem('username') || 'User';
document.getElementById('userInitial').textContent = username.charAt(0).toUpperCase();
document.getElementById('userName').textContent = username;
document.getElementById('userEmail').textContent = username.toLowerCase() + '@example.com';

// Dropdown menu control
const accountBtn = document.getElementById('accountBtn');
const dropdownMenu = document.getElementById('dropdownMenu');

accountBtn.addEventListener('click', () => {
    dropdownMenu.classList.toggle('show');
});

// Close dropdown when clicking elsewhere
window.addEventListener('click', (e) => {
    if (!accountBtn.contains(e.target) && !dropdownMenu.contains(e.target)) {
        dropdownMenu.classList.remove('show');
    }
});

// Logout function
function handleLogout() {
    localStorage.removeItem('isLoggedIn');
    localStorage.removeItem('username');
    window.location.href = 'login.html';
}

// DOM Elements
const videoElement = document.getElementById('camera-stream');
const startButton = document.getElementById('start-btn');
const stopButton = document.getElementById('stop-btn');
const captureButton = document.getElementById('capture-btn');
const errorMessage = document.getElementById('error-message');
const screenshotList = document.getElementById('screenshot-list');
const cameraUpButton = document.getElementById('camera-up-btn');
const cameraDownButton = document.getElementById('camera-down-btn');

// Direction control buttons
const directionButtons = {
    forward: document.getElementById('forward-btn'),
    back: document.getElementById('back-btn'),
    left: document.getElementById('left-btn'),
    right: document.getElementById('right-btn')
};

// Variables
let mediaStream = null;
const maxSpeed = 100;
const maxDistance = 200;

// WebSocket 連接設置
function setupWebSocketConnection() {
    console.log("正在建立 WebSocket 連線...");
    
    // 替換為您的樹莓派 IP 
    const socket = new WebSocket('ws://192.168.137.202:8084');
    
    // 連接打開時
    socket.onopen = function() {
        console.log('已連接到機器人數據伺服器');
        // 重置重连次数
        reconnectAttempts = 0;
    };
    
    // 接收到消息時
    socket.onmessage = function(event) {
        try {
            // 解析從伺服器收到的 JSON 數據
            const data = JSON.parse(event.data);
            console.log('收到數據:', data);
            
            // 創建系統資源數據格式
            const resourceData = {};
             // 直接传入完整数据
        updateSystemResources(data);

          // 直接使用数据更新
        updateSystemResources({
            batteryLevel: data.percentage,
            voltage: data.voltage
        });
            
            // 如果有其他數據(例如 CPU, GPU)，也可以添加到 resourceData
            // if (data.cpu) 
            {
                // resourceData.cpuUsage = data.cpu.usage;
                resourceData.cpuTemp = data.cpu.temperature;
            }
            
            if (data.gpu) {
                resourceData.gpuUsage = data.gpu.usage;
                resourceData.gpuTemp = data.gpu.temperature;
            }
            
            if (data.memory) {
                resourceData.ramUsage = data.memory.ram;
                resourceData.swapUsage = data.memory.swap;
            }
            
            // 使用您的現有函數更新資源顯示
            updateSystemResources(resourceData);
            
        } catch (error) {
            console.error('處理數據時出錯:', error);
        }
    };
    
    // 連接關閉時
    socket.onclose = function() {
        console.log('與機器人數據伺服器的連接已關閉');
        // 5秒後嘗試重新連接
        reconnect();
    };
    
    // 連接錯誤時
    socket.onerror = function(error) {
        console.error('WebSocket 錯誤:', error);
        reconnect();
    };
    
    // 保存 socket 引用以便在頁面卸載時關閉
    window.robotSocket = socket;
    let reconnectAttempts = 0;
function reconnect() {
    if (reconnectAttempts < 5) {
        reconnectAttempts++;
        const delay = Math.pow(2, reconnectAttempts) * 1000;
        
        setTimeout(() => {
            console.log(`嘗試重新連接 (${reconnectAttempts}/5)`);
            setupWebSocketConnection();
        }, delay);
    } else {
        console.error('無法重新建立WebSocket連接');
    }
    }
}


// Function to update gauges
function updateGauges(speed, distance) {
    // Update speed gauge
    const speedPointer = document.getElementById('speed-pointer');
    const speedText = document.getElementById('speed-text');
    const speedAngle = (speed / maxSpeed) * 180 - 90;
    speedPointer.style.transform = `rotate(${speedAngle}deg)`;
    speedText.textContent = `${speed} km/h`;

    // Update distance gauge
    const distancePointer = document.getElementById('distance-pointer');
    const distanceText = document.getElementById('distance-text');
    const distanceAngle = (distance / maxDistance) * 180 - 90;
    distancePointer.style.transform = `rotate(${distanceAngle}deg)`;
    distanceText.textContent = `${distance} cm`;
}

// Start camera function
async function startCamera() {
    try {
        mediaStream = await navigator.mediaDevices.getUserMedia({
            video: { 
                facingMode: "environment"
            }, 
            audio: false 
        });

        videoElement.srcObject = mediaStream;
        startButton.disabled = true;
        stopButton.disabled = false;
        captureButton.disabled = false;
        if (errorMessage) errorMessage.textContent = '';
    } catch (err) {
        console.error('Camera access error:', err);
        if (errorMessage) errorMessage.textContent = `Error: ${err.message}`;
    }
}

// Stop camera function
function stopCamera() {
    if (mediaStream) {
        mediaStream.getTracks().forEach(track => track.stop());
        videoElement.srcObject = null;
        startButton.disabled = false;
        stopButton.disabled = true;
        captureButton.disabled = true;
    }
}

// Handle direction function
function handleDirection(direction) {
    console.log(`Moving: ${direction}`);
    // Add your Raspberry Pi control logic here
    
    // Simulate gauge changes (replace with actual values from robot)
    let currentSpeed = parseInt(document.getElementById('speed-text').textContent);
    let currentDistance = parseInt(document.getElementById('distance-text').textContent);
    let moveInterval = null;
    
    if (direction === 'forward') {
        currentSpeed = Math.min(maxSpeed, currentSpeed + 10);
        window.robotSocket.send(JSON.stringify({type:"command", action: "move_forward"}));
    } 
    else 
    if (direction === 'back') {
        currentSpeed = Math.max(0, currentSpeed - 10);
        window.robotSocket.send(JSON.stringify({type:"command", action: "move_back"}));
    }
    else 
    if (direction === 'left') {
        currentSpeed = Math.max(0, currentSpeed - 10);
        window.robotSocket.send(JSON.stringify({type:"command", action: "move_left"}));
    }
    else 
    if (direction === 'right') {
        currentSpeed = Math.max(0, currentSpeed - 10);
        window.robotSocket.send(JSON.stringify({type:"command", action: "move_right"}));
    }
    
    // Random distance change for demo purposes
    currentDistance = Math.max(0, Math.min(maxDistance, currentDistance + (Math.random() * 20 - 10)));
    
    updateGauges(currentSpeed, currentDistance);
}

// Handle camera movement
function handleCameraMove(direction) {
    console.log(`Camera moving: ${direction}`);
    // Add camera servo control logic here
}

// Function to handle mechanical arm control
function handleArmControl(action) {
    console.log(`Arm action: ${action}`);
    // Add code to control the mechanical arm via Raspberry Pi
}

// Capture screenshot
function captureScreenshot() {
    if (!videoElement.srcObject) return;
    
    // Create canvas
    const canvas = document.createElement('canvas');
    const ctx = canvas.getContext('2d');
    
    // Set canvas dimensions
    canvas.width = videoElement.videoWidth;
    canvas.height = videoElement.videoHeight;
    ctx.drawImage(videoElement, 0, 0, canvas.width, canvas.height);
    
    // Get image URL
    const imgDataURL = canvas.toDataURL('image/png');
    
    // Create image element
    const img = document.createElement('img');
    img.src = imgDataURL;
    img.setAttribute('alt', 'Screenshot ' + new Date().toLocaleTimeString());
    
    // Create timestamp element
    const now = new Date();
    const timeString = now.toLocaleTimeString();
    const timestamp = document.createElement('div');
    timestamp.textContent = timeString;
    timestamp.className = 'screenshot-timestamp';
    
    // Create container for the image and timestamp
    const container = document.createElement('div');
    container.className = 'screenshot-item';
    container.appendChild(img);
    container.appendChild(timestamp);
    
    // Add to screenshot list
    screenshotList.appendChild(container);
    
    // Scroll to the newly added screenshot
    setTimeout(() => {
        screenshotList.scrollLeft = screenshotList.scrollWidth;
    }, 100);
}

// Fix screenshot scroll functionality
function fixScreenshotScroll() {
    const screenshotContainer = document.querySelector('.screenshot-container');
    const screenshotList = document.getElementById('screenshot-list');
    
    if (!screenshotContainer || !screenshotList) {
        console.error('Cannot find screenshot container or list');
        return;
    }
    
    // Ensure screenshot list can scroll
    screenshotList.style.overflowX = 'auto';
    screenshotList.style.display = 'flex';
    screenshotList.style.flexWrap = 'nowrap';
    screenshotList.style.scrollBehavior = 'smooth';
    
    // Remove any existing scroll buttons
    const oldButtons = screenshotContainer.querySelectorAll('.scroll-btn');
    oldButtons.forEach(btn => btn.remove());
    
    // Add buttons for left and right scroll
    const scrollLeftBtn = document.createElement('button');
    scrollLeftBtn.className = 'scroll-btn scroll-left-btn';
    scrollLeftBtn.innerHTML = '&lt;';
    scrollLeftBtn.title = 'Scroll left';
    
    const scrollRightBtn = document.createElement('button');
    scrollRightBtn.className = 'scroll-btn scroll-right-btn';
    scrollRightBtn.innerHTML = '&gt;';
    scrollRightBtn.title = 'Scroll right';
    
    // Add buttons to container
    screenshotContainer.appendChild(scrollLeftBtn);
    screenshotContainer.appendChild(scrollRightBtn);
    
    // Set button click events
    scrollLeftBtn.addEventListener('click', () => {
        screenshotList.scrollBy({ left: -200, behavior: 'smooth' });
    });
    
    scrollRightBtn.addEventListener('click', () => {
        screenshotList.scrollBy({ left: 200, behavior: 'smooth' });
    });
    
    // Add wheel event handling
    screenshotContainer.addEventListener('wheel', (e) => {
        if (e.deltaY !== 0) {
            e.preventDefault(); // Prevent vertical scrolling
            screenshotList.scrollLeft += e.deltaY;
        }
    }, { passive: false });
    
    console.log('Screenshot scroll functionality fixed');
}

// Scroll variables
let scrollInterval = null;
let scrollSpeed = 3; // pixels per scroll interval
let isMouseInContainer = false;
let mouseX = 0;
let containerWidth = 0;

function createScrollZones() {
    const container = document.querySelector('.screenshot-container');
    if (!container) return;
    
    containerWidth = container.offsetWidth;
    
    container.addEventListener('mousemove', (e) => {
        const rect = container.getBoundingClientRect();
        mouseX = e.clientX - rect.left;
        
        const relativeX = mouseX / containerWidth;
        
        if (relativeX < 0.3) {
            scrollWithSpeed('left', Math.max(1, 10 * (0.3 - relativeX)));
        } else if (relativeX > 0.7) {
            scrollWithSpeed('right', Math.max(1, 10 * (relativeX - 0.7)));
        } else {
            stopScroll();
        }
    });
    
    container.addEventListener('mouseleave', stopScroll);
    window.addEventListener('resize', () => {
        containerWidth = container.offsetWidth;
    });
    
    // Ensure the fix function is called
    fixScreenshotScroll();
}

function scrollWithSpeed(direction, speed) {
    const screenshotList = document.getElementById('screenshot-list');
    if (!screenshotList) return;
    
    stopScroll();
    
    scrollInterval = requestAnimationFrame(function smoothScroll() {
        if (direction === 'left') {
            screenshotList.scrollLeft -= speed;
        } else {
            screenshotList.scrollLeft += speed;
        }
        scrollInterval = requestAnimationFrame(smoothScroll);
    });
}

function stopScroll() {
    if (scrollInterval) {
        cancelAnimationFrame(scrollInterval);
        scrollInterval = null;
    }
}

// Initialize empty system resource containers
function initializeSystemResources() {
    // Get all progress bar elements
    const resourceElements = [
        { id: 'cpu-usage', label: 'CPU Usage' },
        { id: 'cpu-temperature', label: 'CPU Temp' },
        { id: 'gpu-usage', label: 'GPU Usage' },
        { id: 'gpu-temperature', label: 'GPU Temp' },
        { id: 'ram-usage', label: 'RAM Usage' },
        { id: 'swap-usage', label: 'Swap Usage' }
    ];

    // Initialize progress bars and labels
    resourceElements.forEach(resource => {
        const progressBar = document.getElementById(resource.id);
        if (!progressBar) return; // Ensure element exists
        
        // Set initial progress bar value to 0
        progressBar.value = 0;
        
        const progressContainer = progressBar.closest('.progress-container');
        
        // Find or create percentage label
        let percentageLabel = progressContainer.querySelector('.percentage-label');
        if (!percentageLabel) {
            percentageLabel = document.createElement('span');
            percentageLabel.classList.add('percentage-label');
            progressContainer.appendChild(percentageLabel);
        }
        
        // Set label to waiting state
        percentageLabel.textContent = "Waiting for data...";
        percentageLabel.style.marginLeft = '10px';
        percentageLabel.style.color = '#aaa';
        percentageLabel.style.fontSize = '14px';
    });
    
    // Initialize battery display
    const batteryLevel = document.getElementById("battery-level");
    const batteryPercentage = document.getElementById("battery-percentage");
    
    if (batteryLevel) {
        batteryLevel.style.width = "0%"; // Initial battery level at 0
    }
    
    if (batteryPercentage) {
        batteryPercentage.textContent = "Battery: Waiting for data...";
    }
    
    console.log("System resource containers initialized, waiting for real data");
}

// Function to update system resources with real data
function updateSystemResources(data) {
    
    if (!data) {
        console.log("No data provided to updateSystemResources");
        return;
    }
    
    console.log("收到的完整数据:", data);
    
    // 更新电池电量
    if (data.percentage !== undefined) {
        updateBattery(data.percentage);
    }
    
    // 更新电压
    if (data.voltage !== undefined) {
        updateBatteryVoltage(data.voltage);
    }

    if (data.cpuTemp !== undefined) updateProgressBar('cpu-temperature', data.cpuTemp);
    /*
    // Update progress bars with real data
    if (data.cpuUsage !== undefined) updateProgressBar('cpu-usage', data.cpuUsage);
    
    if (data.gpuUsage !== undefined) updateProgressBar('gpu-usage', data.gpuUsage);
    if (data.gpuTemp !== undefined) updateProgressBar('gpu-temperature', data.gpuTemp);
    if (data.ramUsage !== undefined) updateProgressBar('ram-usage', data.ramUsage);
    if (data.swapUsage !== undefined) updateProgressBar('swap-usage', data.swapUsage);
    */ 
    
}

// Helper function to update a single progress bar
function updateProgressBar(id, value) {
    const progressBar = document.getElementById(id);
    if (!progressBar) return;

    const progressContainer = progressBar.closest('.progress-container');
    
    // Update progress bar value
    progressBar.value = value;

    // Find or create percentage label
    let percentageLabel = progressContainer.querySelector('.percentage-label');
    if (!percentageLabel) {
        percentageLabel = document.createElement('span');
        percentageLabel.classList.add('percentage-label');
        progressContainer.appendChild(percentageLabel);
    }
    
    // Update percentage label
    percentageLabel.textContent = `${Math.round(value)}%`;
    percentageLabel.style.marginLeft = '10px';
    percentageLabel.style.fontSize = '14px';

    // Set color based on value
    if (value > 80) {
        progressBar.style.setProperty('--progress-color', '#FF3D00'); // Danger
        percentageLabel.style.color = '#FF3D00';
    } else if (value > 60) {
        progressBar.style.setProperty('--progress-color', '#FF9800'); // Warning
        percentageLabel.style.color = '#FF9800';
    } else {
        progressBar.style.setProperty('--progress-color', '#4CAF50'); // Normal
        percentageLabel.style.color = '#4CAF50';
    }
}

// Helper function to update battery display
function updateBattery(level) {
    const batteryLevel = document.getElementById("battery-level");
    const batteryPercentage = document.getElementById("battery-percentage");
    
    if (!batteryLevel || !batteryPercentage) return;
    
    // Update battery level
    batteryLevel.style.width = `${level}%`;
    batteryPercentage.textContent = `Battery: ${level}%`;
    
    // Set color based on level
    if (level < 20) {
        batteryLevel.classList.add('critical');
        batteryLevel.classList.remove('low');
    } else if (level < 40) {
        batteryLevel.classList.add('low');
        batteryLevel.classList.remove('critical');
    } else {
        batteryLevel.classList.remove('low', 'critical');
    }
}
 // Update battery voltage
function updateBatteryVoltage(voltage) {
    const batteryVoltage = document.getElementById("battery-voltage");
    if (batteryVoltage) {
        batteryVoltage.textContent = `${voltage.toFixed(2)} V`;
    }
}

// Setup mechanical arm tabs
function setupMechanicalArmTabs() {
    console.log("Setting up mechanical arm control tabs...");
    
    // Get DOM elements
    const movementTab = document.getElementById('movement-tab');
    const gripperTab = document.getElementById('gripper-tab');
    const movementBtn = document.querySelector('.arm-tab-btn[data-tab="movement"]');
    const gripperBtn = document.querySelector('.arm-tab-btn[data-tab="gripper"]');
    
    // Check if elements exist
    if (!movementTab || !gripperTab || !movementBtn || !gripperBtn) {
        console.error("Mechanical arm control tab elements don't exist, can't initialize");
        return;
    }
    
    console.log("Found all tab elements, setting up event listeners");
    
    // Set click events - movement control tab
    movementBtn.onclick = function() {
        // Remove all active states
        movementBtn.classList.add('active');
        gripperBtn.classList.remove('active');
        movementTab.classList.add('active');
        gripperTab.classList.remove('active');
        console.log("Switched to movement control tab");
    };
    
    // Set click events - gripper control tab
    gripperBtn.onclick = function() {
        // Remove all active states
        gripperBtn.classList.add('active');
        movementBtn.classList.remove('active');
        gripperTab.classList.add('active');
        movementTab.classList.remove('active');
        console.log("Switched to gripper control tab");
    };
    
    // Add event listeners for mechanical arm buttons
    const armButtons = {
        up: document.getElementById('arm-up-btn'),
        down: document.getElementById('arm-down-btn'),
        forward: document.getElementById('arm-forward-btn'),
        backward: document.getElementById('arm-backward-btn'),
        cut: document.getElementById('arm-cut-btn'),
        rotateLeft: document.getElementById('arm-rotate-left-btn'),
        rotateRight: document.getElementById('arm-rotate-right-btn')
    };
    
    // Add button event listeners
    Object.entries(armButtons).forEach(([key, button]) => {
        if (button) {
            button.addEventListener('click', () => handleArmControl(button.textContent));
        }
    });
    
    console.log("Mechanical arm control tabs setup complete");
}

// Page load initialization
window.addEventListener('load', function() {
    console.log("Page load complete, starting initialization...");
    
    // Initialize camera controls
    if (startButton) startButton.addEventListener('click', startCamera);
    if (stopButton) stopButton.addEventListener('click', stopCamera);
    if (captureButton) captureButton.addEventListener('click', captureScreenshot);
    
    // Initialize direction controls
    // 20250403 Adding Direction Button Logic
    Object.entries(directionButtons).forEach(([direction, button]) => {
        if (button) {
            button.addEventListener('mousedown', () => handleDirection(direction));
            button.addEventListener('mouseup', () => {
                this.window.robotSocket.send(JSON.stringify({ type: "command", action: "stop" }));
            })

            button.addEventListener('touchstart', () => handleDirection(direction));
            button.addEventListener('touchend', () => {
                window.robotSocket.send(JSON.stringify({ type: "command", action: "stop" }));
            });
        }
    });

    // Initialize Keyboard Direction Control For Robot Moving
    const KeyToAction = {
        ArrowUp: "move_forward",
        ArrowDown: "move_back",
        ArrowLeft: "move_left",
        ArrowRight: "move_right"
    }
    
    const pressedKeys = new Set();

    this.window.addEventListener("keydown", function(event) {
        const action = KeyToAction[event.key];
        if (action && !pressedKeys.has(event.key)){
            pressedKeys.add(event.key);
            console.log(`Key down: ${event.key} → ${action}`);
            window.robotSocket.send(JSON.stringify({ type: "command", action }));
        }
    });

    this.window.addEventListener("keyup", function(event) {
        const action = KeyToAction[event.key];
        if (action) {
            pressedKeys.delete(event.key);
            console.log(`Key up: ${event.key} → stop`);
            window.robotSocket.send(JSON.stringify({ type: "command", action: "stop" }));
        }
    });
    
    // Initialize camera movement controls
    if (cameraUpButton) cameraUpButton.addEventListener('click', () => handleCameraMove('up'));
    if (cameraDownButton) cameraDownButton.addEventListener('click', () => handleCameraMove('down'));
    
    // Initialize mechanical arm tab controls
    setupMechanicalArmTabs();
    
    // Initialize screenshot scroll zones
    createScrollZones();
    
    // Initialize system resource indicators
    initializeSystemResources();
    
    // Setup WebSocket connection
    setupWebSocketConnection();
    
    // Initialize gauges
    updateGauges(0, 0);
    
    // Make sure screenshot scroll functionality works
    setTimeout(fixScreenshotScroll, 500);
    
    // Check for browser support
    if (navigator.mediaDevices && navigator.mediaDevices.getUserMedia) {
        console.log("Camera access is supported by this browser");
    } else if (errorMessage) {
        errorMessage.textContent = 'Your browser does not support camera access';
        if (startButton) startButton.disabled = true;
    }
    
    console.log("Page initialization complete");
});

// 在頁面卸載時關閉 WebSocket 連接
window.addEventListener('beforeunload', function() {
    if (window.robotSocket && window.robotSocket.readyState === WebSocket.OPEN) {
        window.robotSocket.close();
    }
});

// Handle window resize for screenshot scroll
window.addEventListener('resize', function() {
    // Delay execution to avoid frequent calls
    clearTimeout(window.resizeTimer);
    window.resizeTimer = setTimeout(fixScreenshotScroll, 200);
});

// Make sure functionality is available when DOM is loaded
document.addEventListener('DOMContentLoaded', function() {
    // Apply fixes after a slight delay to ensure all elements are loaded
    setTimeout(fixScreenshotScroll, 500);
});