#include <Arduino.h>
#include "globals.hpp"
#include "message.hpp"

String PacketToJson(packet pkt);

void checkSendTask( void *Param){
    for(;;){
        if(xSemaphoreTake(packetSemaphore, portMAX_DELAY) == pdTRUE){
                bool pmSet = LoraPacket.sensor.pm25 != NULL;
                bool gpsSet = LoraPacket.sensor.lat != NULL && LoraPacket.sensor.lng != NULL;
                bool unixSet = LoraPacket.sensor.gpsunix != NULL;
                if(pmSet && gpsSet && unixSet){
                    send = true; // Set the send flag so that lora can start transmission
                }
                xSemaphoreGive(packetSemaphore);
        }
        vTaskDelay(1);// allow the other tasks to run
    }


};