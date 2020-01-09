// Redefining serial 1 pins so that hardware serial can be used
#define TX1 13
#define RX1 12

#include <Arduino.h>
#include <SDS011.h>
#include "globals.hpp"




SDS011 my_sds;
HardwareSerial port(1);

void sdsTask( void *Param){
    float pm10, pm25;
    int err;
    my_sds.begin(&port);
    my_sds.wakeup();//the device is probably sleeping, wake it up so it can begin mesurements

    for(;;){
        err = my_sds.read(&pm10, &pm25);
        if(!err){
            if(xSemaphoreTake(packetSemaphore, portMAX_DELAY) == pdTRUE){
                LoraPacket.sensorContent.pm25 = round(pm25*10);
                my_sds.sleep();// Send sensor to sleep(turn fan off) to save power while we dont need data
                xSemaphoreGive(packetSemaphore);
                
            }

        }
        vTaskDelay(1);//Free the system up to do other tasks
        
    }
}