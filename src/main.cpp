#include <Arduino.h>
#include <IotWebConf.h>
#include <Preferences.h>
#include <HTTPClient.h>
#include <rom/rtc.h>
#include "globals.hpp"
#include "message.hpp"
#include "loraTransmission.hpp"
#include "fileSystemAccess.hpp"
#include "powerManagement.hpp"
#include "dustSensor.hpp"
#include "gps.hpp"
#include "keyHandling.hpp"

// Global varible for the tasks
Sensorpacket LoraPacket;
bool sendFlag = false; // Flag to tell the lora task to send
bool sentFlag = false; 
bool sleepFlag = false;// flag to tell the system to deep sleep
bool ttnConnected = false; // decide what we should do with the mesurements
bool wifiConnected = false;
unsigned long gpslocTimeUnix;
Preferences preferences;

const char deviceName[] = "commutePollution";
const char wifiPassword[] = "commutePollution";

//Config stuff
#define CONFIG_VERSION "NetworkSetUp"
DNSServer dnsServer;
WebServer server(80);

WiFiClient net;
IotWebConf iotConf(deviceName, &dnsServer, &server, wifiPassword,CONFIG_VERSION);
char devEUIInput[devEUILen];
char appKEYInput[appKeyLen];
boolean setupMode = false;
IotWebConfParameter devEUIConfig = IotWebConfParameter("devEUI", "EUI", devEUIInput, devEUILen, "text", NULL, NULL);
IotWebConfParameter appkeyConfig = IotWebConfParameter("appKey", "KEY", appKEYInput, appKeyLen, "password", NULL, NULL);

void handleRoot();
void handleConfigSaved();
void handleReset();
void ShowMSGs();
void clrmsg();
void sendMessagesOverWifi();

void wifiCon(){
  wifiConnected=true;
}

void setup() {
  initFileSystem();
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN,1);
  Serial.begin(9600);
  Serial.println("Starting up...");

  //////////////////////////////////////////////////
  // Reset counter to get device into setup mode //
  ////////////////////////////////////////////////
  
  //Load Counter from flash storage 
  preferences.begin("CP",false);
  unsigned int counter = preferences.getUInt("counter", 0);

  // Detect if the reset reason was power based, we dont want deep sleep triggering this
  if((int)rtc_get_reset_reason(0) == 1){
    counter++;
    preferences.putUInt("counter", counter);
    if(counter >= 3){
      counter = 0;
      Serial.println("3 Restarts, going into transmit mode");
    }
  }
  // if the button has been pressed twice 
  if(counter >= 2 || !KeysExistInPref()){
    setupMode=true;
    Serial.println("2 Restarts, going into setup mode");
  }else{
    // Reset reset counter if we are not in setup mode and the reset was not power based
    if((int)rtc_get_reset_reason(0) != 1){
      counter = 0;
    }
  }
  
  delay(2000);//Allow the user to reset befor doing anything
  digitalWrite(LED_BUILTIN,0);
  if(setupMode){
    digitalWrite(LED_BUILTIN, 1);
    iotConf.addParameter(&devEUIConfig);
    iotConf.addParameter(&appkeyConfig);
    iotConf.init();
    iotConf.setConfigSavedCallback(&handleConfigSaved);
    iotConf.setWifiConnectionCallback(wifiCon);
    server.on("/", handleRoot);
    server.on("/reset", handleReset);
    server.on("/msgs", ShowMSGs);
    server.on("/clear", clrmsg);
    server.on("/send", sendMessagesOverWifi);
    server.on("/config", []{ iotConf.handleConfig(); });
    server.onNotFound([](){ iotConf.handleNotFound(); });
  }else{
    digitalWrite(LED_BUILTIN, 0);
    counter = 0;
    LoraPacket.sensorContent.pm25 = -1;
    LoraPacket.sensorContent.lat = GPS_NULL;
    LoraPacket.sensorContent.lng = GPS_NULL;
    xTaskCreatePinnedToCore(stateLedThread, "stateLedThread", 2048, NULL, 2, NULL, 0);
  }
   // save the counter
  preferences.putUInt("counter", counter);
}

void loop() {
  if(setupMode){
    iotConf.doLoop();
  }else{
    MessageStateMachine();
  }
}

// Store the save settings in the prefrences so we dont have to start the config to get them
void handleConfigSaved(){
  Serial.println("Configuration was updated.");
  preferences.putString("APPKEY", appKEYInput);
  preferences.putString("DEVEUI", devEUIInput);
}

void clrmsg(){
  rmDirContents("/msgs");
  String s = "<html><head><meta http-equiv=\"Refresh\" content=\"0; url=/msgs\" /></head><body><p>Please follow <a href=\"/msgs\">this link</a>.</p></body></html>";
  server.send(301, "text/html", s);
}

void sendMessagesOverWifi(){
  Serial.println("Sending Packets");
  sendMessages(net);
  String s = "<html><head><meta http-equiv=\"Refresh\" content=\"0; url=/msgs\" /></head><body><p>Please follow <a href=\"/msgs\">this link</a>.</p></body></html>";
  server.send(200, "text/html", s);
}

void handleReset(){
  esp_restart();
}

void handleRoot(){
    if(iotConf.handleCaptivePortal()){
      return;
    }
    String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
    s += "<title>Commute Polution</title></head><body>";
    s += "<h1>Commute Pollution Sensor</h1>";
    s += "<ul>";
    s += "</ul><a href='msgs'><h2>Saved Messages</h2></a>";
    s += lsDirHTML("/msgs");
    s += "<h2>root</h2>";
    s += lsRootHTML();
    s += "Go to <a href='config'>configure page</a> to change values.<br>";
    s += "<a href='reset'>Reset Node</a>";
    s += "</body></html>\n";
    server.send(200, "text/html", s);
}

void ShowMSGs(){
    String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
    s += "<title>Commute Polution</title></head><body>";
    s += listMessages("/msgs");
    if(wifiConnected){
      unsigned int cachePrevention = esp_random();
      String cacheStr= String(cachePrevention);
      s+="<a href='/send?v="+cacheStr+"'>Send Messages</a><br>";
    }
    s += "<a href='/clear'>clear</a><br><a href='/'>Back to root</a>";
    s += "</body></html>\n";
    server.send(200, "text/html", s);
}