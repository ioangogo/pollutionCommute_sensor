// Redefining serial 1 pins so that hardware serial can be used
#define dustTX 13
#define dustRX 12

#include <Arduino.h>
#include <SDS011.h>
#include "globals.hpp"



#define sdsSerial Serial1
SDS011 my_sds;
bool notcap = true;
//record when we started up as we want to run the fan for 30 seconds before we take a mesurement
unsigned long startMilis;

void sleepSDS(){
    if(sdsSerial){
        my_sds.sleep();
    }
}

int samples = 3;
float sample = 0.0;

void initSDS(){
    #ifndef NO_SENSORS
    my_sds.begin(&sdsSerial, dustRX, dustTX);
    my_sds.sleep();
    #endif
}

void SDSstateInit(){
    my_sds.wakeup();
    startMilis = millis();
}

void doSDS(){
    #ifndef NO_SENSORS
    float pm10, pm25;
    int err;
    unsigned long timeSinceInit = millis() - startMilis;
    if(notcap && timeSinceInit >= 30000){
        while(samples >0){
            err = my_sds.read(&pm25, &pm10);
            if(err == 0){
                Serial.printf("%f\r\n", pm25);
                Serial.println("got pm2.5");
                sample+=pm25;
                samples -=1;
                delay(10000);
            }
        }
    }
    if(samples==0){
        sample=sample/3;
        LoraPacket.sensorContent.pm25 = lround(sample*10);
    }
    
    #else
    if(notcap){
    if(xSemaphoreTake(packetSemaphore, portMAX_DELAY) == pdTRUE){
        Serial.println("got pm2.5");
        LoraPacket.sensorContent.pm25 = lround(10.5*10);
        notcap = false;
        xSemaphoreGive(packetSemaphore);   
    }}
    #endif
}