// Check login status
if (!localStorage.getItem('isLoggedIn')) {
    window.location.href = 'login.html';
}

// 設置用戶資訊
const username = localStorage.getItem('username') || 'User';
document.getElementById('userInitial').textContent = username.charAt(0).toUpperCase();
document.getElementById('userName').textContent = username;
document.getElementById('userEmail').textContent = username.toLowerCase() + '@example.com';

// 下拉選單控制
const accountBtn = document.getElementById('accountBtn');
const dropdownMenu = document.getElementById('dropdownMenu');

accountBtn.addEventListener('click', () => {
    dropdownMenu.classList.toggle('show');
});

// 點擊其他地方關閉下拉選單
window.addEventListener('click', (e) => {
    if (!accountBtn.contains(e.target) && !dropdownMenu.contains(e.target)) {
        dropdownMenu.classList.remove('show');
    }
});

// 登出功能
function handleLogout() {
    localStorage.removeItem('isLoggedIn');
    localStorage.removeItem('username');
    window.location.href = 'login.html';
}


const videoElement = document.getElementById('camera-stream');
const startButton = document.getElementById('start-btn');
const stopButton = document.getElementById('stop-btn');
const errorMessage = document.getElementById('error-message');
const directionButtons = {
    up: document.getElementById('up-btn'),
    down: document.getElementById('down-btn'),
    left: document.getElementById('left-btn'),
    right: document.getElementById('right-btn')
};
let mediaStream = null;

// Gauge related variables
const maxSpeed = 100;
const maxDistance = 200;

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
        errorMessage.textContent = '';
    } catch (err) {
        console.error('Camera access error:', err);
        errorMessage.textContent = `Error: ${err.message}`;
    }
}

function stopCamera() {
    if (mediaStream) {
        mediaStream.getTracks().forEach(track => track.stop());
        videoElement.srcObject = null;
        startButton.disabled = false;
        stopButton.disabled = true;
    }
}

function handleDirection(direction) {
    console.log(`Moving: ${direction}`);
    // Add your Raspberry Pi control logic here
}

// Event Listeners
startButton.addEventListener('click', startCamera);
stopButton.addEventListener('click', stopCamera);

directionButtons.up.addEventListener('click', () => handleDirection('up'));
directionButtons.down.addEventListener('click', () => handleDirection('down'));
directionButtons.left.addEventListener('click', () => handleDirection('left'));
directionButtons.right.addEventListener('click', () => handleDirection('right'));

// Check for browser support
if (!navigator.mediaDevices || !navigator.mediaDevices.getUserMedia) {
    errorMessage.textContent = 'Your browser does not support camera access';
    startButton.disabled = true;
}

// Initialize gauges with default values
document.addEventListener('DOMContentLoaded', () => {
    updateGauges(0, 0);
});