var seatsDiv = document.getElementById('seats');
var modal = document.getElementById("reservation-modal");
var span = document.getElementsByClassName("close")[0];
var reserveButton = document.getElementById("reserve-button");
var customerNameInput = document.getElementById("customer-name");
var reservationTimeInput = document.getElementById("reservation-time");
var modalContent = modal.getElementsByClassName("modal-content")[0];
var currentSeat = null;

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

document.addEventListener('DOMContentLoaded', function () {
    console.log("DOMContentLoaded");
    // GET
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
        seats.forEach(function (seat) {
            var seatDiv = document.getElementById('seat-' + seat.seat_id);
            if (seat.user_id) {
                seatDiv.className = 'seat reserved';
            } else {
                seatDiv.className = 'seat';
            }
            seatDiv.onclick = function () {
                console.log('Seat clicked:', seat);
                showModal();
                currentSeat = seat;
                var modalHeaderText = document.getElementById("modal-header-text");
                var reserveButton = document.getElementById("reserve-button");
                if (seat.user_id) {
                    modalHeaderText.innerText = '🔄 예약을 변경하려면 새로운 시간을 입력해주세요';
                    reserveButton.innerText = '예약 변경하기';
                    document.getElementById("customer-name").style.visibility = "visible";
                } else {
                    modalHeaderText.innerText = '🤔 이름을 입력해주세요';
                    reserveButton.innerText = '예약하기';
                }
            };
        });
    })
    .catch(error => {
        console.error('Error fetching seats:', error);
    });
});

reserveButton.onclick = function () {
    console.log("Reserve button clicked");
    var name = customerNameInput.value;
    var time = parseInt(reservationTimeInput.value);

    console.log(`Name: ${name}, Time: ${time}`);

    if (typeof name === 'string' && time && !isNaN(time)) {
        var url = currentSeat.user_id ? "http://3.38.79.202:8080/seats?api_key=kaupassword!" : "http://3.38.79.202:8080/seats?api_key=kaupassword!";
        var method = currentSeat.user_id ? "PUT" : "POST";

        var now = new Date();
        var usageStart = formatDatetimeToCustomFormat(now.toISOString());

        var usageEnd = new Date(now.getTime() + time * 60 * 60 * 1000).toISOString();
        usageEnd = formatDatetimeToCustomFormat(usageEnd);

        console.log(`URL: ${url}, Method: ${method}, Usage start: ${usageStart}, Usage end: ${usageEnd}`);

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
        })
        .catch(error => {
            console.error('Error:', error);
        });

        hideModal();
    }
};

span.onclick = function() {
    hideModal();
};

window.onclick = function (event) {
    if (event.target == modal) {
        console.log("Modal background clicked");
        hideModal();
    }
};

