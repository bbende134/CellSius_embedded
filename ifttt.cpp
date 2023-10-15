#include <Arduino.h>

#include <WiFi.h>
#include <HTTPClient.h>

#include "time.h"

/////////////////// VARIABLES TO FILL OR CHANGE /////////////////

const char* ssid = "VOL_25"; // ESP32 Wifi SSID (your Wifi network name)
const char* password = "135792468"; // ESP32 WIFI Password (your Wifi password)

String eventNAME = "Light_Toggle_ESP32"; // Your IFTTT webhooks url
String webhooksKEY = "bk-rNjIlmxawG-VkvLWg9K"; // Your IFTTT webhooks url

// IFTTT website root certificate. 

int scanTime = 2; //In seconds, increase if your device needs more time to be discovered
int initScanTime = 25;
int leftTimeInterval = 200; // choose a somewhat big interval if you don't want false negative
time_t lastTimeDetected = 0;
bool isHere = NULL;

const int httpsPort = 443;
String url = "https://maker.ifttt.com/trigger/" + eventNAME + "/with/key/" + webhooksKEY;

// time constants

const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 0;
const int   daylightOffset_sec = 0;

void connectToWIFI(){
  if(WiFi.status() != WL_CONNECTED){
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nWiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void postWebhooks(int state = 2){
  /*
    state determines if you want to force the lights in a state:
      0 is off
      1 is on
      2 is toggle
    
    currently only toggle is implemented
  */

  // WIFI setup
  connectToWIFI();

  if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
    HTTPClient http;   
    
    http.begin(url);  //Specify destination for HTTP request
    int httpResponseCode = http.POST("");   //Send the actual POST request
    
    if(httpResponseCode>0){
    
      String response = http.getString();                       //Get the response to the request
    
      Serial.println(httpResponseCode);   //Print return code
      Serial.println(response);           //Print request answer
    
    }else{
    
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);
    
    }
    
    http.end();  //Free resources
    
  }else{
      Serial.println("Error in WiFi connection");   
  }
}

time_t getUnixTime()
{
  connectToWIFI();
  time_t now;
  time(&now);
  return now;
}





int scanID = 0;
void setup() {
  Serial.begin(115200);

  // WIFI setup
  Serial.println();
  Serial.print("connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  connectToWIFI();

  // time setup
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  getUnixTime(); // init time with ntp

  // Bluetooth setup
  Serial.println("Scanning...");
 // less or equal setInterval value

}

void loop() {

  double interval = difftime( getUnixTime(), lastTimeDetected);
  if(interval >= leftTimeInterval){ // user left
    postWebhooks();
    
  }
}