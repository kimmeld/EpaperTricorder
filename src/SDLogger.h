#ifndef SDLOGGER_H_
#define SDLOGGER_H_

#include "SensorBase.h"
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <vector>
#include <string>

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
    uint64_t cardUsed;
    std::string filename;

    uint64_t GetLogRuntime();
private:
    SPIClass SDSPI;
    SDFile logFile;
    std::vector<SensorBase*> sensors;
    uint64_t logStartTime;
};

#endif