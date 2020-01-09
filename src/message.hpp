#include <Arduino.h>

typedef struct
{
    int pm25;
    int lat;
    int lng;
    uint64_t gpsunix;
    bool dataPacket;
} sensorData;

#define PACKET_SIZE sizeof(sensorData)
#define INT_SIZE sizeof(int)
#define long_SIZE sizeof(uint64_t)

typedef union{
    sensorData sensor;
    byte packetBytes[PACKET_SIZE];
}packet;

String PacketToJson(packet pkt);