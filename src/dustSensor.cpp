// Redefining serial 1 pins so that hardware serial can be used
#define TX1 13
#define RX1 12

#include <Arduino.h>
#include <SDS011.h>
#include "globals.hpp"




SDS011 my_sds;
HardwareSerial sds(1);
bool cap = false;

void initSDS(){
    my_sds.begin(&sds);
    my_sds.wakeup();
}

void doSDS(){
    float pm10, pm25;
    int err;
    err = my_sds.read(&pm10, &pm25);
    if(!err && !cap){
        if(xSemaphoreTake(packetSemaphore, portMAX_DELAY) == pdTRUE){
            LoraPacket.sensorContent.pm25 = round(pm25*10);
            #ifndef SENSORLESS
            my_sds.sleep();// Send sensor to sleep(turn fan off) to save power while we dont need data
            #endif
            cap = true;
            xSemaphoreGive(packetSemaphore);   
        }
    }
}