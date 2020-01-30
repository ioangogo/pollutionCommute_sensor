#include <Arduino.h>

#ifndef MESSAGE_H
#define MESSAGE_H

typedef struct
{
    int pm25;
    int lat;
    int lng;
} sensorData;

#define PACKET_SIZE (int)sizeof(sensorData)
#define INT_SIZE sizeof(int)
#define long_SIZE sizeof(uint64_t)

typedef union{
    sensorData sensorContent;
    byte packetBytes[PACKET_SIZE];
} Sensorpacket;
#endif

String PacketToJson(Sensorpacket pkt);

void stateLedThread( void *Param);

void MessageStateMachine();