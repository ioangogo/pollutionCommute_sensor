#include <Arduino.h>
#include "dustSensor.hpp"
#include "gps.hpp"
#include "globals.hpp"
#include "message.hpp"

int state = GPS;

String PacketToJson(Sensorpacket pkt);

void MessageStateMachine(){
    switch(state){
        case GPS:
        doGPSTask();
        bool gpsSet = LoraPacket.sensorContent.lat != GPS_NULL && LoraPacket.sensorContent.lng != GPS_NULL;
        state = gpsSet?SDS:GPS;
        break;

        case SDS:
        doSDS();
        bool pmSet = LoraPacket.sensorContent.pm25 != -1;
        state = pmSet?LORA_SEND:SDS;
        break;

    }

}