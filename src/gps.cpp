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
    gpsPort.begin(9600, SERIAL_8N1, RXD2, TXD2);

    for(;;){
        while(gps.available(gpsPort)){
            fix = gps.read();
        }
        if(fix.valid.time){
            uint32_t UTCy2k = (NeoGPS::clock_t) fix.dateTime;
            uint64_t unixtimestamp = Y2KtoUnix(UTCy2k);
            if(xSemaphoreTake(packetSemaphore, portMAX_DELAY) == pdTRUE){
                LoraPacket.sensor.gpsunix = unixtimestamp;
                xSemaphoreGive(packetSemaphore);
            }

        }
        if(fix.valid.location){
            // multipling by 1000 for transmit efficency and also to limit accuracy to 111m
            int lat = round(fix.latitude()*1000);
            int lng = round(fix.longitude()*1000);
            if(xSemaphoreTake(packetSemaphore, portMAX_DELAY) == pdTRUE){
                LoraPacket.sensor.lat = lat;
                LoraPacket.sensor.lat = lng;
                xSemaphoreGive(packetSemaphore);
            }



        }
        vTaskDelay(1);//Free the system up to do other tasks
    }
    

}