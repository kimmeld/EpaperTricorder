/*
    CO2 sensor

*/
#include "CO2.h"
#include <Wire.h>

CO2Sensor::CO2Sensor()
{
    ccs811 = new CCS811(0x5A);
    if (ccs811->begin())
    {
        Serial.println("Sensor setup complete");
    }
    else
    {
        Serial.println("Sensor setup FAIL");
    }

    xTaskCreate(
        CO2Sensor::CO2SensorTask,
        "CO2Sensor",
        2048,
        (void *)this,
        1,
        &SensorTask);
}

void CO2Sensor::CO2SensorTask(void *parameter)
{
    CO2Sensor *sensor = (CO2Sensor *)parameter;
    for (;;)
    {
        if (sensor->ccs811->dataAvailable())
        {
            sensor->ccs811->readAlgorithmResults();

            Serial.print("CO2: ");
            Serial.print(sensor->ccs811->getCO2());
            Serial.print("ppm, TVOC: ");
            Serial.print(sensor->ccs811->getTVOC());
            Serial.print(" Err: ");
            Serial.println(sensor->ccs811->getErrorRegister());

            if (sensor->Lock())
            {
                for (int x = 0; x < 199; x++)
                {
                    sensor->co2[x] = sensor->co2[x + 1];
                    // sensor->tvoc[x] = sensor->tvoc[x + 1];
                }
                sensor->co2[199] = sensor->ccs811->getCO2();
                // sensor->tvoc[99] = sensor->sensor->getTVOC();
                sensor->NewData = true;
                sensor->Unlock();
            }
        }
        else
        {
            Serial.println("No data");
        }

        // Wait a bit before scanning again
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}