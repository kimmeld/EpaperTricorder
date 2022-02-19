#ifndef CO2_H
#define CO2_H

#include <SparkFunCCS811.h>
#include <Arduino.h>
#include "SensorBase.h"


class CO2Sensor: public SensorBase
{
  public:
    CO2Sensor();
    CCS811 *sensor;

    int16_t co2[200] = {-1};
    //int16_t tvoc[100] = {-1};

    static void CO2SensorTask(void * param);
};



#endif
