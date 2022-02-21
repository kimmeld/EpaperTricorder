#include "SDLogger.h"

SDLogger::SDLogger() : SDSPI(HSPI)
{
    active = false;
    SDSPI.begin(14, 2, 15, 13);  // Pinout comes from schematic diagram, why did they put MISO on IO2 instead of IO12?

    xTaskCreate(
        SDLogger::SDLoggerTask,
        "SDLogger",
        2048,
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
            logFile = SD.open("/log.txt", "a");
            logFile.println("=== BEGIN ===");
            active = true;
        }
        Unlock();
    }
}

// Stop logging to the SD card
void SDLogger::end()
{
    if (Lock())
    {
        active = false;
        logFile.println("=== END ===");
        logFile.close();
        SD.end();

        cardType = CARD_NONE;
        cardSize = 0;
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
                        logger->logFile.print("Tick:");
                        logger->logFile.println(millis());
                        s->Log(&logger->logFile);
                    }
                }

                logger->logFile.flush();
                logger->Unlock();
            }
        }

        vTaskDelay(250 / portTICK_PERIOD_MS);
    }
}