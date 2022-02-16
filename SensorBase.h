/*
    Base sensor class

    This is a base class that should be used for all sensor objects.
*/
#ifndef SENSORBASE_H
#define SENSORBASE_H

#include <RTOS.h>

class SensorBase
{
  public:
    SensorBase();

    bool Enable;

    bool NewData;

    /* Methods for locking/unlocking the internal data */
    virtual bool TryLock();
    virtual bool Lock();
    virtual void Unlock();

  protected:
    SemaphoreHandle_t sem;    // Semaphore used to protect access to data
    TaskHandle_t SensorTask;  // Most sensors will need a background task, so here's a handle to use
};



#endif
