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

// 初始化方向按鈕引用
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

// 獲取樹莓派的IP地址 - 這裡應該設為您樹莓派的實際IP地址
const raspberryPiIP = window.location.hostname; // 自動獲取當前伺服器IP
const raspberryPiPort = window.location.port || "8081"; // 使用當前端口或默認8081

// Gauge related variables
const maxSpeed = 100;
const maxDistance = 200;

function updateGauges(speed, distance) {
    // Update speed gauge
    const speedPointer = document.getElementById('speed-pointer');
    const speedText = document.getElementById('speed-text');
    if (speedPointer && speedText) {
        const speedAngle = (speed / maxSpeed) * 180 - 90;
        speedPointer.setAttribute('transform', `rotate(${speedAngle} 60 60)`);
        speedText.textContent = `${speed} km/h`;
    }

    // Update distance gauge
    const distancePointer = document.getElementById('distance-pointer');
    const distanceText = document.getElementById('distance-text');
    if (distancePointer && distanceText) {
        const distanceAngle = (distance / maxDistance) * 180 - 90;
        distancePointer.setAttribute('transform', `rotate(${distanceAngle} 60 60)`);
        distanceText.textContent = `${distance} cm`;
    }
}

// 刷新串流功能 - 使用相對路徑並定期更新圖像
function refreshStream() {
    // 停止任何現有的更新計時器
    if (window.streamTimer) {
        clearTimeout(window.streamTimer);
    }
    
    // 定義更新函數
    function updateImage() {
        streamImage.src = `http://${window.location.hostname}:8081/video_feed?t=${new Date().getTime()}`;
        // 只有在圖像顯示時繼續更新
        if (!streamImage.style.display || streamImage.style.display !== 'none') {
            window.streamTimer = setTimeout(updateImage, 200); // 每200毫秒更新一次
        }
    }
    
    // 開始更新
    updateImage();
    errorMessage.textContent = '';
    console.log('開始定期更新視頻流');
}

// 隱藏/顯示串流功能
function toggleStream() {
    if (streamImage.style.display === 'none') {
        streamImage.style.display = 'block';
        stopButton.textContent = "隱藏串流";
        refreshStream(); // 顯示時重新開始更新流
    } else {
        streamImage.style.display = 'none';
        stopButton.textContent = "顯示串流";
        // 隱藏時停止更新
        if (window.streamTimer) {
            clearTimeout(window.streamTimer);
        }
    }
}

// 處理方向控制
function handleDirection(direction) {
    console.log(`移動方向: ${direction}`);
    // 向伺服器發送命令，使用完整URL
    fetch(`http://${raspberryPiIP}:${raspberryPiPort}/control?dir=${direction}`, { 
        method: 'POST',
        mode: 'no-cors' // 添加 no-cors 模式避免CORS錯誤
    })
    .then(response => {
        if (response.type === 'opaque') {
            // no-cors模式下無法讀取響應內容，但表示請求已發送
            console.log('控制命令已發送');
            return;
        }
        if (!response.ok) {
            throw new Error('控制命令發送失敗');
        }
        return response.json();
    })
    .then(data => {
        if (data) {
            console.log('控制命令響應:', data);
        }
    })
    .catch(error => {
        console.error('控制錯誤:', error);
        errorMessage.textContent = `錯誤: ${error.message}`;
    });
}

// 事件監聽器
startButton.addEventListener('click', refreshStream);
stopButton.addEventListener('click', toggleStream);

// 檢查並初始化方向按鈕
if (directionButtons.up) directionButtons.up.addEventListener('click', () => handleDirection('forward'));
if (directionButtons.down) directionButtons.down.addEventListener('click', () => handleDirection('backward'));
if (directionButtons.left) directionButtons.left.addEventListener('click', () => handleDirection('left'));
if (directionButtons.right) directionButtons.right.addEventListener('click', () => handleDirection('right'));

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
    console.log('頁面已載入，初始化應用程序...');
    
    // 初始更新儀表盤
    updateGauges(0, 0);
    
    // 確保串流顯示正確
    refreshStream();
    
    // 啟動模擬數據
    simulateDataUpdates();
});

// 處理串流加載錯誤
streamImage.onerror = function() {
    console.error('串流加載失敗');
    errorMessage.textContent = "無法加載攝像頭串流，請確保伺服器運行中";
    // 5秒後自動重試
    setTimeout(refreshStream, 5000);
};

// 處理串流加載成功
streamImage.onload = function() {
    console.log('串流加載成功');
    errorMessage.textContent = "";
};