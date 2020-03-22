#include <Arduino.h>
#include <ArduinoJson.h>
#include "dustSensor.hpp"
#include "gps.hpp"
#include "globals.hpp"
#include "message.hpp"
#include "loraTransmission.hpp"
#include "powerManagement.hpp"
#include "fileSystemAccess.hpp"

int state = INIT;

String PacketToJson(Sensorpacket pkt, unsigned long time){
    String output;
    const size_t capacity = JSON_OBJECT_SIZE(3) + JSON_OBJECT_SIZE(4);
    DynamicJsonDocument doc(capacity);

    doc["sensor"] = "291A45544A0FA55D";
    doc["time"] = time;

    JsonObject data = doc.createNestedObject("data");
    data["pm25"] = pkt.sensorContent.pm25;
    data["lat"] = pkt.sensorContent.lat;
    data["lng"] = pkt.sensorContent.lng;
    data["nonce"] = pkt.sensorContent.nonce;

    serializeJson(doc, output);
    return output;
}

void stateLedThread(void *Param){
    for(;;){
        digitalRead(LED_BUILTIN)?digitalWrite(LED_BUILTIN,0):digitalWrite(LED_BUILTIN,1);
        delay((state+1)*1000);// Change blink speed depending on state
    }
}

void MessageStateMachine(){
    switch(state){
        case INIT:{
            // to make sure we dont send the same packet twice to the webserver
            // we generate a random number
            LoraPacket.sensorContent.nonce = esp_random();

            Serial.println("Inital Init");
            initGPS();
            initSDS();
            state = GPS;
            Serial.println("GPS Sate");
        }
        case GPS:{
            doGPSTask();
            bool gpsSet = LoraPacket.sensorContent.lat != GPS_NULL && LoraPacket.sensorContent.lng != GPS_NULL;
            state = gpsSet?SDS_INIT:GPS;
            break;
        }
        case SDS_INIT:{
            Serial.println("SDS state");
            SDSstateInit();
            state = SDS;
            break;
        }
        case SDS:{
            doSDS();
            bool pmSet = LoraPacket.sensorContent.pm25 != -1;
            state = pmSet?LORA_JOIN:SDS;
            break;
        }
        case LORA_JOIN:{
            Serial.println("Send State");
            loraInit();
            state=LORA_SEND;
            break;
        }
        case LORA_SEND:{
            loraLoop();
            break;
        }
        case LORA_FAILED:
            failedmessageState();
            break;
        case SLEEP:{
            startTimerDeepSleep();
            break;
        }
    }

}