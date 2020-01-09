#include <Arduino.h>
#include "globals.hpp"

#define SLEEP_MINS 1

/*
Function: startTimerDeepSleep
Discription: puts board perifrals into a low power state and triggers
the ESP32's deep sleep
Param: int sleepMs, the ammount of time to sleep
*/
void startTimerDeepSleep()
{
    //TODO: OLED Power Off
    //TODO: Unmount filesystem
    //TODO: Trigger Deepsleep of board
    Serial.printf("Sleeping for %d Mins\n", SLEEP_MINS);
    esp_deep_sleep(1*MIN_TO_MS);//Set Deepsleep timer for when we will go back to sleep
    
}