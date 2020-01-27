#include <Arduino.h>
#include <NMEAGPS.h>
#include "globals.hpp"
#include "gps.hpp"

#define GPS_Serial Serial2
#define gpsPort GPS_Serial
#define GPS_PORT_NAME "GPS_Serial 2"
#define RXpin 37 //16 is used for OLED_RST ! If we are using GPS we can not use the OLED
#define TXpin 17
bool locGot = false;
bool timeGot = false;

static NMEAGPS gps;
static gps_fix fix;

void sleepDevice(HardwareSerial &serial){
    serial.write(SLEEP_CMD);
}
void WakeDevice(HardwareSerial &serial){
    serial.write(WAKE_CMD);
}

void deepsleepSleep(){
    if(gpsPort){
        sleepDevice(gpsPort);
    }
}

uint64_t Y2KtoUnix(uint32_t timestamp){
    return timestamp + Y2K_OFFSET;
}
unsigned long lastMilis = 0;

void initGPS(){
    #ifndef NO_SENSORS
    gpsPort.begin(9600, SERIAL_8N1, RXpin, TXpin);
    WakeDevice(gpsPort);
    #endif
    
}

void doGPSTask(){
    #ifndef NO_SENSORS
    while(gps.available(gpsPort)){
        fix = gps.read();
    }
    if(fix.valid.time && !timeGot){
        Serial.println("got Time");
        uint32_t UTCy2k = (NeoGPS::clock_t) fix.dateTime;
        uint64_t unixtimestamp = Y2KtoUnix(UTCy2k);
        //timeGot = true;
        gpslocTimeUnix = unixtimestamp;
    }
    if(fix.valid.location && !locGot){
        Serial.println("got location");
        // multipling by 1000 for transmit efficency and also to limit accuracy to 111m
        int lat = lround(fix.latitude()*1000);
        int lng = lround(fix.longitude()*1000);
        if(xSemaphoreTake(packetSemaphore, portMAX_DELAY) == pdTRUE){
            LoraPacket.sensorContent.lat = lat;
            LoraPacket.sensorContent.lat = lng;
            locGot = true;
            sleepDevice(gpsPort);
            xSemaphoreGive(packetSemaphore);
        }
    }
    #else
    if(fix.valid.time && !timeGot){
        gpslocTimeUnix = 1580121254;
        timeGot = true;
    }
    if(!locGot){
        Serial.println("got location");
        // multipling by 1000 for transmit efficency and also to limit accuracy to 111m
        int lat = lround(51.4981278*10000);
        int lng = lround(-2.5380904*10000);
        if(xSemaphoreTake(packetSemaphore, portMAX_DELAY) == pdTRUE){
            LoraPacket.sensorContent.lat = lat;
            LoraPacket.sensorContent.lng = lng;
            locGot = true;
            xSemaphoreGive(packetSemaphore);
        }
    }
    #endif
}
