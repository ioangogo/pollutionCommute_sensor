// Redefining serial 1 pins so that hardware serial can be used
#define TX1 13
#define RX1 12

#include <Arduino.h>
#include <IotWebConf.h>
#include <Preferences.h>
#include <rom/rtc.h>
#include "globals.hpp"
#include "message.hpp"
#include "loraTransmission.hpp"
#include "powerManagement.hpp"
#include "dustSensor.hpp"
#include "gps.hpp"
// Global varible for the tasks, will be semaphore protected
Sensorpacket LoraPacket;
SemaphoreHandle_t packetSemaphore;
bool sendFlag = false; // Flag to tell the lora task to send
bool sentFlag = false; 
bool sleepFlag = false;// flag to tell the system to deep sleep
bool ttnConnected = false; // decide what we should do with the mesurements
uint64_t gpslocTimeUnix;
Preferences preferences;

const char deviceName[] = "commutePollution";
const char wifiPassword[] = "commutePollution";

//Config stuff
#define CONFIG_VERSION "NetworkSetUp"
DNSServer dnsServer;
WebServer server(80);
IotWebConf iotConf(deviceName, &dnsServer, &server, wifiPassword,CONFIG_VERSION);
char devEUIInput[devEUILen];
char appKEYInput[appKeyLen];
boolean setupMode = false;
IotWebConfParameter devEUIConfig = IotWebConfParameter("devEUI", "EUI", devEUIInput, devEUILen, "text", NULL, NULL);
IotWebConfParameter appkeyConfig = IotWebConfParameter("appKey", "KEY", appKEYInput, appKeyLen, "password", NULL, NULL);

void handleRoot();
void handleConfigSaved();

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(9600);
  Serial.println("Starting up...");

  //////////////////////////////////////////////////
  // Reset counter to get device into setup mode //
  ////////////////////////////////////////////////
  
  //Load Counter from flash storage 
  preferences.begin("CP",false);
  unsigned int counter = preferences.getUInt("counter", 0);
  bool configued = preferences.getBool("configured", false);

  // Detect if the reset reason was power based, we dont want deep sleep triggering this
  if((int)rtc_get_reset_reason(0) == 1){
    counter++;
    if(counter > 3){
      counter = 0;
      Serial.println("3 Restarts, going into transmit mode");
    }
  }
  // if the button has been pressed twice 
  if(counter > 2 || !configued){
    setupMode=true;
    Serial.println("2 Restarts, going into setup mode");
  }else{
    // Reset reset counter if we are not in setup mode and the reset was not power based
    if((int)rtc_get_reset_reason(0) != 1){
      counter = 0;
    }
  }
  preferences.putUInt("counter", counter); // save the counter
  delay(1000);//Allow the user to reset befor doing anything

  if(setupMode){
    iotConf.addParameter(&devEUIConfig);
    iotConf.addParameter(&appkeyConfig);
    iotConf.init();
    iotConf.setConfigSavedCallback(&handleConfigSaved);
    server.on("/", handleRoot);
    server.on("/config", []{ iotConf.handleConfig(); });
    server.onNotFound([](){ iotConf.handleNotFound(); });
  }else
  {
    LoraPacket.sensorContent.pm25 = -1;
    LoraPacket.sensorContent.lat = GPS_NULL;
    LoraPacket.sensorContent.lng = GPS_NULL;
    packetSemaphore = xSemaphoreCreateMutex();
    xTaskCreatePinnedToCore(ttnHandling, "HandelTTN", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(LoraSend, "sendTask", 2048, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(checkSendTask, "checksendTask", 2048, NULL, 2, NULL, 0);
  }
  
}

void loop() {
  if(setupMode){
    iotConf.doLoop();
  }else{
    if(sleepFlag){
      startTimerDeepSleep();
    }
    vTaskDelay(portTICK_PERIOD_MS/20000); //allow other threads to run
  }
  // put your main code here, to run repeatedly:
}

// Store the save settings in the prefrences so we dont have to start the config to get them
void handleConfigSaved(){
  Serial.println("Configuration was updated.");
  preferences.putString("APPKEY", appKEYInput);
  preferences.putString("DEVEUI", devEUIInput);
  preferences.putBool("configured",true);

}

void handleRoot(){
    Serial.println("got Request");
    if(iotConf.handleCaptivePortal()){
      return;
    }
    String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
    s += "<title>Commute Polution</title></head><body>";
    s += "<ul>";
    s += "<li> Appkey: ";
    s += appKEYInput;
    s += "</ul>";
    s += "Go to <a href='config'>configure page</a> to change values.";
    s += "</body></html>\n";
    server.send(200, "text/html", s);
}