#include "SDLogger.h"

SDLogger::SDLogger() : SDSPI(HSPI)
{
}

void SDLogger::begin()
{
    // Initialize SD card
    // Pinout comes from schematic diagram, why did they put MISO on IO2 instead of IO12?
    SDSPI.begin(14, 2, 15, 13);
    Serial.println("Card Mount start");
    if (!SD.begin(13, SDSPI))
    {
        Serial.println("Card Mount Failed");
        return;
    }
    Serial.println("Card Mount done");
    uint8_t cardType = SD.cardType();

    if (cardType == CARD_NONE)
    {
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if (cardType == CARD_MMC)
    {
        Serial.println("MMC");
    }
    else if (cardType == CARD_SD)
    {
        Serial.println("SDSC");
    }
    else if (cardType == CARD_SDHC)
    {
        Serial.println("SDHC");
    }
    else
    {
        Serial.println("UNKNOWN");
    }

    logFile = SD.open("/log.txt", "a");
    logFile.println("=== BEGIN ===");

    // If all of the above succeeded, create the logger task
    // Otherwise, we don't need to do any logging (obviously)
    xTaskCreate(
        SDLogger::SDLoggerTask,
        "SDLogger",
        8192,
        (void *)this,
        1,
        &SensorTask);
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
        logger->logFile.print("Tick:");
        logger->logFile.println(millis());

        // Tell each configured sensor to go log itself
        for (SensorBase *s : logger->sensors)
        {
            s->Log(&logger->logFile);
        }

        logger->logFile.flush();
        Serial.println("Log");
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}