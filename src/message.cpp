#include <Arduino.h>
#include "dustSensor.hpp"
#include "gps.hpp"
#include "globals.hpp"
#include "message.hpp"

String PacketToJson(Sensorpacket pkt);

void checkSendTask( void *Param){
    initGPS();
    initSDS();
    for(;;){
        doGPSTask();
        doSDS();
        if(xSemaphoreTake(packetSemaphore, portMAX_DELAY) == pdTRUE){
                bool pmSet = LoraPacket.sensorContent.pm25 != -1;
                bool gpsSet = LoraPacket.sensorContent.lat != GPS_NULL && LoraPacket.sensorContent.lng != GPS_NULL;
                if(pmSet && gpsSet ){
                    float pm = LoraPacket.sensorContent.pm25/10.0;
                    float outLat = LoraPacket.sensorContent.lat/1000.0;
                    float outLng = LoraPacket.sensorContent.lng/1000.0;
                    Serial.printf("PM2.5: %f GPS: %f, %f\r\n", pm, outLat, outLng);
                    sendFlag = true; // Set the send flag so that lora can start transmission
                }
                xSemaphoreGive(packetSemaphore);
        }
        vTaskDelay(1);// allow the other tasks to run
    }


};