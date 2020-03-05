#include <Arduino.h>

#ifndef MESSAGE_H
#define MESSAGE_H

typedef struct
{
    int pm25;
    int lat;
    int lng;
    int nonce;
} sensorData;

#define PACKET_SIZE (int)sizeof(sensorData)
#define INT_SIZE sizeof(int)

typedef struct
{
    byte packetBytes[PACKET_SIZE];
    long time;
} sensorFileStruct;

#define FILE_SIZE (int)sizeof(sensorFileStruct)

typedef union{
    sensorData sensorContent;
    byte packetBytes[PACKET_SIZE];
} Sensorpacket;

typedef union{
    sensorFileStruct fileContent;
    byte fileBytes[FILE_SIZE];
} SensorFile;

#endif

String PacketToJson(Sensorpacket pkt, unsigned long time);

void stateLedThread( void *Param);

void MessageStateMachine();