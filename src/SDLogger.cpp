#include "SDLogger.h"
#include <ArduinoJson.h>
#include <string>

SDLogger::SDLogger() : SDSPI(HSPI)
{
    active = false;
    cardSize = 0;
    cardUsed = 0;
    filename = "N/A";
    SDSPI.begin(14, 2, 15, 13); // Pinout comes from schematic diagram, why did they put MISO on IO2 instead of IO12?

    xTaskCreate(
        SDLogger::SDLoggerTask,
        "SDLogger",
        8192,
        (void *)this,
        1,
        &SensorTask);
}

// Activate logging
void SDLogger::begin()
{
    if (Lock())
    {
        bool canActivate = true;
        Serial.println("Card Mount start");
        if (!SD.begin(13, SDSPI))
        {
            Serial.println("Card Mount Failed");
            canActivate = false;
        }
        Serial.println("Card Mount done");
        cardType = SD.cardType();

        if (cardType == CARD_NONE)
        {
            Serial.println("No SD card attached");
            canActivate = false;
        }

        if (canActivate)
        {
            cardSize = SD.cardSize();

            char fname[20];
            for (int i = 0; i < 1000; i++)
            {
                snprintf(fname, sizeof(fname), "/log_%d.json", i);
                if (!SD.exists(fname))
                {
                    break;
                }
            }
            filename = fname;
            logFile = SD.open(fname, "a");
            // logFile.println("=== BEGIN ===");

            DynamicJsonDocument logent(128);
            logent["tick"] = millis();
            logent["log"] = "BEGIN";
            serializeJson(logent, logFile);
            logFile.println();
            active = true;
            logStartTime = millis();
        }
        Unlock();
    }
}

uint64_t SDLogger::GetLogRuntime()
{
    if (active)
    {
        return millis() - logStartTime;
    }
    return 0;
}

// Stop logging to the SD card
void SDLogger::end()
{
    if (Lock())
    {
        active = false;
        DynamicJsonDocument logent(128);
        logent["tick"] = millis();
        logent["log"] = "END";
        serializeJson(logent, logFile);
        logFile.println();
        logFile.close();
        SD.end();

        cardType = CARD_NONE;
        cardSize = 0;
        cardUsed = 0;
        filename = "N/A";
        logStartTime = -1;
        Unlock();
    }
}

void SDLogger::AddSensor(SensorBase *sens)
{
    sensors.push_back(sens);
}

void SDLogger::SDLoggerTask(void *param)
{
    SDLogger *logger = (SDLogger *)param;
    for (;;)
    {
        if (logger->active)
        {
            if (logger->TryLock())
            {
                // For each sensor with new data, log the timestamp and its data
                for (SensorBase *s : logger->sensors)
                {
                    if (s->NewLogData)
                    {
                        DynamicJsonDocument logent(512);
                        logent["tick"] = millis();
                        logent["sensor_log"] = s->GetLog();
                        serializeJson(logent, logger->logFile);
                        logger->logFile.println();
                        // logger->logFile.print("Tick:");
                        // logger->logFile.println(millis());
                        // s->Log(&logger->logFile);
                    }
                }

                logger->logFile.flush();
                logger->cardUsed = SD.usedBytes();
                logger->Unlock();
            }
        }

        vTaskDelay(250 / portTICK_PERIOD_MS);
    }
}