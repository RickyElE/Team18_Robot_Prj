const socket = new WebSocket("ws://192.168.137.202:8080");

socket.binaryType = "arraybuffer"; // 非常关键，确保能接收 binary 数据

socket.onmessage = function(event) {
    const blob = new Blob([event.data], { type: 'image/jpeg' });
    const imgUrl = URL.createObjectURL(blob);
    document.getElementById("preview").src = imgUrl;
};