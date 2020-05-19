#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "Adafruit_CCS811.h"

#ifndef APSSID
#define APSSID "AQBox" // For testing
#define APPSK  "123456789" // For testing
#endif

/* Set these to your desired credentials. */
const char *ssid = APSSID;
const char *password = APPSK;

ESP8266WebServer server(8080);
Adafruit_CCS811 ccs;

/* Just a little test message.  Go to http://192.168.4.1:8080 in a web browser
   connected to this access point to see it.
*/

void setup(){
  
  Serial.begin(9600);

  settingServer();
  settingCCSParameters(); 
}

void loop() {
  server.handleClient();
  readDataIfAvailable();
}

/// SERVER-START
/// 
/// FUNCTIONS AND STUFF
/// 
/// SERVER-START

void settingServer() {
  delay(1000);
  
  Serial.println("Configuring access point...");
  
  /* You can remove the password parameter if you want the AP to be open. */
  WiFi.softAP(ssid, password);

  IPAddress myIP = WiFi.softAPIP();
  
  Serial.print("AP IP address: ");
  Serial.println(myIP);
  
  server.on("/", handleRoot);
  server.on("/co2", getCO2);
  server.on("/tvoc", getTVOC);
  server.on("/temp", getTemp);
  server.begin();
  Serial.println("HTTP server started");
}

void handleRoot() {  
  server.send(200, "text/html", "<h1>Welcome to the main page of your Air Quality box</h1>");
}

void getCO2() {
  StaticJsonDocument<200> doc;
  doc["co2"] = getCO2Datas();
  
  String responseAsJsonStr;
  serializeJson(doc, responseAsJsonStr);      
  
  if(responseAsJsonStr){
    server.send(200, "application/json", responseAsJsonStr);
  }
  else{
    server.send(204, "text/plain", "No content" );
  }
}

void getTVOC() {
  StaticJsonDocument<200> doc;
  doc["tvoc"] = getTVOCDatas();  
  String responseAsJsonStr;
  serializeJson(doc, responseAsJsonStr);  
  
  if(responseAsJsonStr){
    server.send(200, "application/json", responseAsJsonStr);
  }
  else{
    server.send(204, "text/plain", "No content" );
  }
}

void getTemp() {
  StaticJsonDocument<200> doc;
  doc["temperature"] = getCurrentTemperature();  
  String responseAsJsonStr;
  serializeJson(doc, responseAsJsonStr);
  
  if(responseAsJsonStr){
    server.send(200, "application/json", responseAsJsonStr);
  }
  else{
    server.send(204, "text/plain", "No content" );
  }
}

/// SERVER-END
/// 
/// FUNCTIONS AND STUFF
/// 
/// SERVER-END

/// CCS-START
/// 
/// CCS PARAMS AND DATA READING
/// 
/// CCS-START

void settingCCSParameters(){
  if(!ccs.begin()){
    Serial.println("Failed to start sensor! Please check your wiring.");
    while(1);
  }
  //calibrate temperature sensor
  while(!ccs.available());
  
  float temp = ccs.calculateTemperature();
  
  ccs.setTempOffset(temp - 25.0);
}

void readDataIfAvailable(){
  if(ccs.available()){    
    if(!ccs.readData()){
      int co2 = getCO2Datas();
      int tvoc = getTVOCDatas();
      float temp = getCurrentTemperature();
    }
    else{
      Serial.println("Error, can't get any datas !");
      while(1);
    }
  }
  delay(1000);
}

int getCO2Datas(){
  int co2 = ccs.geteCO2();
  if(co2){
    Serial.println(String("CO2: ") + co2 + String(" ppm"));
    return co2;
  }
  else {
    Serial.println("No values for CO2 sensor");
    return 0;
  }
}

int getTVOCDatas(){
  int tvocValue = ccs.getTVOC();

  if(ccs.geteCO2()){
    Serial.println(String("TVOC: ") + tvocValue + String(" ppb"));
    return tvocValue;
  }
  else {
    Serial.println("No values for TVOC sensor");
    return 0;
  }  
}
      
float getCurrentTemperature(){
  float celciusTemperature = ccs.calculateTemperature();
  
  Serial.println(String("Temp: ") + celciusTemperature + String(" C°"));
  return celciusTemperature;
}

/// CCS-END
/// 
/// CCS PARAMS AND DATA READING
/// 
/// CCS-END
