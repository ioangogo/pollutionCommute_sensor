#include <CayenneLPP.h>

class Sensor{
    public:
     bool CollectData(CayenneLPP *msg); // a function required of all sensors to perform data collection and add it to a message
};