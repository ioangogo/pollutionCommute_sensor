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

void initSDS(){
    my_sds.begin(&sdsSerial, dustRX, dustTX);
    my_sds.wakeup();
    startMilis = millis();
}

void doSDS(){
    float pm10, pm25;
    int err;
    unsigned long timeSinceInit = millis() - startMilis;
    if(notcap && timeSinceInit == 30000){
        err = my_sds.read(&pm25, &pm10);
        if(err == 0){
            Serial.printf("%f\r\n", pm25);
            if(xSemaphoreTake(packetSemaphore, portMAX_DELAY) == pdTRUE){
                LoraPacket.sensorContent.pm25 = lround(pm25*10);
                my_sds.sleep();// Send sensor to sleep(turn fan off) to save power while we dont need data
                notcap = false;
                xSemaphoreGive(packetSemaphore);   
            }
        }
    }
}