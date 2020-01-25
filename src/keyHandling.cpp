#include <Arduino.h>
#include "globals.hpp"
#include <lmic.h>

constexpr char const appEui[] = "70B3D57ED00259BF";


// This code is from the lmicpp source for their key handler
uint8_t HexCharToInt(char const char1) {

  return (char1 >= '0' && char1 <= '9')
             ? char1 - '0'
             : (char1 >= 'A' && char1 <= 'F')
                   ? char1 - 'A' + 0x0A
                   : (char1 >= 'a' && char1 <= 'f') ? char1 - 'a' + 0x0A : 0;
}
uint8_t HexCharsToInt(char const char1, char const char2) {

  return (HexCharToInt(char1) * 0x10) + HexCharToInt(char2);
}

void getappEui(uint8_t *buf) {
    uint8_t appeui[EUIIntLen];
    int strLen = devEUILen;
    for(int i = 0; i < EUIIntLen; i++){
        appeui[i] = HexCharsToInt(appEui[strLen-1], appEui[strLen]);
        strLen-=2;
    }
    memcpy_P(buf, appeui, EUIIntLen);
}

void getdevEui(uint8_t *buf) {
    String devui = preferences.getString("DEVEUI");
    uint8_t deveui[EUIIntLen];
    unsigned int strLen = devui.length();
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
    for(int i = 0; i < EUIIntLen; i++){
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

    int deveuiNotSet = devui.compareTo("NOT SET");
    int keyNotSet = key.compareTo("NOT SET");

    return (!deveuiNotSet && !keyNotSet);


}