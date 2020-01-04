#include "sensor.hpp"

class BatterySensor : Sensor
{
    public:
    bool BatterySensor::CollectData(CayenneLPP *msg);
};