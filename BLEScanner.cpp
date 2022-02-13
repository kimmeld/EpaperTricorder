/*
    Wifi Scanner sensor


*/
#include "BLEScanner.h"

BLEScanner::BLEScanner() {
  NewData = false;
  Count = -1;
  BLEDevice::init("");
  sem = xSemaphoreCreateMutex();

  xTaskCreate(
    BLEScanner::BLEScannerTask,
    "BLEScanner",
    32768,
    (void *)this,
    1,
    &scannerTask
  );
}

bool BLEScanner::TryLock() {
  return xSemaphoreTake(sem, 1);
}

void BLEScanner::Unlock() {
  xSemaphoreGive(sem);
}

void BLEScanner::BLEScannerTask(void * parameter)
{
  BLEScanner* scan = (BLEScanner*)parameter;

  BLEScan *pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(0x50);
  pBLEScan->setWindow(0x30);
  for (;;)
  {
    if (xSemaphoreTake(scan->sem, portMAX_DELAY)) {
      scan->FoundDevices = pBLEScan->start(5);
      scan->Count = scan->FoundDevices.getCount();
      scan->NewData = true;
      scan->Unlock();
    }
    // Wait a bit before scanning again
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}
