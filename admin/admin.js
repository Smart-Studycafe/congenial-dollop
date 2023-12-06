var awsIot = require('aws-iot-device-sdk');
const topic = 'esp32/bme280';

const seats = [
  { id: 1, reserved: false },
  { id: 2, reserved: false },
  { id: 3, reserved: false },
  { id: 4, reserved: false }
];

const cameraModal = document.getElementById("camera-modal");
const cameraFrame = document.getElementById("camera-frame");
const modalBuzzerButton = document.getElementById("modal-buzzer-button");
const modalContent = cameraModal.getElementsByClassName("modal-content")[0];

var device = awsIot.device({
  keyPath: '9eb51cfc1d691fd6f04c392a8d0ebff343271d20c690e1e7226e96ad10bc52f6-private.pem.key',
  certPath: '9eb51cfc1d691fd6f04c392a8d0ebff343271d20c690e1e7226e96ad10bc52f6-certificate.pem.crt',
  caPath: 'AmazonRootCA1.pem',
  clientId: 'smartstudycafe',
  host: 'a2ab3bby7g4h30-ats.iot.ap-northeast-2.amazonaws.com'
});

device.on('connect', function () {
  console.log('AWS IoT에 연결되었습니다.');
  device.subscribe(topic);
});

device.on('message', function (topic, payload) {
  console.log('메시지 수신', payload.toString());
  const data = JSON.parse(payload.toString());

  if ('image' in data) {
    const imageData = data.image;
    console.log('이미지 데이터:', imageData);
    cameraFrame.src = "data:image/jpeg;base64," + imageData;
  }
});

device.on('error', function (error) {
  console.log('에러 발생:', error);
});

function showModal() {
  cameraModal.style.display = "block";
  modalContent.style.visibility = "visible";
  modalContent.classList.remove("modal-hide");
  modalContent.classList.add("modal-show");
}

function hideModal() {
  modalContent.classList.remove("modal-show");
  modalContent.classList.add("modal-hide");
  modalContent.addEventListener('animationend', function () {
    modalContent.style.visibility = "hidden";
    cameraModal.style.display = "none";
    modalContent.classList.remove("modal-hide");
    modalContent.classList.remove("modal-show");
  }, { once: true });
}

seats.forEach(function (seat) {
  const seatDiv = document.getElementById('seat-' + seat.id);
  seatDiv.className = 'seat' + (seat.reserved ? ' reserved' : '');
  seatDiv.onclick = function () {
    console.log('좌석 클릭!');
    cameraModal.style.visibility = "block";

    modalBuzzerButton.onclick = function () {
      const message = JSON.stringify({
        api_key: 'kaupassword!',
        device_id: seat.id
      });
      // POST 요청 보내기
      fetch('http://3.38.79.202:8080/buzzer?api_key=kaupassword!&device_id=' + seat.id, {
          method: 'POST',
          headers: {
              'Content-Type': 'application/json'
          },
          body: message
      })
      .then(response => response.json())
      .then(response => {
          if (response.success) {
              console.log('Buzzer 요청이 성공했습니다.');
          } else {
              console.error('Buzzer 요청이 실패했습니다.', response.message);
          }
      })
      .catch(error => {
          console.error('Buzzer 요청 중 오류 발생:', error);
      });
      device.publish(topic, message);
    };

    showModal();
  };
});

window.onclick = function (event) {
  if (event.target == cameraModal) {
    hideModal();
  }
};

const cameraModalClose = document.getElementsByClassName("close")[0];
cameraModalClose.onclick = function () {
  hideModal();
};
