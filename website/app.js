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
    
    // Convert to image
    const img = document.createElement('img');
    img.src = canvas.toDataURL('image/png');
    
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