/*
Function: startTimerDeepSleep
Discription: puts board perifrals into a low power state and triggers
the ESP32's deep sleep
Param: int sleepMs, the ammount of time to sleep
*/
void startTimerDeepSleep(int sleepMs)
{
    //TODO: OLED Power Off
    //TODO: Unmount filesystem
    //TODO: Trigger Deepsleep of board

}

/*
Function: isCharging
Discription: attempts to detect if the board is charging
Returns: Boll, the are we charging

note: For testing purposes while detection isnt implemented it always returns true
*/
bool isCharging(int sleepMs)
{
    //TODO: See if the battery management chip is connected to a GPIO PIN
    //TODO: Investigate other methods of detecting if we are connected to USB or Battery
    // Possible leads: Voltage?
    return true;
}

float getBatVoltage(){

}

int getBatPercent(){
    
}