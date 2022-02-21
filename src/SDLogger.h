#ifndef SDLOGGER_H_
#define SDLOGGER_H_

#include "SensorBase.h"
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <vector>

class SDLogger : public SensorBase
{
public:
    SDLogger();
    void begin();

    void AddSensor(SensorBase *sens);

    static void SDLoggerTask(void *param);

private:
    SPIClass SDSPI;
    SDFile logFile;
    std::vector<SensorBase*> sensors;
};

#endif