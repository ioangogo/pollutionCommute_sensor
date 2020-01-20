 
#include <Arduino.h>
#include <SPI.h>

#include <hal/hal_io.h>
#include <hal/print_debug.h>
#include <keyhandler.h>
#include <lmic.h>
#include "loraTransmission.hpp"
#include "message.hpp"
#include "globals.hpp"

const lmic_pinmap lmic_pins = {
    .nss = 18,
    .prepare_antenna_tx = nullptr,
    .rst = 14,
    .dio = {DIO0, DIO1}
};

OsScheduler OSS;
RadioSx1276 radio {lmic_pins};
LmicEu868 LMIC {radio, OSS};

//OsJob sendjob{OSS};

// Code from example code of the arduino-LMIC libary examples
// Find original source here: https://github.com/mcci-catena/arduino-lmic/blob/master/examples/ttn-otaa/ttn-otaa.ino
void printHex2(unsigned v) {
    v &= 0xff;
    if (v < 16)
        Serial.print('0');
    Serial.print(v, HEX);
}
void onEvent (EventType ev) {
    switch(ev) {
        case EventType::JOINED:
            Serial.println(F("EventType::JOINED"));
            ttnConnected = true;
            break;
        case EventType::JOINING:
            Serial.println(F("EventType::JOINING"));
            break;
        case EventType::TXCOMPLETE:
            Serial.println(F("EventType::TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.getTxRxFlags().test(TxRxStatus::ACK)) {
                PRINT_DEBUG(1, F("Received ack"));
                if(sentFlag == true){
                    // reset send state and allow other processes to use packet
                    sentFlag=false;
                    sleepFlag=true;
                }
            }
            if (LMIC.getDataLen()) {
                PRINT_DEBUG(1, F("Received %d bytes of payload"), LMIC.getDataLen());
                auto data = LMIC.getData();
                if (data) {
                    uint8_t port = LMIC.getPort();
                }
            }
            break;
            default:
                PRINT_DEBUG(1, F("Other Event: "));
    }
}
// End of code from external source

void LoraSend(void * param){
    for(;;){
        if(sendFlag && ttnConnected){
            if(xSemaphoreTake(packetSemaphore, portMAX_DELAY) == pdTRUE){
                // copy message buffer for packet
                byte buf[PACKET_SIZE];
                memcpy(buf, LoraPacket.packetBytes, PACKET_SIZE);

                for(int i = 0; i < PACKET_SIZE; i++){
                    Serial.printf("%02X", buf[i]);
                }
                Serial.println();

                //Set the packet as the next thing to be transmitted
                LMIC.setTxData2(2, buf, PACKET_SIZE, 0);

                // reset packet
                LoraPacket.sensorContent.pm25 = -1;
                LoraPacket.sensorContent.lat = GPS_NULL;
                LoraPacket.sensorContent.lng = GPS_NULL;
                
                // Signal to the main loop that we have sent the message
                sentFlag = true;

                xSemaphoreGive(packetSemaphore);
            }
        }
    vTaskDelay(1000/portTICK_PERIOD_MS);//Give other tasks a chance to run on the processor
    }
}


// Innitalising these with dummy values, we will strcpy over them later
constexpr char const devEui[devEUILen] = "D0740BC175E5F4BE";
constexpr char const appKey[appKeyLen] = "391F21F0D4859ED3249FE1C5DDB7C77E";

void ttnHandling(void * param){
    SPI.begin(5,19,27,18);
    os_init();
    LMIC.init();
    LMIC.reset();
    LMIC.setEventCallBack(onEvent);

    //Slightly hacky strcpy to get the user configured strings into the consts
    strcpy((char *)preferences.getString("DEVEUI").c_str(), devEui);
    strcpy((char *)preferences.getString("APPKEY").c_str(), appKey);
    
    SetupLmicKey<appEui, devEui, appKey>::setup(LMIC);

    Serial.printf("DEBUG: eui: %s, Key: %s\n", devEui, appKey);


    LMIC.setClockError(MAX_CLOCK_ERROR * 5 / 100);
    //LMIC.setAntennaPowerAdjustment(-14);

    LMIC.startJoining();

    while(true){
        OsDeltaTime to_wait = OSS.runloopOnce();
        //Due to timings we only want to free up the processor
        //to other tasks if the time we have to wait is larger than 10 seconds
        if(to_wait > OsDeltaTime(10)){
            vTaskDelay(10/portTICK_PERIOD_MS);
        }
        
        //Give other tasks a chance to run on the processor
    }
    
}