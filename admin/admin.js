const seats = [
  { id: 1, reserved: false },
  { id: 2, reserved: false },
  { id: 3, reserved: false },
  { id: 4, reserved: false }
];

const cameraModal = document.getElementById("camera-modal");
const modalBuzzerButton = document.getElementById("modal-buzzer-button");
const modalContent = cameraModal.getElementsByClassName("modal-content")[0];

setInterval(function() {
  var now = new Date();
  var currentTime = now.getHours() + ":" + now.getMinutes() + ":" + now.getSeconds();
  document.getElementById('current-time').textContent = "현재 시간: " + currentTime;
}, 1000);

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

    // 부저 버튼 클릭
    modalBuzzerButton.onclick = function () {
      const message = JSON.stringify({
        api_key: 'kaupassword!',
        device_id: seat.id
      });
      
      // POST 요청
      fetch('http://3.38.79.202:8080/buzzer?api_key=kaupassword!', {
          method: 'POST',
          headers: {
              'Content-Type': 'application/json'
          },
          body: message
      })
      .then(response => response.json())
      .then(response => {
          console.log(response);
      })
      .catch(error => {
          console.error('Buzzer 요청 중 오류 발생:', error);
      });
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
