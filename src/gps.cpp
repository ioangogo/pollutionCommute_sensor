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
        WakeDevice(gpsPort);
        sleepDevice(gpsPort);
    }
}

unsigned long Y2KtoUnix(uint32_t timestamp){
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
    bool hastime = false;
    while(gps.available(gpsPort) && !locGot){
        fix = gps.read();
        if(fix.valid.time && !timeGot){
            uint32_t UTCy2k = (NeoGPS::clock_t) fix.dateTime;
            unsigned long unixtimestamp = Y2KtoUnix(UTCy2k);
            //timeGot = true;
            gpslocTimeUnix = unixtimestamp;
            if (!hastime){printf("got time\n"); hastime=true;}
            
        }
        if(fix.valid.location){
            // multipling by 1000 for transmit efficency and also to limit accuracy to 111m
            int lat = lround(fix.latitude()*10000);
            int lng = lround(fix.longitude()*10000);

            LoraPacket.sensorContent.lat = lat;
            LoraPacket.sensorContent.lng = lng;
            locGot = true;
            sleepDevice(gpsPort);
        }
    }
    #else
    if(!timeGot){
        gpslocTimeUnix = 1583590184;
        timeGot = true;
    }
    if(!locGot){
        // multipling by 1000 for transmit efficency and also to limit accuracy to 111m
        // in test mode we just pretend that we are recording some data at null island
        int lat = lround(0.0*10000);
        int lng = lround(0.0*10000);
            LoraPacket.sensorContent.lat = lat;
            LoraPacket.sensorContent.lng = lng;
            locGot = true;
    }
    #endif
}
