#include <Arduino.h>
#include "globals.hpp"
#include <lmic.h>

void getdevEui(uint8_t *buf);
AesKey getappKey();
void getappEui(uint8_t *buf);