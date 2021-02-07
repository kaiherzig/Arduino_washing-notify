#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
/*
 *Edit WIFI_SSID to your SSID 
 *Edit WIFI_KEY to your Password
 *Edit NOTIFY_URL_START to your WebApi
 *Edit NOTIFY_URL_STOP to your WebApi
 *To use CallMeBot referer to https://www.callmebot.com/blog/free-api-whatsapp-messages/
 *URL consists of phone= Number in +xxnumber
 *text seperated by +
 *apikey is your api key
 */
#define WIFI_SSID "<<FILLME>>"
#define WIFI_KEY "<<FILLME>>"
#define NOTIFY_URL_START "http://api.callmebot.com/whatsapp.php?phone=+49<<NUMBER>>&text=Die+Waschmaschine+schlaeudert&apikey=<<APIKEY>>"
#define NOTIFY_URL_STOP "http://api.callmebot.com/whatsapp.php?phone=+49<NUMBER>>&text=Die+Waschmaschine+ist+fertig&apikey=<<APIKEY>>"

#define SECOND 1000
#define QUARTER_SECOND 250

#define SENSOR_PIN D1

bool machineRunning = false;

bool lastState = false;
int lastTripped = 0;

int tripBucket = 0;
int tripBucketLastDripped = 0;


void setup() {
  Serial.begin(115200);

  pinMode(SENSOR_PIN, INPUT);
}


void loop() {
  int now = millis();
  int sinceLastTripped = now - lastTripped;
  int sinceLastDrip = now - tripBucketLastDripped;

  if (tripBucket > 0 && sinceLastDrip > SECOND) {
    tripBucket--;
    tripBucketLastDripped = now;
    Serial.print("Drip! ");
    Serial.println(tripBucket);
  }

  // Read the state and see if the sensor was tripped
  bool state = digitalRead(SENSOR_PIN) == 0 ? false : true;
  if (lastState != state) {
    lastState = state;

    // Can be tripped a maximum of once per second
    if (sinceLastTripped > QUARTER_SECOND) {
      lastTripped = now;

      if (tripBucket < 300) {
        tripBucket++;
      }
    }
  }


  if (machineRunning && tripBucket == 0) {
    machineRunning = false;
    Serial.println("Machine stopped");
    sendDoneNotification();
  }

  if (!machineRunning && tripBucket > 60) {
    machineRunning = true;
    Serial.println("Machine started");
    sendStartNotification();
  }

  delay(5);
}


void sendStartNotification() {
  WiFi.begin(WIFI_SSID, WIFI_KEY);
  
  while((WiFi.status() != WL_CONNECTED)) {
    delay(100);
  }

  HTTPClient http;
  http.begin(NOTIFY_URL_START);
  int httpCode = http.GET();
  if(httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);

    // file found at server
    if(httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
    }
  } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();

  WiFi.disconnect();
}
void sendDoneNotification() {
  WiFi.begin(WIFI_SSID, WIFI_KEY);
  
  while((WiFi.status() != WL_CONNECTED)) {
    delay(100);
  }

  HTTPClient http;
  http.begin(NOTIFY_URL_STOP);
  int httpCode = http.GET();
  if(httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);

    // file found at server
    if(httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println(payload);
    }
  } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }
  http.end();

  WiFi.disconnect();
}
