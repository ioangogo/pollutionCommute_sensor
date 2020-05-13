#ifndef _hexUtil_h_
#define _hexUtil_h_

#include <Arduino.h>

// This code is adapted from the lmicpp source for their key handler
// If the keyhandler fitted my needs i would be using it anyway, so i split it out into a seprate h file
// this code is EPL 0.1 licenced
// Find original source under a Eclipse Public License v1.0 here: https://github.com/ngraziano/LMICPP-Arduino/blob/master/src/keyhandler.h

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

#endif