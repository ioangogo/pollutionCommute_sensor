#define IOTWEBCONF_DEBUG_TO_SERIAL true
#include <Arduino.h>
#include <IotWebConf.h>
#include <Preferences.h>
#include "transmission.hpp"
#include <rom/rtc.h>



const char deviceName[] = "commutePollution";
const char wifiPassword[] = "commutePollution";

Preferences preferences;

DNSServer dnsServer;
WebServer server(80);
IotWebConf iotConf(deviceName, &dnsServer, &server, wifiPassword);
#define BOOL_LEN 3
char sds11EN[BOOL_LEN];
boolean setupMode = false;

IotWebConfParameter sdsParam = IotWebConfParameter("enable SDS011 Sensor", "sds", sds11EN, BOOL_LEN, "number", NULL, "0", "max='1' min='0'");

void handleRoot();
void handleConfigSaved();

void setup() {
  Serial.begin(9600);
  Serial.println();
  Serial.println("Starting up...");

  //////////////////////////////////////////////////
  // Reset counter to get device into setup mode //
  ////////////////////////////////////////////////
  preferences.begin("CP",false);
  unsigned int counter = preferences.getUInt("counter", 0);
  if((int)rtc_get_reset_reason(0) == 1){
    counter++;
    if(counter > 3){
      counter = 0;
      Serial.println("3 Restarts, going into transmit mode");
    }
  }
  if(counter > 2){
      setupMode=true;
      Serial.println("2 Restarts, going into setup mode");
    }
  preferences.putUInt("counter", counter);
  // put your setup code here, to run once:


  if(setupMode){
    iotConf.addParameter(&sdsParam);
    iotConf.init();
    iotConf.setConfigSavedCallback(&handleConfigSaved);
    server.on("/", handleRoot);
    server.on("/config", []{ iotConf.handleConfig(); });
    server.onNotFound([](){ iotConf.handleNotFound(); });
  }
}

void loop() {
  if(setupMode){
    iotConf.doLoop();
  }else{
    doTransimission();
  }
  // put your main code here, to run repeatedly:
}

// Store the save settings in the prefrences so we dont have to start the config to get them
void handleConfigSaved(){
  Serial.println("Configuration was updated.");
  Serial.println(sds11EN);
  bool sdsEnabled = atoi(sds11EN);
  preferences.putBool("sdsEn", sdsEnabled);

}

void handleRoot(){
    Serial.println("got Request");
    if(iotConf.handleCaptivePortal()){
      return;
    }
    String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
    s += "<title>Commute Polution</title></head><body>";
    s += "<ul>";
    s += "<li> Enabled Sensors: ";
    s += sds11EN;
    s += "<li>String param value: ";
    s += preferences.getBool("sdsEn");
    s += "</ul>";
    s += "Go to <a href='config'>configure page</a> to change values.";
    s += "</body></html>\n";
    server.send(200, "text/html", s);
}