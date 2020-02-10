 
#include <Arduino.h>
#include <SPI.h>

#include <hal/hal_io.h>
#include <hal/print_debug.h>
#include <lmic.h>
#include "loraTransmission.hpp"
#include "message.hpp"
#include "globals.hpp"
#include "keyHandling.hpp"

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
        case EventType::JOIN_FAILED:
            state=LORA_FAILED;
            break;
        case EventType::JOINED:
            Serial.println(F("EventType::JOINED"));
            break;
        case EventType::JOINING:
            Serial.println(F("EventType::JOINING"));
            break;
        case EventType::TXCOMPLETE:
            Serial.println(F("EventType::TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.getTxRxFlags().test(TxRxStatus::ACK)) {
                PRINT_DEBUG(1, F("Received ack"));
                state = SLEEP;
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

void LoraSend(){
  if (LMIC.getOpMode().test(OpState::TXRXPEND)) {
    PRINT_DEBUG(1, F("OpState::TXRXPEND, not sending"));
  } else {
    for(int i = 0; i < PACKET_SIZE; i++){
        Serial.printf("%02X", LoraPacket.packetBytes[i]);
    }
    Serial.println();

    //LMIC.setDrTx(SF12);

    //Set the packet as the next thing to be transmitted
    LMIC.setTxData2(2, LoraPacket.packetBytes, PACKET_SIZE, 0);

    // reset packet
    LoraPacket.sensorContent.pm25 = -1;
    LoraPacket.sensorContent.lat = GPS_NULL;
    LoraPacket.sensorContent.lng = GPS_NULL;
    
    Serial.println("Sent Packet");
  }
}

void loraInit(){
    SPI.begin(5,19,27,18);
    os_init();
    LMIC.init();
    LMIC.reset();
    LMIC.setEventCallBack(onEvent);
    LMIC.setArtEuiCallback(getappEui);
    LMIC.setDevKey(getappKey());
    LMIC.setDevEuiCallback(getdevEui);


    LMIC.setClockError(MAX_CLOCK_ERROR*(30 / 100));
    LMIC.setAntennaPowerAdjustment(-14);
    sendjob.setCallbackRunnable(LoraSend);
}

void loraLoop(){
    //Doing the rest of the loop takes too long, lets wait here
    while(state == LORA_SEND){
        OsDeltaTime to_wait = OSS.runloopOnce();
        //Due to timings we only want to free up the processor
        //to other tasks if the time we have to wait is larger than 10 seconds
        if(to_wait > OsDeltaTime(6)){
            delay(to_wait.to_ms());
        }
    }
}

/* void ttnHandling(void * param){
    SPI.begin(5,19,27,18);
    os_init();
    LMIC.init();
    LMIC.reset();
    LMIC.setEventCallBack(onEvent);
    LMIC.setArtEuiCallback(getappEui);
    LMIC.setDevKey(getappKey());
    LMIC.setDevEuiCallback(getdevEui);


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
    
} */