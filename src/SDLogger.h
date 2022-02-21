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
    void end();

    void AddSensor(SensorBase *sens);

    static void SDLoggerTask(void *param);

    bool active;

    uint8_t cardType;
    uint64_t cardSize;
private:
    SPIClass SDSPI;
    SDFile logFile;
    std::vector<SensorBase*> sensors;
};

#endif