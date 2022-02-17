/*
 * Base sensor class implementation
 */

#include "SensorBase.h"
#include <FreeRTOS.h>
 
SensorBase::SensorBase() {
  NewData = false;
  Enable = false;
  sem = xSemaphoreCreateMutex();
}

bool SensorBase::TryLock() {
  return xSemaphoreTake(sem, 1);
}

bool SensorBase::Lock() {
  return xSemaphoreTake(sem, portMAX_DELAY);
}

void SensorBase::Unlock() {
  xSemaphoreGive(sem);
}
