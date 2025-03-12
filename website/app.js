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
    up: document.getElementById('up-btn'),
    down: document.getElementById('down-btn'),
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
    
    if (direction === 'up') {
        currentSpeed = Math.min(maxSpeed, currentSpeed + 10);
    } else if (direction === 'down') {
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
    // Convert to image
    const img = document.createElement('img');
    img.src = imgDataURL;
    // 建立下載連結，設定 download 屬性以指定檔名
    const downloadLink = document.createElement('a');
    downloadLink.href = imgDataURL;
    downloadLink.download = `screenshot_${Date.now()}.png`;  // 可依需要修改檔名
    downloadLink.textContent = '下載圖片';
    downloadLink.style.marginTop = '10px';
    
    // Add timestamp
    const timestamp = document.createElement('div');
    timestamp.textContent = new Date().toLocaleTimeString();
    timestamp.style.fontSize = '12px';
    timestamp.style.color = 'white';
    timestamp.style.textAlign = 'center';
    
    // Create container for the image and timestamp
    const container = document.createElement('div');
    container.style.display = 'flex';
    container.style.flexDirection = 'column';
    container.style.margin = '5px';
    
    container.appendChild(img);
    container.appendChild(timestamp);
    
    // Add to screenshot list
    screenshotList.appendChild(container);
}

// Event Listeners
startButton.addEventListener('click', startCamera);
stopButton.addEventListener('click', stopCamera);
captureButton.addEventListener('click', captureScreenshot);

// Direction button event listeners
directionButtons.up.addEventListener('click', () => handleDirection('up'));
directionButtons.down.addEventListener('click', () => handleDirection('down'));
directionButtons.left.addEventListener('click', () => handleDirection('left'));
directionButtons.right.addEventListener('click', () => handleDirection('right'));

// Camera control event listeners
cameraUpButton.addEventListener('click', () => handleCameraMove('up'));
cameraDownButton.addEventListener('click', () => handleCameraMove('down'));

// Check for browser support
if (!navigator.mediaDevices || !navigator.mediaDevices.getUserMedia) {
    errorMessage.textContent = 'Your browser does not support camera access';
    startButton.disabled = true;
}

// Initialize gauges with default values
document.addEventListener('DOMContentLoaded', () => {
    updateGauges(0, 0);
});

// 獲取 DOM 元素
const batteryLevel = document.getElementById("battery-level");
const batteryPercentage = document.getElementById("battery-percentage");

// 模擬獲取電池電量的數據（可替換為實際從 Raspberry Pi 讀取
let scrollInterval = null;
let scrollSpeed = 3; // pixels per scroll interval
let isMouseInContainer = false;
let mouseX = 0;
let containerWidth = 0;

function createScrollZones() {
    const container = document.querySelector('.screenshot-container');
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
}

function scrollWithSpeed(direction, speed) {
    const screenshotList = document.getElementById('screenshot-list');
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

function captureScreenshot() {
    if (!videoElement.srcObject) return;
    
    const canvas = document.createElement('canvas');
    const ctx = canvas.getContext('2d');
    
    canvas.width = videoElement.videoWidth;
    canvas.height = videoElement.videoHeight;
    ctx.drawImage(videoElement, 0, 0, canvas.width, canvas.height);
    
    const imgDataURL = canvas.toDataURL('image/png');
    
    const img = document.createElement('img');
    img.src = imgDataURL;
    
    img.setAttribute('alt', 'Screenshot ' + new Date().toLocaleTimeString());
    
    const now = new Date();
    const timeString = now.toLocaleTimeString();
    
    const timestamp = document.createElement('div');
    timestamp.textContent = timeString;
    timestamp.className = 'screenshot-timestamp';
    
    const container = document.createElement('div');
    container.className = 'screenshot-item';
    
    container.appendChild(img);
    container.appendChild(timestamp);
    
    screenshotList.appendChild(container);
    
    // 保持滾動位置不變
}

document.addEventListener('DOMContentLoaded', () => {
    createScrollZones();
    
    const screenshotList = document.getElementById('screenshot-list');
    screenshotList.style.overflowX = 'auto'; // 確保滾動條顯示
    screenshotList.style.scrollbarWidth = 'auto'; // 適用於 Firefox
    
    // 修正 wheel 事件，讓 shift + 滾輪才會水平滾動
    screenshotList.addEventListener('wheel', (event) => {
        if (event.shiftKey) {
            event.preventDefault();
            screenshotList.scrollLeft += event.deltaY;
        }
    });

    // 滑鼠拖曳滾動
    screenshotList.addEventListener('mousedown', (event) => {
        let isDragging = false;
        let startX = event.pageX - screenshotList.offsetLeft;
        let scrollLeft = screenshotList.scrollLeft;

        function mouseMoveHandler(e) {
            isDragging = true;
            const x = e.pageX - screenshotList.offsetLeft;
            const walk = (x - startX) * 2; // 調整拖動靈敏度
            screenshotList.scrollLeft = scrollLeft - walk;
        }

        function mouseUpHandler() {
            if (!isDragging) return;
            isDragging = false;
            document.removeEventListener('mousemove', mouseMoveHandler);
            document.removeEventListener('mouseup', mouseUpHandler);
        }

        document.addEventListener('mousemove', mouseMoveHandler);
        document.addEventListener('mouseup', mouseUpHandler);
    });

    // 觸控裝置支援 (滑動手勢)
    let touchStartX = 0;
    let scrollLeftStart = 0;

    screenshotList.addEventListener('touchstart', (event) => {
        touchStartX = event.touches[0].pageX;
        scrollLeftStart = screenshotList.scrollLeft;
    });

    screenshotList.addEventListener('touchmove', (event) => {
        const touchX = event.touches[0].pageX;
        const moveX = touchX - touchStartX;
        screenshotList.scrollLeft = scrollLeftStart - moveX;
    });
});

document.addEventListener('DOMContentLoaded', () => {
    const captureButton = document.getElementById('capture-btn');
    if (captureButton) {
        const newCaptureButton = captureButton.cloneNode(true);
        captureButton.parentNode.replaceChild(newCaptureButton, captureButton);
        newCaptureButton.addEventListener('click', captureScreenshot);
    }
});

// Function to update system resources progress bars
function updateSystemResources() {
    // Simulated data - replace with actual Raspberry Pi readings
    const cpuUsage = Math.floor(Math.random() * 80) + 10; // 10-90%
    const cpuTemp = Math.floor(Math.random() * 60) + 30; // 30-90°C
    const gpuUsage = Math.floor(Math.random() * 70) + 10; // 10-80%
    const gpuTemp = Math.floor(Math.random() * 55) + 35; // 35-90°C
    const ramUsage = Math.floor(Math.random() * 70) + 20; // 20-90%
    const swapUsage = Math.floor(Math.random() * 30); // 0-30%

    // Update progress bars and create/update percentage labels
    const resourceMapping = [
        { id: 'cpu-usage', value: cpuUsage, label: 'CPU Usage' },
        { id: 'cpu-temperature', value: cpuTemp, label: 'CPU Temp' },
        { id: 'gpu-usage', value: gpuUsage, label: 'GPU Usage' },
        { id: 'gpu-temperature', value: gpuTemp, label: 'GPU Temp' },
        { id: 'ram-usage', value: ramUsage, label: 'RAM Usage' },
        { id: 'swap-usage', value: swapUsage, label: 'Swap Usage' }
    ];

    resourceMapping.forEach(resource => {
        const progressBar = document.getElementById(resource.id);
        const progressContainer = progressBar.closest('.progress-container');
        
        // Update progress bar value
        progressBar.value = resource.value;

        // Find or create percentage label
        let percentageLabel = progressContainer.querySelector('.percentage-label');
        if (!percentageLabel) {
            percentageLabel = document.createElement('span');
            percentageLabel.classList.add('percentage-label');
            progressContainer.appendChild(percentageLabel);
        }
        
        // Update percentage label
        percentageLabel.textContent = `${resource.value}%`;
        percentageLabel.style.marginLeft = '10px';
        percentageLabel.style.color = 'white';
        percentageLabel.style.fontSize = '14px';

        // Color coding for critical values
        if (resource.value > 80) {
            progressBar.style.setProperty('--progress-color', '#FF3D00'); // Red for critical
            percentageLabel.style.color = '#FF3D00';
        } else if (resource.value > 60) {
            progressBar.style.setProperty('--progress-color', '#FF9800'); // Orange for warning
            percentageLabel.style.color = '#FF9800';
        } else {
            progressBar.style.setProperty('--progress-color', '#4CAF50'); // Green for normal
            percentageLabel.style.color = '#4CAF50';
        }
    });
}

// Update system resources every 3 seconds
setInterval(updateSystemResources, 3000);

// Initial update
document.addEventListener('DOMContentLoaded', updateSystemResources);