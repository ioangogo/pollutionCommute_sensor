#include <Arduino.h>
#include <NMEAGPS.h>
#include "globals.hpp"

HardwareSerial GPS_Serial(2); 
#define gpsPort GPS_Serial
#define GPS_PORT_NAME "GPS_Serial 2"
#define RXD2 23 //16 is used for OLED_RST ! If we are using GPS we can not use the OLED
#define TXD2 17

static NMEAGPS gps;
static gps_fix fix;

uint64_t Y2KtoUnix(uint32_t timestamp){
    return timestamp + Y2K_OFFSET;
}

void gpsTask( void *Param){
    //gpsPort.begin(9600, SERIAL_8N1, RXD2, TXD2);
    bool locGot = false;
    bool timeGot = false;

    for(;;){

        if(!timeGot){
            if(xSemaphoreTake(packetSemaphore, portMAX_DELAY) == pdTRUE){
                LoraPacket.sensorContent.gpsunix = 3454363674;
                timeGot = true;
                xSemaphoreGive(packetSemaphore);
            }

        }
        if(!locGot){
            // multipling by 1000 for transmit efficency and also to limit accuracy to 111m
            if(xSemaphoreTake(packetSemaphore, portMAX_DELAY) == pdTRUE){
                LoraPacket.sensorContent.lat = 33.03*1000;
                LoraPacket.sensorContent.lng = 33.03*1000;
                locGot = true;
                xSemaphoreGive(packetSemaphore);
            }
        /*while(gps.available(gpsPort)){
            fix = gps.read();
        }
        if(fix.valid.time && !timeGot){
            uint32_t UTCy2k = (NeoGPS::clock_t) fix.dateTime;
            uint64_t unixtimestamp = Y2KtoUnix(UTCy2k);
            if(xSemaphoreTake(packetSemaphore, portMAX_DELAY) == pdTRUE){
                LoraPacket.sensorContent.gpsunix = unixtimestamp;
                timeGot = true;
                xSemaphoreGive(packetSemaphore);
            }

        }
        if(fix.valid.location && !locGot){
            // multipling by 1000 for transmit efficency and also to limit accuracy to 111m
            int lat = round(fix.latitude()*1000);
            int lng = round(fix.longitude()*1000);
            if(xSemaphoreTake(packetSemaphore, portMAX_DELAY) == pdTRUE){
                LoraPacket.sensorContent.lat = lat;
                LoraPacket.sensorContent.lat = lng;
                locGot = true;
                xSemaphoreGive(packetSemaphore);
            }*/
        


        }
        vTaskDelay(1);//Free the system up to do other tasks
    }
    

}