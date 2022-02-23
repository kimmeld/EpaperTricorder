/*
 * Base sensor class implementation
 */

#include "SensorBase.h"
#include <FreeRTOS.h>
 
SensorBase::SensorBase() {
  NewData = false;
  NewLogData = false;
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

void SensorBase::Log(SDFile *log) {
  return;
}

DynamicJsonDocument SensorBase::GetLog() {
  DynamicJsonDocument logent(128);
  logent["error"] = "Not implemented";
  return logent;
}