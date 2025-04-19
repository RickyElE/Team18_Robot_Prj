
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
    const socket = new WebSocket('ws://192.168.1.100:8084');
    socket.binaryType = "arraybuffer";

    let isCameraRequested = false;
    
    // 連接打開時
    socket.onopen = function() {
        console.log('已連接到機器人數據伺服器');
        // 重置重连次数
        reconnectAttempts = 0;
    };
    
    // 接收到消息時
    socket.onmessage = function(event) {
        console.log('收到消息类型:', typeof event.data);
        console.log('消息内容:', event.data);
        console.log('是否为ArrayBuffer:', event.data instanceof ArrayBuffer);

        try {
            // 首先检查是否是二进制数据
            if (event.data instanceof ArrayBuffer) {
                console.log('收到二进制图像数据');
                
                const blob = new Blob([event.data], { type: 'image/jpeg' });
                const url = URL.createObjectURL(blob);
                
                const imgElement = document.getElementById('camera-stream');
                if (imgElement) {
                    imgElement.src = url;
                    imgElement.style.display = 'block';
                    
                    imgElement.onload = () => {
                        URL.revokeObjectURL(url);
                    };
                }
                return; // 阻止继续执行
            }
    

            // 解析從伺服器收到的 JSON 數據
            const data = JSON.parse(event.data);
            console.log('收到數據:', data);

            // // 處理截圖響應
            // if (data.type === 'screenshot' && data.imageData) {
            //     // 創建圖片元素
            //     const img = document.createElement('img');
            //     img.src = 'data:image/jpeg;base64,' + data.imageData;
            //     img.setAttribute('alt', 'Screenshot ' + new Date().toLocaleTimeString());
            //     img.className = 'screenshot-image';
        
            //     // screenshot-time
            //     const now = new Date();
            //     const timeString = now.toLocaleTimeString('en-US');
            //     const timestamp = document.createElement('div');
            //     timestamp.textContent = timeString;
            //     timestamp.className = 'screenshot-timestamp';
        
            //     // 創建容器
            //     const container = document.createElement('div');
            //     container.className = 'screenshot-item';
            //     container.appendChild(img);
            //     container.appendChild(timestamp);
        
            //     // 添加到截圖列表
            //     screenshotList.appendChild(container);
        
            //     // 滾動到新添加的截圖
            //     setTimeout(() => {
            //         screenshotList.scrollLeft = screenshotList.scrollWidth;
            //     }, 100);
        
            //     console.log('收到並顯示了來自樹莓派的截圖');
            // }

            // const buffer = event.data;

            // const view = new DataView(buffer);


            // const jsonLen = view.getUint32(0,false);
            // console.log("JSON 长度:", jsonLen);

            // const jsonBytes = new Uint8Array(buffer, 4, jsonLen);
            // const jsonText = new TextDecoder().decode(jsonBytes);
            // const data = JSON.parse(jsonText);
            // console.log('收到數據:', data);

            // const jpegBytes = new Uint8Array(buffer, 4 + jsonLen);
            // const blob = new Blob([jpegBytes], { type: "image/jpeg" });
            // const url = URL.createObjectURL(blob);

            // const img = document.getElementById("camera-stream");

            // if (isCameraRequested){
            //     img.src = url;
            //     img.onload = () => URL.revokeObjectURL(url);
            // }



            // 使用您的現有函數更新資源顯示
            updateSystemResources(data);
            
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
function updateGauges(distance) {
  
     // 將距離數值四捨五入到小數點後兩位
     const formattedDistance = parseFloat(distance).toFixed(2);

    // Update distance gauge
    const distancePointer = document.getElementById('distance-pointer');
    const distanceText = document.getElementById('distance-text');
    const distanceAngle = (formattedDistance / maxDistance) * 180 - 90;
    distancePointer.style.transform = `rotate(${distanceAngle}deg)`;
    distanceText.textContent = `${formattedDistance} cm`;
}

// Start camera function
async function startCamera() {

    console.log("start camera function");

    document.getElementById('camera-placeholder').style.display = 'none';
    document.getElementById('camera-stream').style.display = 'block';

    startButton.disabled = true;
    stopButton.disabled = false;
    captureButton.disabled = false;


    if (window.robotSocket && window.robotSocket.readyState === WebSocket.OPEN) {
        window.robotSocket.send(JSON.stringify({
            type: "command", 
            action: "start_camera"
        }));
        console.log("start_camera");
    }
}

// Stop camera function
function stopCamera() {
    // 移除所有事件監聽器再清除源

    if (!window.robotSocket){
        console.log("websocket不存在");
    }
    else{
        console.log("websocket 存在");
    }

    if (window.robotSocket && window.robotSocket.readyState === WebSocket.OPEN) {
        window.robotSocket.send(JSON.stringify({
            type: "command", 
            action: "stop_camera"
        }));
        console.log("sending camera stop message");
    }
    
    document.getElementById('camera-placeholder').style.display = 'flex';
    document.getElementById('camera-stream').style.display = 'none';

    startButton.disabled = false;
    stopButton.disabled = true;
    captureButton.disabled = true;
    
    console.log('攝影機已停止');
}

// Handle chassis direction function
function handleDirection(direction) {
    console.log(`Moving: ${direction}`);
   
    // Simulate gauge changes (replace with actual values from robot)
    let currentDistance = parseInt(document.getElementById('distance-text').textContent);
    let moveInterval = null;

    if (direction === 'Forward') {
        window.robotSocket.send(JSON.stringify({type:"command", action: "move_forward"}));
    } 
    else 
    if (direction === 'Backward') {
        window.robotSocket.send(JSON.stringify({type:"command", action: "move_back"}));
    }
    else 
    if (direction === 'Left') {
        window.robotSocket.send(JSON.stringify({type:"command", action: "move_left"}));
    }
    else 
    if (direction === 'Right') {
        window.robotSocket.send(JSON.stringify({type:"command", action: "move_right"}));
    }
     // Random distance change for demo purposes
     currentDistance = Math.max(0, Math.min(maxDistance, currentDistance + (Math.random() * 20 - 10)));
    
     updateGauges(currentDistance);
}


  // 添加handleArmControl 函數
function handleArmControl(action) {
    console.log(`Arm action: ${action}`);

    if (window.robotSocket && window.robotSocket.readyState === WebSocket.OPEN) {
        if (action === 'Head Up') {
            window.robotSocket.send(JSON.stringify({type: "command", action: "up"}));
        } else if (action === 'Head Down') {
            window.robotSocket.send(JSON.stringify({type: "command", action: "down"}));
        }else if (action === 'Forward'){
            window.robotSocket.send(JSON.stringify({type: "command", action: "For"}));
        }else if (action === 'Backward'){
            window.robotSocket.send(JSON.stringify({type: "command", action: "Back"}));
        }else if (action === 'Rotate Left'){
            window.robotSocket.send(JSON.stringify({type: "command", action: "Rotate_L"}));
        }else if (action === 'Rotate Right'){
            window.robotSocket.send(JSON.stringify({type: "command", action: "Rotate_R"}));
        }else if (action === 'Cut'){
            window.robotSocket.send(JSON.stringify({type: "command", action: "cut"}));
        }else if (action === 'Release'){
            window.robotSocket.send(JSON.stringify({type: "command", action: "release"}));
        
    } else {
        console.error('WebSocket 尚未連線');
    }
}
}
   

// Handle camera movement
function handleCameraMove(direction) {
    console.log(`攝像頭移動: ${direction}`);


    if (direction === 'up') {
        window.robotSocket.send(JSON.stringify({type:"command", action: "camera_move_up"}));
    } 
    else if (direction === 'down') {
        window.robotSocket.send(JSON.stringify({type:"command", action: "camera_move_down"}));
    }
    else{
        console.error()
    }
    
    // 確保 WebSocket 已連接
    // if (window.robotSocket && window.robotSocket.readyState === WebSocket.OPEN) {
    //     try {
    //         if (direction === "up") {
    //             window.robotSocket.send(JSON.stringify({
    //                 type: "command", 
    //                 action: "camera_move_up"
    //             }));
    //         } else if (direction === "down") {
    //             window.robotSocket.send(JSON.stringify({
    //                 type: "command", 
    //                 action: "camera_move_down"
    //             }));
    //         }
    //     } catch (error) {
    //         console.error('發送 WebSocket 命令時出錯:', error);
    //     }
    // } else {
    //     console.error('WebSocket 未連接，無法發送攝像頭移動命令');
    // }
}


// Capture screenshot
function captureScreenshot() {


    console.log("開始截圖...");
    
    // 獲取視頻元素
    const videoElement = document.getElementById('camera-stream');
    console.log("video dimensions:", videoElement.videoWidth, videoElement.videoHeight);
    // 創建 Canvas 元素
    const canvas = document.createElement('canvas');
    canvas.width = videoElement.naturalWidth;
    canvas.height = videoElement.naturalHeight;
    
    // 在 Canvas 上繪製當前視頻幀
    const ctx = canvas.getContext('2d');
    ctx.drawImage(videoElement, 0, 0, canvas.width, canvas.height);
    
    // 將 Canvas 轉換為圖片
    const imgSrc = canvas.toDataURL('image/jpeg');
    
    // 創建圖片元素
    const img = document.createElement('img');
    img.src = imgSrc;
    img.className = 'screenshot-image';
    img.style.width = '160px';
    img.style.height = '120px';
    img.style.objectFit = 'cover';
    img.style.border = '1px solid #ddd';
    img.style.borderRadius = '4px';
    img.style.transform = 'rotate(180deg)';
    
    // 創建時間戳
    const now = new Date();
    const timeString = now.toLocaleTimeString();
    const timestamp_elem = document.createElement('div');
    timestamp_elem.textContent = timeString;
    timestamp_elem.className = 'screenshot-timestamp';
    
    // 創建容器
    const container = document.createElement('div');
    container.className = 'screenshot-item';
    container.appendChild(img);
    container.appendChild(timestamp_elem);
    
    // 添加到截圖列表
    screenshotList.appendChild(container);
    
    // 滾動到新添加的截圖
    setTimeout(() => {
        screenshotList.scrollLeft = screenshotList.scrollWidth;
    }, 100);
    
    console.log('已添加視頻畫面作為截圖');
    

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
    
    console.log("收到的完整數据:", data);
    
    // 更新电池电量
    if (data.percentage !== undefined) {
        console.log("更新電池百分比:", data.percentage);
        updateBattery(data.percentage);
    }
    
    // 更新电压
    if (data.voltage !== undefined) {
        updateBatteryVoltage(data.voltage);
    }
 // 更新 CPU info
 if (data.cpu) {
    if (data.cpu.temperature !== undefined) {
        updateProgressBar('cpu-temperature', data.cpu.temperature);
    }
    if (data.cpu.usage !== undefined) {
        updateProgressBar('cpu-usage', data.cpu.usage);
    }
}
//更新ram/swap
    if (data.memory.ram !== undefined) updateProgressBar('ram-usage', data.memory.ram);
    if (data.memory.swap !== undefined) updateProgressBar('swap-usage', data.memory.swap);

// 更新distance
if (data.distance !== undefined) updateGauges(data.distance);    

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
    
    // 根據 ID 判斷是否為 CPU 溫度
    if (id === 'cpu-temperature') {
        // 溫度使用度數單位
        percentageLabel.textContent = `${Math.round(value)}°C`;
    } else {
        // 其他進度條使用百分比單位
        percentageLabel.textContent = `${Math.round(value)}%`;
    }
    
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
    
    if (!batteryLevel || !batteryPercentage) {
        console.error("找不到電池 DOM 元素");
        return;
    }
    
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
    if (!movementTab || !gripperTab ||  
        !movementBtn || !gripperBtn ) {
        console.error("Mechanical arm controfl tab elements don't exist, can't initialize");
        return;
    }
    
    console.log("Found all tab elements, setting up event listeners");
    
    // Set click events - movement control tab
    movementBtn.onclick = function() {
        movementBtn.classList.add('active');
        gripperBtn.classList.remove('active');
        
        movementTab.classList.add('active');
        gripperTab.classList.remove('active');
        
        console.log("Switched to movement control tab");
    };

    // Set click events - gripper control tab
       gripperBtn.onclick = function() {
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
        release: document.getElementById('arm-release-btn'),
        rotateLeft: document.getElementById('arm-rotate-left-btn'),
        rotateRight: document.getElementById('arm-rotate-right-btn'),
       
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
    updateGauges(0);
    
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
