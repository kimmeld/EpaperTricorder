
#include "WifiScanner.h"

WifiScanner::WifiScanner() {
  NewData = false;
  Count = -1;

  sem = xSemaphoreCreateMutex();
  
  xTaskCreate(
    WifiScanner::WifiScannerTask,
    "WifiScanner",
    1000,
    (void *)this,
    1,
    &scannerTask
  );
}

bool WifiScanner::TryLock() {
  return xSemaphoreTake(sem, 1);
}

void WifiScanner::Unlock() {
  xSemaphoreGive(sem);
}

void WifiScanner::WifiScannerTask(void * parameter)
{
  WifiScanner* scan = (WifiScanner*)parameter;
  for (;;)
  {
    // WiFi.scanNetworks will return the number of networks found
    if (xSemaphoreTake(scan->sem, portMAX_DELAY)) {
      int n = WiFi.scanNetworks();
      scan->Count = n;
      scan->NewData = true;
      xSemaphoreGive(scan->sem);
    }
    // Wait a bit before scanning again
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}
