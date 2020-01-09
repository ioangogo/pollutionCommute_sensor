#include <Arduino.h>
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
    esp_deep_sleep_start();
}