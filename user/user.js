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
    // GET
    fetch("127.0.0.1:5500/seats?api_key=kaupassword!", {
        method: 'GET',
        mode: 'cors',
    })
    .then(response => {
        if (!response.ok) {
            throw new Error(`HTTP error! Status: ${response.status}`);
        }
        return response.json();
    })
    .then(seats => {
        seats.forEach(function (seat) {
            var seatDiv = document.getElementById('seat-' + seat.seat_id);
            if (seat.user_id) {
                seatDiv.className = 'seat reserved';
            } else {
                seatDiv.className = 'seat';
            }
            seatDiv.onclick = function () {
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
    var name = customerNameInput.value;
    var time = parseInt(reservationTimeInput.value);

    if (typeof name === 'string' && time && !isNaN(time)) {
        var url = currentSeat.user_id ? "127.0.0.1:5500/seats?api_key=kaupassword!" + currentSeat.seat_id + "?api_key=kaupassword!" : "127.0.0.1:5500/seats?api_key=kaupassword!";
        var method = currentSeat.user_id ? "PUT" : "POST";

        var now = new Date();
        var usageStart = formatDatetimeToCustomFormat(now.toISOString());

        var usageEnd = new Date(now.getTime() + time * 60 * 60 * 1000).toISOString();
        usageEnd = formatDatetimeToCustomFormat(usageEnd);

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
            if (response.success) {
                currentSeat.user_id = 1;
                var seatDiv = document.getElementById('seat-' + currentSeat.seat_id);
                if (currentSeat.user_id) {
                    seatDiv.className = 'seat reserved'; 
                } else {
                    seatDiv.className = 'seat';
                }
            } else {
                console.error(response.message);
            }
        })
        .catch(error => {
            console.error('Error fetching seats:', error);
        });

        hideModal();
    }
};

window.onclick = function (event) {
    if (event.target == modal) {
        hideModal();
    }
};
