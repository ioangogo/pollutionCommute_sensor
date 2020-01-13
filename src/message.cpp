#include <Arduino.h>
#include "globals.hpp"
#include "message.hpp"

String PacketToJson(Sensorpacket pkt);

void checkSendTask( void *Param){
    for(;;){
        if(xSemaphoreTake(packetSemaphore, portMAX_DELAY) == pdTRUE){
                bool pmSet = LoraPacket.sensorContent.pm25 != -1;
                bool gpsSet = LoraPacket.sensorContent.lat != GPS_NULL && LoraPacket.sensorContent.lng != GPS_NULL;
                bool unixSet = LoraPacket.sensorContent.gpsunix != 0;
                if(pmSet && gpsSet && unixSet){
                    sendFlag = true; // Set the send flag so that lora can start transmission
                }
                xSemaphoreGive(packetSemaphore);
        }
        vTaskDelay(1);// allow the other tasks to run
    }


};