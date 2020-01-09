#include <Arduino.h>

typedef struct
{
    int pm25 = NULL;
    int lat = NULL;
    int lng = NULL;
    uint64_t gpsunix = NULL;
} sensorData;

#define PACKET_SIZE sizeof(sensorData)
#define INT_SIZE sizeof(int)
#define long_SIZE sizeof(uint64_t)

typedef union{
    sensorData sensor;
    byte packetBytes[PACKET_SIZE];
} packet;

String PacketToJson(packet pkt);

void checkSendTask( void *Param);