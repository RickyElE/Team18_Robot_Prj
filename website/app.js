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
        errorMessage.textContent = '';
    } catch (err) {
        console.error('Camera access error:', err);
        errorMessage.textContent = `Error: ${err.message}`;
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
    
    if (direction === 'forward') {
        currentSpeed = Math.min(maxSpeed, currentSpeed + 10);
    } else if (direction === 'back') {
        currentSpeed = Math.max(0, currentSpeed - 10);
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

// 修復截圖區塊滾動功能
function fixScreenshotScroll() {
    // 獲取截圖容器和列表元素
    const screenshotContainer = document.querySelector('.screenshot-container');
    const screenshotList = document.getElementById('screenshot-list');
    
    if (!screenshotContainer || !screenshotList) {
        console.error('找不到截圖容器或列表元素');
        return;
    }
    
    // 確保截圖列表可以水平滾動
    screenshotList.style.overflowX = 'auto';
    screenshotList.style.display = 'flex';
    screenshotList.style.flexWrap = 'nowrap';
    screenshotList.style.scrollBehavior = 'smooth';
    
    // 移除可能存在的舊按鈕
    const oldButtons = screenshotContainer.querySelectorAll('.scroll-btn');
    oldButtons.forEach(btn => btn.remove());
    
    // 添加左右滾動按鈕
    const scrollLeftBtn = document.createElement('button');
    scrollLeftBtn.className = 'scroll-btn scroll-left-btn';
    scrollLeftBtn.innerHTML = '&lt;';
    scrollLeftBtn.title = '向左滾動';
    
    const scrollRightBtn = document.createElement('button');
    scrollRightBtn.className = 'scroll-btn scroll-right-btn';
    scrollRightBtn.innerHTML = '&gt;';
    scrollRightBtn.title = '向右滾動';
    
    // 添加按鈕到容器
    screenshotContainer.appendChild(scrollLeftBtn);
    screenshotContainer.appendChild(scrollRightBtn);
    
    // 設置按鈕點擊事件
    scrollLeftBtn.addEventListener('click', () => {
        screenshotList.scrollBy({ left: -200, behavior: 'smooth' });
    });
    
    scrollRightBtn.addEventListener('click', () => {
        screenshotList.scrollBy({ left: 200, behavior: 'smooth' });
    });
    
    // 添加滾輪事件處理
    screenshotContainer.addEventListener('wheel', (e) => {
        if (e.deltaY !== 0) {
            e.preventDefault(); // 防止垂直滾動
            screenshotList.scrollLeft += e.deltaY;
        }
    }, { passive: false });
    
    console.log('截圖滾動功能已修復');
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
    
    // 取得圖片資料 URL
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

// 獲取 DOM 元素
const batteryLevel = document.getElementById("battery-level");
const batteryPercentage = document.getElementById("battery-percentage");

// 設置滾動相關變數
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
    
    // 確保調用修復函數
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

// 初始化空的系統資源容器
function initializeSystemResources() {
    // 獲取所有進度條元素
    const resourceElements = [
        { id: 'cpu-usage', label: 'CPU Usage' },
        { id: 'cpu-temperature', label: 'CPU Temp' },
        { id: 'gpu-usage', label: 'GPU Usage' },
        { id: 'gpu-temperature', label: 'GPU Temp' },
        { id: 'ram-usage', label: 'RAM Usage' },
        { id: 'swap-usage', label: 'Swap Usage' }
    ];

    // 初始化進度條和標籤
    resourceElements.forEach(resource => {
        const progressBar = document.getElementById(resource.id);
        if (!progressBar) return; // 確保元素存在
        
        // 設置進度條初始值為0
        progressBar.value = 0;
        
        const progressContainer = progressBar.closest('.progress-container');
        
        // 找到或創建百分比標籤
        let percentageLabel = progressContainer.querySelector('.percentage-label');
        if (!percentageLabel) {
            percentageLabel = document.createElement('span');
            percentageLabel.classList.add('percentage-label');
            progressContainer.appendChild(percentageLabel);
        }
        
        // 設置標籤為等待數據狀態
        percentageLabel.textContent = "Waiting for data...";
        percentageLabel.style.marginLeft = '10px';
        percentageLabel.style.color = '#aaa';
        percentageLabel.style.fontSize = '14px';
    });
    
    // 初始化電池顯示
    const batteryLevel = document.getElementById("battery-level");
    const batteryPercentage = document.getElementById("battery-percentage");
    
    if (batteryLevel) {
        batteryLevel.style.width = "0%"; // 初始電量為0
    }
    
    if (batteryPercentage) {
        batteryPercentage.textContent = "Battery: Waiting for data...";
    }
    
    console.log("系統資源容器已初始化，等待真實數據");
}

// 用於將來更新系統資源的函數框架
function updateSystemResources(data) {
    // 這個函數將來會用於更新實際數據
    // 參數 data 應該包含所有需要的系統資源信息
    // 例如: data = { cpuUsage: 40, cpuTemp: 45, ... }
    
    console.log("將來這裡會用實際數據更新系統資源顯示");
    
    // 示例用法 (注釋掉，等你準備好實際數據再使用)
    /*
    updateProgressBar('cpu-usage', data.cpuUsage);
    updateProgressBar('cpu-temperature', data.cpuTemp);
    updateProgressBar('gpu-usage', data.gpuUsage);
    updateProgressBar('gpu-temperature', data.gpuTemp);
    updateProgressBar('ram-usage', data.ramUsage);
    updateProgressBar('swap-usage', data.swapUsage);
    
    if (data.batteryLevel !== undefined) {
        updateBattery(data.batteryLevel);
    }
    */
}

// 更新單個進度條的輔助函數
function updateProgressBar(id, value) {
    const progressBar = document.getElementById(id);
    if (!progressBar) return;

    const progressContainer = progressBar.closest('.progress-container');
    
    // 更新進度條值
    progressBar.value = value;

    // 找到或創建百分比標籤
    let percentageLabel = progressContainer.querySelector('.percentage-label');
    if (!percentageLabel) {
        percentageLabel = document.createElement('span');
        percentageLabel.classList.add('percentage-label');
        progressContainer.appendChild(percentageLabel);
    }
    
    // 更新百分比標籤
    percentageLabel.textContent = `${Math.round(value)}%`;
    percentageLabel.style.marginLeft = '10px';
    percentageLabel.style.fontSize = '14px';

    // 根據數值設置顏色
    if (value > 80) {
        progressBar.style.setProperty('--progress-color', '#FF3D00'); // 危險
        percentageLabel.style.color = '#FF3D00';
    } else if (value > 60) {
        progressBar.style.setProperty('--progress-color', '#FF9800'); // 警告
        percentageLabel.style.color = '#FF9800';
    } else {
        progressBar.style.setProperty('--progress-color', '#4CAF50'); // 正常
        percentageLabel.style.color = '#4CAF50';
    }
}

// 更新電池顯示的輔助函數
function updateBattery(level) {
    const batteryLevel = document.getElementById("battery-level");
    const batteryPercentage = document.getElementById("battery-percentage");
    
    if (!batteryLevel || !batteryPercentage) return;
    
    // 更新電池電量
    batteryLevel.style.width = `${level}%`;
    batteryPercentage.textContent = `Battery: ${level}%`;
    
    // 根據電量設置顏色
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

// 簡化的書籤切換函數
function setupMechanicalArmTabs() {
    console.log("設置機械臂控制書籤...");
    
    // 直接獲取DOM元素
    const movementTab = document.getElementById('movement-tab');
    const gripperTab = document.getElementById('gripper-tab');
    const movementBtn = document.querySelector('.arm-tab-btn[data-tab="movement"]');
    const gripperBtn = document.querySelector('.arm-tab-btn[data-tab="gripper"]');
    
    // 檢查元素是否存在
    if (!movementTab || !gripperTab || !movementBtn || !gripperBtn) {
        console.error("機械臂控制書籤元素不存在，無法初始化");
        return;
    }
    
    console.log("找到所有書籤元素，開始設置事件監聽器");
    
    // 設置點擊事件 - 移動控制標籤
    movementBtn.onclick = function() {
        // 移除所有活動狀態
        movementBtn.classList.add('active');
        gripperBtn.classList.remove('active');
        movementTab.classList.add('active');
        gripperTab.classList.remove('active');
        console.log("切換到移動控制標籤");
    };
    
    // 設置點擊事件 - 夾子控制標籤
    gripperBtn.onclick = function() {
        // 移除所有活動狀態
        gripperBtn.classList.add('active');
        movementBtn.classList.remove('active');
        gripperTab.classList.add('active');
        movementTab.classList.remove('active');
        console.log("切換到夾子控制標籤");
    };
    
    // 為機械臂按鈕添加事件監聽器
    const armButtons = {
        up: document.getElementById('arm-up-btn'),
        down: document.getElementById('arm-down-btn'),
        forward: document.getElementById('arm-forward-btn'),
        backward: document.getElementById('arm-backward-btn'),
        cut: document.getElementById('arm-cut-btn'),
        rotateLeft: document.getElementById('arm-rotate-left-btn'),
        rotateRight: document.getElementById('arm-rotate-right-btn')
    };
    
    // 添加按鈕事件監聽器
    Object.entries(armButtons).forEach(([key, button]) => {
        if (button) {
            button.addEventListener('click', () => handleArmControl(button.textContent));
        }
    });
    
    console.log("機械臂控制書籤設置完成");
}

// 頁面加載初始化
window.addEventListener('load', function() {
    console.log("頁面加載完成，開始初始化...");
    
    // 初始化攝像頭控制
    if (startButton) startButton.addEventListener('click', startCamera);
    if (stopButton) stopButton.addEventListener('click', stopCamera);
    if (captureButton) captureButton.addEventListener('click', captureScreenshot);
    
    // 初始化方向控制
    Object.entries(directionButtons).forEach(([direction, button]) => {
        if (button) {
            button.addEventListener('click', () => handleDirection(direction));
        }
    });
    
    // 初始化攝像頭移動控制
    if (cameraUpButton) cameraUpButton.addEventListener('click', () => handleCameraMove('up'));
    if (cameraDownButton) cameraDownButton.addEventListener('click', () => handleCameraMove('down'));
    
    // 初始化機械臂書籤控制
    setupMechanicalArmTabs();
    
    // 初始化截圖滾動區域
    createScrollZones();
    
    // 初始化系統資源指示器
    initializeSystemResources();
    
    // 初始化儀表盤
    updateGauges(0, 0);
    
    // 直接調用修復函數確保功能生效
    setTimeout(fixScreenshotScroll, 500);
    
    console.log("頁面初始化完成");
});
// 修復截圖滾動功能的JavaScript程式碼
// 將這些函數添加到你現有的app.js中

// 修復截圖區塊滾動功能
function fixScreenshotScroll() {
    // 獲取截圖容器和列表元素
    const screenshotContainer = document.querySelector('.screenshot-container');
    const screenshotList = document.getElementById('screenshot-list');
    
    if (!screenshotContainer || !screenshotList) {
        console.error('找不到截圖容器或列表元素');
        return;
    }
    
    // 確保截圖列表可以水平滾動
    screenshotList.style.overflowX = 'auto';
    
    // 移除可能存在的舊按鈕
    const oldButtons = screenshotContainer.querySelectorAll('.scroll-btn');
    oldButtons.forEach(btn => btn.remove());
    
    // 添加左右滾動按鈕
    const scrollLeftBtn = document.createElement('button');
    scrollLeftBtn.className = 'scroll-btn scroll-left-btn';
    scrollLeftBtn.innerHTML = '&lt;';
    scrollLeftBtn.title = '向左滾動';
    
    const scrollRightBtn = document.createElement('button');
    scrollRightBtn.className = 'scroll-btn scroll-right-btn';
    scrollRightBtn.innerHTML = '&gt;';
    scrollRightBtn.title = '向右滾動';
    
    // 添加按鈕到容器
    screenshotContainer.appendChild(scrollLeftBtn);
    screenshotContainer.appendChild(scrollRightBtn);
    
    // 設置按鈕點擊事件
    scrollLeftBtn.addEventListener('click', () => {
        screenshotList.scrollBy({ left: -200, behavior: 'smooth' });
    });
    
    scrollRightBtn.addEventListener('click', () => {
        screenshotList.scrollBy({ left: 200, behavior: 'smooth' });
    });
    
    // 添加滾輪事件處理
    screenshotContainer.addEventListener('wheel', (e) => {
        // 如果按下Shift鍵或沒有垂直滾動，直接進行水平滾動
        if (e.shiftKey || Math.abs(e.deltaX) > Math.abs(e.deltaY)) {
            e.preventDefault();
            screenshotList.scrollLeft += e.deltaX || e.deltaY;
        } 
        // 如果有垂直滾動且沒按Shift，轉換為水平滾動
        else if (Math.abs(e.deltaY) > 0) {
            e.preventDefault();
            screenshotList.scrollLeft += e.deltaY;
        }
    }, { passive: false });
    
    // 截圖後自動滾動到最新的截圖
    const originalCaptureScreenshot = window.captureScreenshot || function(){};
    window.captureScreenshot = function() {
        originalCaptureScreenshot.apply(this, arguments);
        setTimeout(() => {
            if (screenshotList.children.length > 0) {
                screenshotList.scrollLeft = screenshotList.scrollWidth;
            }
        }, 100);
    };
    
    console.log('截圖滾動功能已修復');
}

// 在頁面加載完成後自動應用修復
document.addEventListener('DOMContentLoaded', function() {
    // 延遲執行以確保所有元素都已載入
    setTimeout(fixScreenshotScroll, 500);
});

// 在窗口大小變化時重新應用修復
window.addEventListener('resize', function() {
    // 延遲執行以避免頻繁調用
    clearTimeout(window.resizeTimer);
    window.resizeTimer = setTimeout(fixScreenshotScroll, 200);
});