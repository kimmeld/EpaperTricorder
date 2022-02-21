#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <SparkFunCCS811.h>
#include <Adafruit_BMP280.h>
#include <Arduino.h>
#include "SensorBase.h"


class EnvironmentSensor: public SensorBase
{
  public:
    EnvironmentSensor();
    
    CCS811 *ccs811;
    uint16_t ccs811baseline;
    
    Adafruit_BMP280 *bmp280;

    int16_t co2[200] = {-1};
    float temp[200] = {0};
    float pres[200] = {0};
    uint32_t sample_count = 0;
    //int16_t tvoc[100] = {-1};

    static void CO2SensorTask(void * param);

    virtual void Log(SDFile* log);
};



#endif
