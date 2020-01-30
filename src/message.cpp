#include <Arduino.h>
#include "dustSensor.hpp"
#include "gps.hpp"
#include "globals.hpp"
#include "message.hpp"
#include "loraTransmission.hpp"
#include "powerManagement.hpp"

int state = GPS;

String PacketToJson(Sensorpacket pkt);

void stateLedThread(void *Param){
    for(;;){
        digitalRead(LED_BUILTIN)?digitalWrite(LED_BUILTIN,0):digitalWrite(LED_BUILTIN,1);
        delay(state+1*1000);
    }
}

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

        case LORA_SEND:
        bool runOnce = false;
        if(!runOnce){
            runOnce = true;
            loraInit();
        }
        loraLoop();
        break;

        case SLEEP:
        startTimerDeepSleep();
        break;
        

    }

}