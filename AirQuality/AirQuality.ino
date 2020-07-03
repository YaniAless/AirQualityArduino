#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "Adafruit_CCS811.h"
#include "DHT.h"

#ifndef APSSID
#define APSSID "AQBox" // For testing
#define APPSK  "123456789" // For testing
#define DHTPIN 2
#define DHTTYPE DHT11
#endif

/* Set these to your desired credentials. */
const char *ssid = APSSID;
const char *password = APPSK;

// Case informations
String caseName = "AIRQUALITY_1";
const char *caseType = "AQ01";
const String sensors[] = { "Temp","Humidity","CO2","TVOC" };



DHT dht(DHTPIN, DHTTYPE);

ESP8266WebServer server(8080);
Adafruit_CCS811 ccs;

/* Just a little test message.  Go to http://192.168.4.1:8080 in a web browser
   connected to this access point to see it.
*/

void setup(){
  
  Serial.begin(9600);

  settingServer();
  settingCCSParameters(); 
  dht.begin();
}

void loop() {
  server.handleClient();
  //readDataIfAvailable();
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
  server.on("/humidity", getHumidity);
  server.on("/sensors/all", getAllSensors);
  server.on("/settings", getOrUpdateSettings);
  
  server.begin();
  Serial.println("HTTP server started");
}

void handleRoot() {  
  server.send(200, "text/html", "<h1>Welcome to the main page of your Air Quality box</h1>");
}

void getAllSensors() {
  StaticJsonDocument<200> doc;
  doc["temperature"] = getCurrentTemperature();
  doc["humidity"] = getCurrentHumidity();
  doc["co2"] = getCO2Datas();
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

void getCO2() {
  StaticJsonDocument<30> doc;
  doc = getCO2Datas();
  
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
  StaticJsonDocument<30> doc;
  doc = getTVOCDatas();  
  
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
  StaticJsonDocument<30> doc;
  doc = getCurrentTemperature();  
  String responseAsJsonStr;
  serializeJson(doc, responseAsJsonStr);
  
  if(responseAsJsonStr){
    server.send(200, "application/json", responseAsJsonStr);
  }
  else{
    server.send(204, "text/plain", "No content" );
  }
}

void getHumidity() {
  StaticJsonDocument<29> doc;
  doc = getCurrentHumidity();  
  String responseAsJsonStr;
  serializeJson(doc, responseAsJsonStr);
  
  if(responseAsJsonStr){
    server.send(200, "application/json", responseAsJsonStr);
  }
  else{
    server.send(204, "text/plain", "No content" );
  }
}

void getOrUpdateSettings() {
  if(server.method() == HTTP_POST){
    if(server.hasArg("plain") == false){
      server.send(200, "text/plain", "Body not received");
    }
    String body = server.arg("plain");
    StaticJsonDocument<200> doc;
    DeserializationError err = deserializeJson(doc, body);

    if(err){
      Serial.print("ERROR : ");
      Serial.println(err.c_str());
    }

    if(doc["caseName"].as<String>() != ""){
      caseName = doc["caseName"].as<String>();
      Serial.println(caseName);
      server.send(200,"text/plain", caseName);
    } else {
      Serial.println(caseName);
      server.send(200,"text/plain", "No param found");
    }    
  } else if(server.method() == HTTP_GET){    

    int sensorsArray = sizeof(sensors) / sizeof(String);
    // allocate the memory for the document
    StaticJsonDocument<200> jsonArray;   
     
    // create an empty array
    JsonArray sensorsAsJSON = jsonArray.to<JsonArray>();
    
    for(int i = 0; i < sensorsArray; i++){
      sensorsAsJSON.add(sensors[i]);
    }
    StaticJsonDocument<200> settingsAsJSON;
  
    settingsAsJSON["caseName"] = caseName;
    settingsAsJSON["caseType"] = caseType;
    settingsAsJSON["sensors"] = sensorsAsJSON;
    
    String responseAsJsonStr;
    serializeJson(settingsAsJSON, responseAsJsonStr);
    
    if(responseAsJsonStr){
      server.send(200, "application/json", responseAsJsonStr);
    }
    else{
      server.send(204, "text/plain", "No content" );
    }
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
      float humidity = getCurrentHumidity();
    }
    else{
      Serial.println("Error, can't get any datas !");
      while(1);
    }
  }
  delay(1000);
}

int getCO2Datas(){  
  if(ccs.available()){    
    if(!ccs.readData()){
      Serial.println(ccs.geteCO2());
      return ccs.geteCO2();
    }
    else{
      Serial.println("No values for CO2 sensor");;
      return -1;
    }
  }
}

int getTVOCDatas(){
  if(ccs.available()){    
    if(!ccs.readData()){
      Serial.println(ccs.getTVOC());
      return ccs.getTVOC();
    }
    else{
      Serial.println("No values for TVOC sensor");;
      return -1;
    }
  } 
}
      
float getCurrentTemperature(){
  float celciusTemperature = ccs.calculateTemperature();
  
  //Serial.println(String("Temp: ") + celciusTemperature + String(" C°"));
  return celciusTemperature;
}

float getCurrentHumidity(){
  float humidity = dht.readHumidity();
  
  //Serial.println(String("Humidity: ") + humidity + String(" %°"));
  return humidity;
}

/// CCS-END
/// 
/// CCS PARAMS AND DATA READING
/// 
/// CCS-END
