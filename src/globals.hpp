#include <Arduino.h>
#include <Preferences.h>
#include "message.hpp"

#define Y2K_OFFSET 946684800
#define GPS_NULL 333.33*1000
#define uS_TO_S_FACTOR 1000000
#define MIN_TO_MS 60*uS_TO_S_FACTOR 

extern Sensorpacket LoraPacket;
extern SemaphoreHandle_t packetSemaphore;
extern bool send;
extern bool sleepFlag; // This is to send the device into deep sleep when needed
extern bool sent; // This is to send the device into deep sleep when needed

extern Preferences preferences;
