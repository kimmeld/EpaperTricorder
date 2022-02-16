/*
    Wifi Scanner sensor


*/
#include "WifiScanner.h"

WifiScanner::WifiScanner() {
  Count = -1;

  xTaskCreate(
    WifiScanner::WifiScannerTask,
    "WifiScanner",
    1000,
    (void *)this,
    1,
    &SensorTask
  );
}

void WifiScanner::WifiScannerTask(void * parameter)
{
  WifiScanner* scan = (WifiScanner*)parameter;
  for (;;)
  {
    // WiFi.scanNetworks will return the number of networks found
    if (scan->Lock()) {
      if (scan->Enable) {
        int n = WiFi.scanNetworks();
        scan->Count = n;
        scan->NewData = true;
      }
      scan->Unlock();
    }
    // Wait a bit before scanning again
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}
