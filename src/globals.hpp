#include <Arduino.h>
#include <Preferences.h>
#include <NeoTime.h>
#include "message.hpp"

#define Y2K_OFFSET 946684800
#define GPS_NULL 333.3333*10000
#define uS_TO_S_FACTOR 1000000
#define MIN_TO_MS 60*uS_TO_S_FACTOR 
#define devEUILen 17
#define appKeyLen 33

#define EUIIntLen 8
#define appKeyIntLen 16

#ifndef enumStates_H
#define enumStates_H

enum States {
    INIT,
    GPS,
    SDS_INIT,
    SDS,
    LORA_JOIN,
    LORA_SEND,
    LORA_FAILED,
    SLEEP
};
#endif

extern int state;

extern Sensorpacket LoraPacket;
extern bool sendFlag;
extern bool sleepFlag; // This is to send the device into deep sleep when needed
extern bool sentFlag; // This is to send the device into deep sleep when needed
extern bool ttnConnected;
extern uint64_t gpslocTimeUnix;//unix time of a GPS lock, using a 64bit interger becuse we are close to 2038

extern Preferences preferences;
