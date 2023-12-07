#include <AWS_IOT.h> 
#include <WiFi.h>
#include <Arduino_JSON.h>  // refer JSONObject example for more 
#include "RTClib.h" //DateTime 객체를 사용하기 위한 라이브러리
#include "time.h" // NTP관련 라이브러리
#include <HTTPClient.h> //http 통신을 위한 라이브러리
AWS_IOT testButton;
// wifi 설정
const char* ssid = "CNS401b";
const char* password = "CNS401bs401";
const int seat_num = 1; // 좌석마다 달라지는 값
// aws iot core 설정
char HOST_ADDRESS[]="a2ijsbymx6vfow-ats.iot.ap-northeast-2.amazonaws.com"; // aws 주소 
char client_name[]= "seat-"; // 좌석 번호에 따라 달라지는 좌석 번호 
char client_id[50];
char stopic[] = "smartstudy/buzzer/";
char buzzer_topic[50]; // 부저 울리는 요청을 받을 주소

char detect_publish_topic[]= "$aws/things/smartstudy/shadow/update"; // 적외선, 압력 값 보내기 위한 주소
unsigned long preMil = 0; // 서버에 publish 할 때마다 갱신 
char payload[512]; // publish 할 payload
char rcvdPayload[512]; // subscribe한 주소로부터 받은 payload
int msgReceived = 0; // 구독한 주제로부터 메시지 수신 여부
// 회로 pin 정보 
const int motionSensor = 34; // PIR 센서 input (모션)
const int fsrPin = 32; // fsr 센서 input (압력)
const int buzPin = 23; // 부저 
// pwm 변수
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;
int duty = 128;
// variables 
int sVal;

unsigned long now = millis(); 
bool startTimer = false;
bool flag = false;
// 연주
void playNote(int frequency, int dur, int num) {
  for (int i = 0; i<num; i++){ 
    ledcSetup(ledChannel, frequency, resolution); 
    ledcWrite(ledChannel, duty);
    delay(dur);
    ledcWrite(ledChannel, 0);
    delay(dur);
  }

}
// LED 감지하면 flag true로 설정
// 여기에 복잡한 로직 넣으면 오류 발생 flag로 처리
void IRAM_ATTR detectsMovement() {
  flag = true;
  startTimer = true; 

}

// 부정 검출 관련
const char seatInfoAPI[] = "http://3.38.79.202:8080/seats?api_key=kaupassword!&seat_id=1"; // id를 query parameter로 주면 해당 좌석의 이용 시작시간, 이용 종료시간을 받아올 수 있는 API 주소 1은 보드(좌석) 별로 달라져야함.
const char facedetectAPI[] = "http://3.38.79.202:8080/detect?api_key=kaupassword!";
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600*9;
const int daylightOffset_sec = 0;
const int paymentInterval = 60000; // API 요청 간격(1분)
bool paymentStatus = true; // 결제 상태(true이면 결제된 상태, false이면 안된상태)
unsigned long currentPaymentTime; // 마지막으로 함수가 수행된 시간을 받기 위한 변수
HTTPClient http; //http 통신을 위한 객체
unsigned long preMilServer = 0; // 서버에 publish 할 때마다 갱신 
// char stopicShadow[] = "$aws/things/smartstudy2/shadow/get/accepted"; // shadow 받아올 주소
char ptopicShadow[]= "$aws/things/smartstudy/shadow/get"; // shadow 요청할 주소
char ptopicSensor[]= "$aws/things/smartstudy/shadow/update"; // shadow 요청할 주소
char shadowPayload[512]; // publish 할 payload
char shadowRcvdPayload[512]; // subscribe한 주소로부터 받은 payload
int shadowMsgReceived = 0; // 구독한 주제로부터 메시지 수신 여부
char email[]= "smartstudy/seat"; // 이메일 보내기 위한 주소

// void ShadowSubCallBackHandler (char *topicName, int payloadLen, char *payLoad) 
// {
//   strncpy(shadowRcvdPayload,payLoad,payloadLen); 
//   rcvdPayload[payloadLen] = 0;
//   shadowMsgReceived=1; 
// }

// String 형태로오는 DateTime을 파싱하여 DateTime 객체로 바꾸어주는 함수
DateTime parseDateTime(String dateString) {
  int year = dateString.substring(0, 4).toInt();
  int month = dateString.substring(5, 7).toInt();
  int day = dateString.substring(8, 10).toInt();
  int hour = dateString.substring(11, 13).toInt();
  int minute = dateString.substring(14, 16).toInt();
  int second = dateString.substring(17, 19).toInt();
  return DateTime(year, month, day, hour, minute, second);
}

// 현재 시간을 DateTime 객체로 가져오는 함수
DateTime getCurrentDateTime(){
  struct tm timeInfo;
  DateTime dt;
  if(!getLocalTime(&timeInfo)){
    Serial.println("Failed to obtain time");
    return dt;  
  }
  dt = DateTime(
    timeInfo.tm_year + 1900, 
    timeInfo.tm_mon + 1, 
    timeInfo.tm_mday, 
    timeInfo.tm_hour, 
    timeInfo.tm_min, 
    timeInfo.tm_sec
  );
  return dt;
}


void getPaymentStatus(){
  http.begin(seatInfoAPI); // 지정된 api 주소로 통신 시작
  int httpCode = http.GET(); // get 요청
//  Serial.print("status code in getPaymentStatus() : ");
//  Serial.println(httpCode);
  if (httpCode > 0) { // 응답 코드가 0 이상인 경우(정상적으로 받은 경우)
    // JSON Parsing
    String response = http.getString(); // payload 받아옴
    JSONVar parsedResponse = JSON.parse(response); // JSON객체로 파싱
    String usageEndDateTimeString = parsedResponse["usage_end"]; // 사용 종료 시간인 usage_end 키를 가져옴

    // getCurrentTime and compare
    DateTime currentDateTime = getCurrentDateTime(); // 현재 시간을 DateTime 객체 형태로 가져옴
    DateTime usageEndDateTime = parseDateTime(usageEndDateTimeString); // 사용 종료 시간을 DateTime 객체로 변환
    Serial.println("parsed Current DateTime : " + String(currentDateTime.timestamp()));
    Serial.println("parsed usageEnd Datetime : " + String(usageEndDateTime.timestamp()));
    if(usageEndDateTime < currentDateTime){ // 부등호로 현재 시간과 이용종료 시간 비교하여 결제 현황 업데이트
      paymentStatus = false;
      Serial.println("Seat unpaid");
    } else{
      paymentStatus = true;
      Serial.println("Seat paid");
    }
  } else Serial.print("http request failed error code : " + String(httpCode));
  http.end(); // http 통신 종료
}

void mySubCallBackHandler (char *topicName, int payloadLen, char *payLoad) 
{
  strncpy(rcvdPayload,payLoad,payloadLen); 
  rcvdPayload[payloadLen] = 0; 
  msgReceived = 1;
}
void setup() {
  // client_id, buzzer_topic 생성
  sprintf(client_id, "%s%d", client_name, seat_num);
  sprintf(buzzer_topic, "%s%d", stopic, seat_num);
  Serial.begin(115200);
  pinMode(motionSensor, INPUT_PULLUP); // PIR 센서 input pull up
  pinMode(fsrPin, INPUT); // fsr 센서 input
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(buzPin, ledChannel); // pwm led channel 연결
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, RISING);
  Serial.setDebugOutput(true);

  // wifi, aws 연결
  Serial.print("WIFI status = "); 
  Serial.println(WiFi.getMode()); 
  WiFi.disconnect(true); 
  delay(1000);
  WiFi.mode(WIFI_STA); 
  delay(1000);
  Serial.print("WIFI status = ");
  Serial.println(WiFi.getMode()); //++choi 
  WiFi.begin(ssid, password);
  // 시간 
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer); // NTP 사용을 위한 설정
  currentPaymentTime = millis() - 50000; // 프로그램 첫 실행시 10초 이후에 getPaymentStatus()를 실행시키기 위한 구문 
  while (WiFi.status() != WL_CONNECTED) { 
    delay(1000);
    Serial.println("Connecting to WiFi.."); 
  }
  Serial.println("Connected to wifi");
  if(testButton.connect(HOST_ADDRESS,client_id)== 0) { 
    Serial.println("Connected to AWS");
    delay(1000);
    if(0==testButton.subscribe(buzzer_topic,mySubCallBackHandler)) { 
      Serial.println("buzer Subscribe Successfull");
    } 
    else {
      Serial.println("buzzer Subscribe Failed, Check the Thing Name and Certificates"); 
      while(1);
    }
    // if(0==testButton.subscribe(stopicShadow,ShadowSubCallBackHandler)) { 
    //   Serial.println("Shadow Subscribe Successfull");
    // } 
    // else {
    //   Serial.println("shadow Subscribe Failed, Check the Thing Name and Certificates"); 
    //   while(1);
    // }
  } 

  else {
    Serial.println("AWS connection failed, Check the HOST Address"); 
    while(1);
  }


}
void loop() {
  // motion 감지하고 5초 정도 지나야 함
  if (millis()-preMil > 5000){
    preMil = millis(); // preMil 갱신
  
    int fsrADC = analogRead(fsrPin); // 압력
    Serial.println("모션 감지\n압력 : "+String(fsrADC)); // 메시지 출력
    sprintf(payload,"{\"state\":{\"reported\" : {\"pir\" : %d, \"press\" :%d}}}",!flag, fsrADC);
    Serial.print("payload : "); 
    Serial.println(payload); 

    if(testButton.publish(ptopicSensor,payload) == 0) { 
     Serial.println("압력 정보 전송 완료");
    }
    else{
      Serial.println("입력 정보 전송 실패");
    }
    flag=false;

  }
  if(millis() - preMilServer >= 60000){ // 1분 간격마다 실행하도록하는 구문
    preMilServer = millis(); // 최신 시간 업데이트
    getPaymentStatus(); // 결제상태 받아오는 함수 실행
    if(paymentStatus){
      Serial.println("It is paid seat!");
    } else{
      Serial.println("It is unpaid seat...");
      int fsrADC = analogRead(fsrPin); // 압력
      if (flag&&fsrADC>3000){
        sprintf(payload,"");
        if(testButton.publish(ptopicShadow,payload) == 0) { 
          Serial.println("부정 사용자 의심 단계 요청 보냄");
        }
        else{
          Serial.println("부정 사용자 의심 단계 요청 실패");
        }
      }
    }
  }

  if (msgReceived==1){    
    msgReceived = 0;
    playNote(262, 1000, 3);
    JSONVar response = JSON.parse(rcvdPayload); 
    String message = (const char*) response["message"];
    Serial.println(message);
  }
    // 이 아래 구문이 실행된다는 뜻은 결제가 안되었는데 사람을 감지해서 이를 비교하기 위해 shadow값을 받아왔다는 뜻
  // 여기서 5분 지났다면 이메일 전송

}
