// Device Commands from https://cdn-shop.adafruit.com/datasheets/PMTK_A11.pdf
#define SLEEP_CMD "$PMTK161,0*28\r\n" // command to get the device into a low power state
#define WAKE_CMD "\n" // Going to use this to wake the device up, although any bytes will wake the device up

void doGPSTask();
void initGPS();
void deepsleepSleep();