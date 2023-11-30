#include <WiFi.h>
#include <Arduino_JSON.h>
#include "esp_camera.h"
#include "base64.h"
#include <HTTPClient.h>
#include <AWS_IOT.h>

#define CAMERA_MODEL_AI_THINKER // Has PSRAM
#include "camera_pins.h"

const char seatInfoAPI[] = "";
const char facedetectAPI[] = "";
char hostAddress[] = "";
const char* ssid = "";
const char* password = "";
char clientId[] = "ghddmsrl100";
char sTOPIC_NAME[] = "esp32/bme280";
char pTOPIC_NAME[] = "esp32/bme280";
HTTPClient http;
AWS_IOT testButton;

void startCameraServer();

void captureQuick(){
  camera_fb_t *fb = NULL;
  fb = esp_camera_fb_get();
  if(!fb) return;
  esp_camera_fb_return(fb);
}


void cameraInit(){
  camera_config_t config;
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

void getPaymentStatus(){
  http.begin(seatInfoAPI);
  int httpCode = http.GET();
  Serial.print("status code : ");
  Serial.println(httpCode);
  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println(payload);

    // pseudo code
    // paseJSON(payload);
    // JSONVar responseObj = JSON.parse(payload);
    // JSONVar state = responseObj["payment"];
    // Serial.println(state);
    // if(state && 카메라가 사람을 인식했을 때){
    //   Serial.println("unusual user");
    //   publishSMS();
    // }
  }
  http.end();
}

void getFacedetect(){
  captureQuick();
  camera_fb_t *fb = NULL;
  fb = esp_camera_fb_get();
  if (!fb) {
    Serial.println("Camera capture failed");
    return;
  }
  
  http.begin(facedetectAPI);
  http.addHeader("Content-Type", "application/json");
  String endcodedImageData = base64::encode(fb->buf, fb->len);
  String httpRequestData = "{\"image\" : \"" + endcodedImageData + "\"}" ;
  Serial.println("Sended data : " + httpRequestData );
  
  int httpCode = http.POST(httpRequestData);
  Serial.print("status code : ");
  Serial.println(httpCode);
  if (httpCode > 0) {
    String payload = http.getString();
    Serial.println(payload);
  }
  http.end();
  esp_camera_fb_return(fb);
}

void takePictureaAndPublish(){
  camera_fb_t *fb = esp_camera_fb_get();
  if(!fb) {
     Serial.println("Camera capture failed in takePictureAndPublish()");
     return;
  }
  const char *data = (const char *)fb->buf;
  // Image metadata.  Yes it should be cleaned up to use printf if the function is available
  Serial.print("Size of image:");
  Serial.println(fb->len);
  Serial.print("Shape->width:");
  Serial.print(fb->width);
  Serial.print("height:");
  Serial.println(fb->height);

  String encodedImageData = base64::encode(fb->buf, fb->len);
  // formating json
  JSONVar myObject;
  myObject["image"] = encodedImageData;
  String jsonString = JSON.stringify(myObject);
  char* payload = (char *)malloc(jsonString.length() + 1);
  strcpy(payload, jsonString.c_str());
  Serial.print("cpy complete");  
  //Serial.println("Sended data : " + jsonString);
  // publishing topic 
  int codeNum = testButton.publish(pTOPIC_NAME, payload); 
  Serial.print("code : ");
  Serial.println(codeNum);
//  if(codeNum == 0){
//    Serial.print("Publish Message: ");
//    Serial.println(jsonString);
//  }
  Serial.println("Published");
  Serial.println();
  free(payload);
  // Killing cam resource
  esp_camera_fb_return(fb);
}


void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  cameraInit();
    
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  //startCameraServer();
  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");

  //AWS Setting
  if(testButton.connect(hostAddress, clientId) == 0){
    Serial.println("Connected to AWS");
    delay(1000);
  }
  else{
    Serial.println("AWS connection failed, Check the HOST Address");
    while(1);
  }
  
}

void loop() {
  if ((WiFi.status() == WL_CONNECTED)) {
//    getPaymentStatus();
//    getFacedetect();
    //publishMessage();
  } else {
    Serial.println("Error on HTTP request");
  }
  takePictureaAndPublish();
  delay(1);
//  String s = "{\"temp\" : 30}";
//  char publishJSON[100] = {0};
//  s.toCharArray(publishJSON, s.length()+1);
//  if(testButton.publish(pTOPIC_NAME, publishJSON) == 0){
//    Serial.print("Publish Message: ");
//    Serial.println(publishJSON);
//    }
//  else Serial.println("Publish failed");
//  delay(500);

  
//  camera_fb_t * frame;
//  frame = esp_camera_fb_get();
//  dl_matrix3du_t *image_matrix = dl_matrix3du_alloc(1, frame->width, frame->height, 3);
//  fmt2rgb888(frame->buf, frame->len, frame->format, image_matrix->item);
//
//  esp_camera_fb_return(frame);
//
//  box_array_t *boxes = face_detect(image_matrix, &mtmn_config);
//  if(boxes != NULL){
//    detections = detections + 1;
//    Serial.printf("Faces detected %d times \n", detections);  
//
//    dl_lib_free(boxes->score);
//    dl_lib_free(boxes->box);
//    dl_lib_free(boxes->landmark);
//    dl_lib_free(boxes);
//  }
//  dl_matrix3du_free(image_matrix);
}
