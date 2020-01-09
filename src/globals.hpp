#include <Arduino.h>
#include "message.hpp"

#define

#define Y2K_OFFSET 946684800;

extern packet LoraPacket;
extern SemaphoreHandle_t packetSemaphore;
extern bool send;