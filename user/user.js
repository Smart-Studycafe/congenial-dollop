var seatsDiv = document.getElementById('seats');
var modal = document.getElementById("reservation-modal");
var span = document.getElementsByClassName("close")[0];
var reserveButton = document.getElementById("reserve-button");
var customerNameInput = document.getElementById("customer-name");
var reservationTimeInput = document.getElementById("reservation-time");
var reservationEndTime = document.getElementById("modal-reservation-end-time");
var modalHeaderText = document.getElementById("modal-header-text");
var modalContent = modal.getElementsByClassName("modal-content")[0];
var cancelReservationButton = document.getElementById("cancel-reservation-button");
var currentSeat = null;

setTimeout(function(){
    location.reload();
}, 10000);

setInterval(function() {
    var now = new Date();
    var currentTime = now.getHours() + ":" + now.getMinutes() + ":" + now.getSeconds();
    document.getElementById('current-time').textContent = "현재 시간: " + currentTime;
}, 1000);

function showModal() {
    modal.style.display = "block";
    modalContent.style.visibility = "visible";
    modalContent.classList.remove("modal-hide");
    modalContent.classList.add("modal-show");
}
  
function hideModal() {
    modalContent.classList.remove("modal-show");
    modalContent.classList.add("modal-hide");
    modalContent.addEventListener('animationend', function () {
        modal.style.display = "none";
        modalContent.classList.remove("modal-hide");
        modalContent.classList.remove("modal-show");
    }, { once: true });
}

// 시간 형식 변환
function formatDatetimeToCustomFormat(isoString) {
    const date = new Date(isoString);
    
    const year = date.getFullYear();
    const month = ('0' + (date.getMonth() + 1)).slice(-2);
    const day = ('0' + date.getDate()).slice(-2);
    const hours = ('0' + date.getHours()).slice(-2);
    const minutes = ('0' + date.getMinutes()).slice(-2);
    const seconds = ('0' + date.getSeconds()).slice(-2);

    return `${year}-${month}-${day} ${hours}:${minutes}:${seconds}`;
}

// 서버에서 좌석 정보를 불러오는 함수
function loadSeatInfo() {
    fetch("http://3.38.79.202:8080/seats?api_key=kaupassword!", {
        method: 'GET'
    })
    .then(response => {
        if (!response.ok) {
            throw new Error(`HTTP error! Status: ${response.status}`);
        }
        return response.json();
    })
    .then(seats => {
        console.log('Received seats from server:', seats);
        var now = new Date();
        // 각 좌석에 대해 반복
        seats.forEach(function (seat) {
            var seatDiv = document.getElementById('seat-' + seat.seat_id);
            var usageEndTime = new Date(seat.usage_end);
            // 좌석을 이용 중인지 확인
            if (seat.user_id) {
                if (usageEndTime < now) {
                    seatDiv.className = 'seat';
                } else {
                    seatDiv.className = 'seat reserved';
                }
            } else {
                seatDiv.className = 'seat';
            }
            // 좌석 클릭
            seatDiv.onclick = function () {
                console.log('Seat clicked:', seat);
                showModal();
                
                // 현재 선택된 좌석 저장
                currentSeat = seat;
                var usageEndTime = new Date(seat.usage_end);
                var now = new Date();

                // 좌석을 이용하고 있고 사용 종료 시간이 지나지 않았을 때
                if (seat.user_id && usageEndTime >= now) {
                    modalHeaderText.innerText = '🔄 예약을 변경하려면 새로운 시간을 입력해주세요';
                    reserveButton.innerText = '예약 변경하기';
                    customerNameInput.style.display = "none";
                    reservationEndTime.textContent = "예약 종료 시간: " + formatDatetimeToCustomFormat(seat.usage_end);
                    cancelReservationButton.style.display = "block";
                } else {
                    modalHeaderText.innerText = '🤔 이름을 입력해주세요';
                    reserveButton.innerText = '예약하기';
                    customerNameInput.style.display = "block";
                    cancelReservationButton.style.display = "none";
                }
            };
        });
    })
    .catch(error => {
        console.error('Error fetching seats:', error);
    });
}

// DOM이 로드되면 좌석 정보를 불러온다.
document.addEventListener('DOMContentLoaded', function () {
    console.log("DOMContentLoaded");
    loadSeatInfo();
});

// 예약
reserveButton.onclick = function () {
    console.log("Reserve button clicked");
    var name = customerNameInput.value;
    var time = parseFloat(reservationTimeInput.value);

    console.log(`Name: ${name}, Time: ${time}`);

    if (typeof name === 'string' && time && !isNaN(time)) {
        var url = currentSeat.user_id ? "http://3.38.79.202:8080/seats?api_key=kaupassword!" : "http://3.38.79.202:8080/seats?api_key=kaupassword!";
        var method = currentSeat.user_id ? "PUT" : "POST";

        var now = new Date();
        var usageStart = formatDatetimeToCustomFormat(now.toISOString());

        var usageEnd = new Date(now.getTime() + time * 60 * 60 * 1000).toISOString();
        usageEnd = formatDatetimeToCustomFormat(usageEnd);

        console.log(`URL: ${url}, Method: ${method}, Usage start: ${usageStart}, Usage end: ${usageEnd}`);

        // 서버에 예약 요청
        fetch(url, {
            method: method,
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({
                seat_id: currentSeat.seat_id,
                user_id: 1,
                usage_start: usageStart,
                usage_end: usageEnd,
            }),
        })
        .then(response => response.json())
        .then(response => {
            console.log('Response from server:', response);
            loadSeatInfo();
        })
        .catch(error => {
            console.error('Error:', error);
        });

        hideModal();
    }
};

// 예약 취소
cancelReservationButton.onclick = function () {
    console.log("Cancel reservation button clicked");
    var now = new Date();
    var usageEnd = formatDatetimeToCustomFormat(now.toISOString());

    // 서버에 예약 취소 요청
    fetch("http://3.38.79.202:8080/seats?api_key=kaupassword!", {
        method: "PUT",
        headers: {
            'Content-Type': 'application/json',
        },
        body: JSON.stringify({
            seat_id: currentSeat.seat_id,
            user_id: 1,
            usage_start: formatDatetimeToCustomFormat(currentSeat.usage_start),
            usage_end: usageEnd,
        }),
    })
    .then(response => response.json())
    .then(response => {
        console.log('Response from server:', response);
        loadSeatInfo();
    })
    .catch(error => {
        console.error('Error:', error);
    });
    document.getElementById('modal-reservation-end-time').textContent = '';
    hideModal();
};

// 모달 창의 X 버튼 클릭
span.onclick = function() {
    hideModal();
};

// 모달 창의 배경 클릭
window.onclick = function (event) {
    if (event.target == modal) {
        console.log("Modal background clicked");
        hideModal();
    }
};
