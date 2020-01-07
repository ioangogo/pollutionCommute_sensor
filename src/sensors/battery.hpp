#include "sensor.hpp"

class BatterySensor : Sensor
{
    public:
    bool CollectData(CayenneLPP *msg);
};