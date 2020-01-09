#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
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

void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            {
              u4_t netid = 0;
              devaddr_t devaddr = 0;
              u1_t nwkKey[16];
              u1_t artKey[16];
              LMIC_getSessionKeys(&netid, &devaddr, nwkKey, artKey);
              Serial.print("netid: ");
              Serial.println(netid, DEC);
              Serial.print("devaddr: ");
              Serial.println(devaddr, HEX);
              Serial.print("AppSKey: ");
              for (size_t i=0; i<sizeof(artKey); ++i) {
                if (i != 0)
                  Serial.print("-");
                printHex2(artKey[i]);
              }
              Serial.println("");
              Serial.print("NwkSKey: ");
              for (size_t i=0; i<sizeof(nwkKey); ++i) {
                      if (i != 0)
                              Serial.print("-");
                      printHex2(nwkKey[i]);
              }
              Serial.println();
            }
            // Disable link check validation (automatically enabled
            // during join, but because slow data rates change max TX
	    // size, we don't use it in this example.
            LMIC_setLinkCheckMode(0);
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_RFU1:
        ||     Serial.println(F("EV_RFU1"));
        ||     break;
        */
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
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
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
        /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_SCAN_FOUND:
        ||    Serial.println(F("EV_SCAN_FOUND"));
        ||    break;
        */
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
                /*byte buf[PACKET_SIZE];
                memcpy(buf, LoraPacket.packetBytes, PACKET_SIZE);

                // prepare to transmit buffer
                LMIC_setDrTxpow(DR_SF11, 1);
                LMIC_setTxData2(1, buf, PACKET_SIZE, 0);

                // reset packet
                LoraPacket.sensorContent.gpsunix = 0;
                LoraPacket.sensorContent.pm25 = 0;
                LoraPacket.sensorContent.lat = 0;
                LoraPacket.sensorContent.lng = 0;
                
                // Signal to the main loop that we have sent the message
                sent = true;*/
                // reset send state and allow other processes to use packet
                send = false;

                Serial.printf("%"PRId64" %d %d %d \n", LoraPacket.sensorContent.gpsunix, LoraPacket.sensorContent.pm25, LoraPacket.sensorContent.lat, LoraPacket.sensorContent.lng);
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
    LMIC_startJoining();

    for(;;){
        os_runloop_once();
        vTaskDelay(1);//Give other tasks a chance to run on the processor
    }
    
}