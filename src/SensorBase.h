/*
    Base sensor class

    This is a base class that should be used for all sensor objects.
*/
#ifndef SENSORBASE_H
#define SENSORBASE_H

#include <FreeRTOS.h>
#include <SD.h>

class SensorBase
{
  public:
    SensorBase();

    bool Enable;

    bool NewData;
    bool NewLogData;

    /* Methods for locking/unlocking the internal data */
    virtual bool TryLock();
    virtual bool Lock();
    virtual void Unlock();

    /* Log relevant data to the log file */
    virtual void Log(SDFile *file);

  protected:
    SemaphoreHandle_t sem;    // Semaphore used to protect access to data
    TaskHandle_t SensorTask;  // Most sensors will need a background task, so here's a handle to use
};



#endif
