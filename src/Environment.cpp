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

void EnvironmentSensor::Log(SDFile *log)
{
    if (!NewLogData)
    {
        return;
    }
    if (Lock())
    {
        log->print("Env: ");
        log->print("Sample:");
        log->print(sample_count);
        log->print(" CO2:");
        log->print(co2[199]);
        log->print("ppm Temp:");
        log->print(temp[199]);
        log->print("C Pres:");
        log->print(pres[199]);
        log->print("pa CCS811Baseline:");
        log->print(ccs811baseline);
        log->println();
        NewLogData = false;
        Unlock();
    }
}

DynamicJsonDocument EnvironmentSensor::GetLog()
{
    DynamicJsonDocument logent(1024);
    if (Lock())
    {
        logent["source"] = "Environment";
        logent["CO2"] = co2[199];
        logent["TVOC"] = tvoc[199];
        logent["Temp"] = temp[199];
        logent["Pres"] = pres[199];
        logent["ccs811baseline"] = ccs811baseline;
        logent["count"] = sample_count;
        NewLogData = false;
        Unlock();
    }
    else
    {
        logent["Error"] = "Could not acquire lock";
    }
    return logent;
}

void EnvironmentSensor::CO2SensorTask(void *parameter)
{
    EnvironmentSensor *sensor = (EnvironmentSensor *)parameter;
    for (;;)
    {
        // CCS811
        if (sensor->ccs811->dataAvailable())
        {
            sensor->sample_count++;
            sensor->ccs811->readAlgorithmResults();
            sensor->ccs811baseline = sensor->ccs811->getBaseline();

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
                    sensor->tvoc[x] = sensor->tvoc[x + 1];
                }
                sensor->co2[199] = sensor->ccs811->getCO2();
                sensor->tvoc[99] = sensor->ccs811->getTVOC();
                sensor->NewData = true;
                sensor->NewLogData = true;
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

        // Feed temperature into ccs811 to get a more accurate eCO2 reading
        // NOTE:  The relative humidity is obviously faked since we don't have a
        //        sensor for that.  I should have ordered a BME280, not a BMP280.
        sensor->ccs811->setEnvironmentalData(20.0, sensor->temp[199]);

        // Wait a bit before scanning again
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}