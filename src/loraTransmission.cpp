#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include "loraTransmission.hpp"
#include "message.hpp"
#include "globals.hpp"


// This EUI must be in little-endian format, so least-significant-byte
// first. When copying an EUI from ttnctl output, this means to reverse
// the bytes. For TTN issued EUIs the last bytes should be 0xD5, 0xB3,
// 0x70.
static const u1_t PROGMEM APPEUI[8]={ 0xCB, 0x85, 0x02, 0xD0, 0x7E, 0xD5, 0xB3, 0x70 };
void os_getArtEui (u1_t* buf) { memcpy_P(buf, APPEUI, 8);}

// This should also be in little endian format, see above.
static const u1_t PROGMEM DEVEUI[8]={ 0x35, 0x4A, 0xF3, 0x44, 0xB8, 0x59, 0x65, 0x00 };
void os_getDevEui (u1_t* buf) { memcpy_P(buf, DEVEUI, 8);}

// This key should be in big endian format (or, since it is not really a
// number but a block of memory, endianness does not really apply). In
// practice, a key taken from ttnctl can be copied as-is.
static const u1_t PROGMEM APPKEY[16] = { 0xF7, 0x20, 0xFB, 0x05, 0x43, 0x0C, 0x0A, 0xC3, 0xAC, 0x83, 0x77, 0xB8, 0x8C, 0x64, 0xE5, 0xE7 };
void os_getDevKey (u1_t* buf) {  memcpy_P(buf, APPKEY, 16);}

osjob_t sendJob;

const lmic_pinmap lmic_pins = {
    .nss = SS,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = RST_LoRa,
    .dio = {DIO0, DIO1, DIO2},
};

// Code from example code of the arduino-LMIC libary examples
// Find original source here: https://github.com/mcci-catena/arduino-lmic/blob/master/examples/ttn-otaa/ttn-otaa.ino
void printHex2(unsigned v) {
    v &= 0xff;
    if (v < 16)
        Serial.print('0');
    Serial.print(v, HEX);
}

const char *event_names[] = {LMIC_EVENT_NAME_TABLE__INIT};

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    Serial.println(event_names[ev]);

    switch(ev) {
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            if (LMIC.txrxFlags & TXRX_ACK)
              Serial.println(F("Received ack"));
              if(sent){
                  // assuming if sent was true then we had sent our packet and we can now sleep
                  sleepFlag = true; 
              }
            if (LMIC.dataLen) {
              Serial.print(F("Received "));
              Serial.print(LMIC.dataLen);
              Serial.println(F(" bytes of payload"));
            }
            break;
        case EV_TXSTART:
            Serial.println(F("EV_TXSTART"));
            break;
        default:
            Serial.print(F("Unknown event: "));
            Serial.println((unsigned) ev);
            break;
    }
}
// End of code from external source

void LoraSend(void * param){
    for(;;){
        if(send){
            if(xSemaphoreTake(packetSemaphore, portMAX_DELAY) == pdTRUE){
                // copy message buffer for packet
                byte buf[PACKET_SIZE];
                memcpy(buf, LoraPacket.packetBytes, PACKET_SIZE);

                for(int i = 0; i < PACKET_SIZE; i++){
                    Serial.printf("%02X", buf[i]);
                }
                Serial.println();

                // prepare to transmit buffer
                LMIC_setDrTxpow(DR_SF11, 1);

                //Set the packet as the next thing to be transmitted
                LMIC_setTxData2(1, buf, PACKET_SIZE, 0);

                // reset packet
                LoraPacket.sensorContent.gpsunix = 0;
                LoraPacket.sensorContent.pm25 = 0;
                LoraPacket.sensorContent.lat = 0;
                LoraPacket.sensorContent.lng = 0;
                
                // Signal to the main loop that we have sent the message
                sent = true;
                // reset send state and allow other processes to use packet
                send = false;

                xSemaphoreGive(packetSemaphore);
                sleepFlag = true;
            }
        }
    vTaskDelay(1000*portTICK_PERIOD_MS);//Give other tasks a chance to run on the processor
    }
}

void ttnHandling(void * param){
    os_init();
    LMIC_reset();
    LMIC_setDrTxpow(DR_SF7, 1);
    LMIC_setClockError(MAX_CLOCK_ERROR * 1 / 100); // Allow for 1% clock error
    LMIC_startJoining();

    /*OTAHolder keys;
    size_t err = preferences.getBytes("otaKeys", keys.KeyBytes, OTA_SIZE);
    if(err == 0){
        LMIC_startJoining();
    }else{
        // If we have keys from a previous session just reuse them
        LMIC_setSession(keys.otakeys.netid, keys.otakeys.devaddr, keys.otakeys.nwkKey, keys.otakeys.artKey);
    }*/

    for(;;){
        os_runloop_once();
        vTaskDelay(1);//Give other tasks a chance to run on the processor
    }
    
}