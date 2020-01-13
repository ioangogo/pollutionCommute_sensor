 
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
    .dio = {26, 35}
};

OsScheduler OSS;
RadioSx1276 radio {lmic_pins};
LmicEu868 LMIC {radio, OSS};

OsJob sendjob{OSS};

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
                LoraPacket.sensorContent.gpsunix = 0;
                LoraPacket.sensorContent.pm25 = 0;
                LoraPacket.sensorContent.lat = 0;
                LoraPacket.sensorContent.lng = 0;
                
                // Signal to the main loop that we have sent the message
                sentFlag = true;
                // reset send state and allow other processes to use packet
                sendFlag = false;

                xSemaphoreGive(packetSemaphore);
            }
        }
    vTaskDelay(1000/portTICK_PERIOD_MS);//Give other tasks a chance to run on the processor
    }
}

void ttnHandling(void * param){
    SPI.begin();
    os_init();
    LMIC.init();
    LMIC.reset();
    LMIC.setEventCallBack(onEvent);
    SetupLmicKey<appEui, devEui, appKey>::setup(LMIC);
    LMIC.setClockError(MAX_CLOCK_ERROR * 3 / 100);

    LMIC.startJoining();

    for(;;){
        OSS.runloopOnce();
        vTaskDelay(1);//Give other tasks a chance to run on the processor
    }
    
}