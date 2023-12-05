#include <WiFi.h>
#include <Arduino_JSON.h>
#include "esp_camera.h"
#include "base64.h"
#include "RTClib.h"
#include "time.h"
#include <HTTPClient.h>
#include <AWS_IOT.h>

#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#include "camera_pins.h"

const char seatInfoAPI[] = "";
const char facedetectAPI[] = "";
char hostAddress[] = "";
const char* ssid = "";
const char* password = "";
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600*9;
const int daylightOffset_sec = 0;
const int paymentInterval = 60000;
const int facedetectInterval = 10000;
const int publishInterval = 10000;

bool paymentStatus = true;
int facedetectExecutionCount; // number of facedetect function execution
int facedetectCount; // number of times a face detected
unsigned long currentFacedetectTime;
unsigned long currentPaymentTime;
unsigned long currentPublishTime;
unsigned long currentSubScenarioTime;

char clientId[] = "smartstudy";
char test[] = "$aws/things/smartstudy/shadow/get/accepted";
char streamingPubTopicName[] = "smartstudy/stream/1";
char alarmsendPubTopicName[] = "smartstudy/seat1";
char getShadowPubTopicName[] = "$aws/things/smartstudy/shadow/get";

HTTPClient http;
AWS_IOT testButton;
TaskHandle_t task1;
camera_config_t config;

void initCamera();
void initAWS();

void captureQuick(){
  camera_fb_t *fb = NULL;
  fb = esp_camera_fb_get();
  if(!fb) return;
  esp_camera_fb_return(fb);
}

DateTime parseDateTime(String dateString) {
  int year = dateString.substring(0, 4).toInt();
  int month = dateString.substring(5, 7).toInt();
  int day = dateString.substring(8, 10).toInt();
  int hour = dateString.substring(11, 13).toInt();
  int minute = dateString.substring(14, 16).toInt();
  int second = dateString.substring(17, 19).toInt();
  return DateTime(year, month, day, hour, minute, second);
}

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
  http.begin(seatInfoAPI);
  int httpCode = http.GET();
//  Serial.print("status code in getPaymentStatus() : ");
//  Serial.println(httpCode);
  if (httpCode > 0) {
    // JSON Parsing
    String response = http.getString();
    JSONVar parsedResponse = JSON.parse(response);
    String usageEndDateTimeString = parsedResponse["usage_end"];

    // getCurrentTime and compare
    DateTime currentDateTime = getCurrentDateTime();
    DateTime usageEndDateTime = parseDateTime(usageEndDateTimeString);
//    Serial.println("parsed Current DateTime : " + String(currentDateTime.timestamp()));
//    Serial.println("parsed usageEnd Datetime : " + String(usageEndDateTime.timestamp()));
    if(usageEndDateTime < currentDateTime){
      paymentStatus = false;
      Serial.println("Seat unpaid");
    }
  } else Serial.print("http request failed error code : " + String(httpCode));
  http.end();
}

// request facedetect info
bool getFacedetect(){
  bool result = false;
  captureQuick();
  camera_fb_t *fb = NULL;
//  config.jpeg_quality = 10;
  
  // capture
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return false;
  }
  // request
  http.begin(facedetectAPI);
  http.addHeader("Content-Type", "application/json");
  String encodedImageData = base64::encode(fb->buf, fb->len);
  JSONVar jsonRequestBody;
  jsonRequestBody["image"] = encodedImageData;
  String requestBody = JSON.stringify(jsonRequestBody); 
//  Serial.println("Sended data : " + requestBody);
  int httpCode = http.POST(requestBody);
//  Serial.print("status code in getFacedetect() : ");
//  Serial.println(httpCode);

  // response
  if (httpCode > 0) {
    String response = http.getString();
//    Serial.println("payload : " + response);    
    JSONVar parsedResponse = JSON.parse(response);
    if (parsedResponse.hasOwnProperty("face_detected")) {
//      Serial.print("face_detected = ");
//      Serial.println((bool) parsedResponse["face_detected"]);
      result = parsedResponse["face_detected"];
    } else Serial.println("Invalid JSON");
  } else Serial.println("http request failed error code : " + String(httpCode));
  if(result){
    Serial.println("face detected!");
  } else Serial.println("face not detected");
  // free resources
//  config.jpeg_quality = 16;
  http.end();
  esp_camera_fb_return(fb);
  return result;
}

// takePicture and publish to mqtt broker
void takePictureAndPublish(){
  camera_fb_t *fb = esp_camera_fb_get();
  if(!fb) {
     Serial.println("Camera capture failed in takePictureAndPublish()");
     return;
  }
  const char *data = (const char *)fb->buf;
  String encodedImageData = base64::encode(fb->buf, fb->len);
  // formating json
  JSONVar myObject;
  myObject["image"] = encodedImageData;
  String jsonString = JSON.stringify(myObject);
  char* payload = (char *)malloc(jsonString.length() + 1);
  strcpy(payload, jsonString.c_str());
  
  // publishing topic 
  int codeNum = testButton.publish(streamingPubTopicName, payload);
  //Serial.print("code : " + String(codeNum) + "\n");

  // free resources
  free(payload);
  esp_camera_fb_return(fb);
}

void Task1code(void * pvParameters){
  while(true){
    takePictureAndPublish();
    delay(1);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  //WiFi Config
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  
  //initCamera, AWS, NTP
  initCamera();
  initAWS();
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  
  xTaskCreatePinnedToCore(Task1code, "Task1", 10000, NULL, 0, &task1, 0);
  currentPaymentTime = millis() - 50000;
}

void loop() {
  if((WiFi.status() == WL_CONNECTED)) {
    if(millis() - currentPaymentTime >= paymentInterval){
      currentPaymentTime = millis();
      getPaymentStatus();
    }
    if(!paymentStatus && (millis() - currentFacedetectTime >= facedetectInterval)){
      currentFacedetectTime = millis();
      facedetectExecutionCount++;
      if(getFacedetect()) facedetectCount++;
      if(facedetectExecutionCount == 5){
        if(facedetectCount >= 3){
          //이메일 알림 pub 추가
          Serial.print("He is illegal user!\n");
        }
        else Serial.print("He is not a illegal user!\n");
        facedetectExecutionCount = facedetectCount = 0;
        paymentStatus = true;
      }
    }
  } else {
    Serial.println("Error on HTTP request");
  }
}


















void initCamera(){
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_QVGA;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.grab_mode = CAMERA_GRAB_LATEST;
  config.jpeg_quality = 10;
  config.fb_count = 2;
  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
}

void initAWS(){
  if(testButton.connect(hostAddress, clientId) == 0){
    Serial.println("Connected to AWS");
    delay(1000);
  }
  else{
    Serial.println("AWS connection failed, Check the HOST Address");
    while(1);
  }
}
