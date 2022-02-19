/*
    Environmental sensor

*/
#include "Environment.h"
#include <Wire.h>

EnvironmentSensor::EnvironmentSensor()
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

    bmp280 = new Adafruit_BMP280();
    if (bmp280->begin(0x76))
    {
        Serial.println("BMP280 ready");
    }
    else
    {
        Serial.println("BMP280 fail");
    }

    xTaskCreate(
        EnvironmentSensor::CO2SensorTask,
        "EnvironmentSensor",
        2048,
        (void *)this,
        1,
        &SensorTask);
}

void EnvironmentSensor::CO2SensorTask(void *parameter)
{
    EnvironmentSensor *sensor = (EnvironmentSensor *)parameter;
    for (;;)
    {
        // CCS811
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

        // BMP280
        if (sensor->Lock())
        {
            for (int x = 0; x < 199; x++)
            {
                sensor->temp[x] = sensor->temp[x + 1];
                sensor->pres[x] = sensor->pres[x + 1];
            }
            sensor->temp[199] = sensor->bmp280->readTemperature();
            sensor->pres[199] = sensor->bmp280->readPressure();
            sensor->NewData = true;
            sensor->Unlock();
        }
        Serial.print("BMP280 temp: ");
        Serial.print(sensor->bmp280->readTemperature());
        Serial.print(" pressure: ");
        Serial.print(sensor->bmp280->readPressure());
        Serial.println();

        // Wait a bit before scanning again
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}