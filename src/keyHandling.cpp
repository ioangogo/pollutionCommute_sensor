#include <Arduino.h>
#include "globals.hpp"
#include <lmic.h>
#include "hexUtil.h"

constexpr char const appEui[] = "70B3D57ED00259BF";

void debugOutput(uint8_t *buf, int Len){
    for(int i = 0; i < Len; i++){
        Serial.printf("%02X ", buf[i]);
    }
}

void getappEui(uint8_t *buf) {
    uint8_t appui[EUIIntLen];
    int strLen = devEUILen-2;
    for(int i = 0; i < EUIIntLen; i++){
        appui[i] = HexCharsToInt(appEui[strLen-1], appEui[strLen]);
        strLen-=2;
    }
    memcpy_P(buf, appui, EUIIntLen);
}

void getdevEui(uint8_t *buf) {
    String devui = preferences.getString("DEVEUI");
    uint8_t deveui[EUIIntLen];
    unsigned int strLen = devui.length()-1;
    for(int i = 0; i < EUIIntLen; i++){
        deveui[i] = HexCharsToInt(devui.charAt(strLen-1), devui.charAt(strLen));
        strLen-=2;
    }
    memcpy_P(buf, deveui, EUIIntLen);
}

AesKey getappKey() {
    String key = preferences.getString("APPKEY");
    uint8_t appkey[appKeyIntLen];
    int strLen = 0;
    for(int i = 0; i < appKeyIntLen; i++){
        appkey[i] = HexCharsToInt(key.charAt(strLen), key.charAt(strLen+1));
        strLen+=2;
    }
    AesKey lmickey;
    memcpy_P(lmickey.data, appkey, appKeyIntLen);
    return lmickey;
}

bool KeysExistInPref(){
    String devui = preferences.getString("DEVEUI", "NOT SET");
    String key = preferences.getString("APPKEY", "NOT SET");

    bool deveuiNotSet = devui == "NOT SET";
    bool keyNotSet = key == "NOT SET";

    return !(deveuiNotSet && keyNotSet);


}
