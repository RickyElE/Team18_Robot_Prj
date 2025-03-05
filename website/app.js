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

// 獲取元素
const streamImage = document.getElementById('camera-stream');
const startButton = document.getElementById('start-btn');
const stopButton = document.getElementById('stop-btn');
const errorMessage = document.getElementById('error-message');
const directionButtons = {
    up: document.getElementById('up-btn'),
    down: document.getElementById('down-btn'),
    left: document.getElementById('left-btn'),
    right: document.getElementById('right-btn')
};

// 修改按鈕功能為刷新和停止串流
startButton.textContent = "刷新串流";
startButton.disabled = false;
stopButton.textContent = "隱藏串流";
stopButton.disabled = false;

// Gauge related variables
const maxSpeed = 100;
const maxDistance = 200;

function updateGauges(speed, distance) {
    // Update speed gauge
    const speedPointer = document.getElementById('speed-pointer');
    const speedText = document.getElementById('speed-text');
    const speedAngle = (speed / maxSpeed) * 180 - 90;
    speedPointer.setAttribute('transform', `rotate(${speedAngle} 60 60)`);
    speedText.textContent = `${speed} km/h`;

    // Update distance gauge
    const distancePointer = document.getElementById('distance-pointer');
    const distanceText = document.getElementById('distance-text');
    const distanceAngle = (distance / maxDistance) * 180 - 90;
    distancePointer.setAttribute('transform', `rotate(${distanceAngle} 60 60)`);
    distanceText.textContent = `${distance} cm`;
}

// 刷新串流功能 - 添加時間戳參數避免瀏覽器快取
function refreshStream() {
    streamImage.src = `/video_feed?t=${new Date().getTime()}`;
    errorMessage.textContent = '';
}

// 隱藏/顯示串流功能
function toggleStream() {
    if (streamImage.style.display === 'none') {
        streamImage.style.display = 'block';
        stopButton.textContent = "隱藏串流";
    } else {
        streamImage.style.display = 'none';
        stopButton.textContent = "顯示串流";
    }
}

// 處理方向控制
function handleDirection(direction) {
    console.log(`移動方向: ${direction}`);
    // 向伺服器發送命令
    fetch(`/control?dir=${direction}`, { method: 'POST' })
        .then(response => {
            if (!response.ok) {
                throw new Error('控制命令發送失敗');
            }
            return response.json();
        })
        .then(data => {
            console.log('控制命令響應:', data);
        })
        .catch(error => {
            console.error('控制錯誤:', error);
            errorMessage.textContent = `錯誤: ${error.message}`;
        });
}

// 事件監聽器
startButton.addEventListener('click', refreshStream);
stopButton.addEventListener('click', toggleStream);

directionButtons.up.addEventListener('click', () => handleDirection('forward'));
directionButtons.down.addEventListener('click', () => handleDirection('backward'));
directionButtons.left.addEventListener('click', () => handleDirection('left'));
directionButtons.right.addEventListener('click', () => handleDirection('right'));

// 模擬數據更新 (實際應用中可從伺服器獲取)
function simulateDataUpdates() {
    setInterval(() => {
        const speed = Math.floor(Math.random() * 50);
        const distance = Math.floor(Math.random() * 100) + 50;
        updateGauges(speed, distance);
    }, 2000);
}

// 頁面載入時初始化
document.addEventListener('DOMContentLoaded', () => {
    // 初始更新儀表盤
    updateGauges(0, 0);
    
    // 確保串流顯示正確
    refreshStream();
    
    // 啟動模擬數據
    simulateDataUpdates();
});

// 處理串流加載錯誤
streamImage.onerror = function() {
    errorMessage.textContent = "無法加載攝像頭串流，請確保伺服器運行中";
};

// 處理串流加載成功
streamImage.onload = function() {
    errorMessage.textContent = "";
};