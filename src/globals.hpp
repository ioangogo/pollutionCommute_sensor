#include <Arduino.h>
#include "message.hpp"

#define Y2K_OFFSET 946684800;

extern Sensorpacket LoraPacket;
extern SemaphoreHandle_t packetSemaphore;
extern bool send;
extern bool sent; // This is to send the device into deep sleep when needed

extern Preferences preferences;