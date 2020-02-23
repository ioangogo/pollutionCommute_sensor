#include <Arduino.h>
#include "globals.hpp"
#include "gps.hpp"
#include "dustSensor.hpp"

#define SLEEP_MINS 5

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
    sendFlag = false;
    sentFlag = false;
    
    esp_deep_sleep(SLEEP_MINS*MIN_TO_US);//Set Deepsleep timer for when we will go back to sleep
    
}