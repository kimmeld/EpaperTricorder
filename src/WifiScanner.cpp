/*
    Wifi Scanner sensor


*/
#include "WifiScanner.h"

WifiScanner::WifiScanner()
{
  Count = -1;
  Enable = true;

  xTaskCreate(
      WifiScanner::WifiScannerTask,
      "WifiScanner",
      2048,
      (void *)this,
      1,
      &SensorTask);
}

void WifiScanner::Log(SDFile *log)
{
  if (!NewLogData)
  {
    return;
  }
  if (Lock())
  {
    log->print("Wifi: NetCount:");
    log->println(Count);
    for (int n = 0; n < Count; n++)
    {
      log->print("Wifi: Network: ");
      log->print(n);
      log->printf("\t");
      log->print(WiFi.SSID(n));
      log->printf("\t");
      log->print(WiFi.RSSI(n));
      log->printf("\t\n");
    }
    log->println();
    NewLogData = false;
    Unlock();
  }
}

DynamicJsonDocument WifiScanner::GetLog()
{
  DynamicJsonDocument logent(2048);
  if (Lock())
  {    
    logent["source"] = "WifiScanner";
    logent["Count"] = Count;
    for (int n = 0; n < Count; n++)
    {
      logent["Networks"][n]["SSID"] = WiFi.SSID(n);
      logent["Networks"][n]["RSSI"] = WiFi.RSSI(n);
    }
    NewLogData = false;
    Unlock();
  } else {
    logent["Error"] = "Could not acquire lock";
  }
  return logent;
}

void WifiScanner::WifiScannerTask(void *parameter)
{
  WifiScanner *scan = (WifiScanner *)parameter;
  for (;;)
  {
    // WiFi.scanNetworks will return the number of networks found
    if (scan->Lock())
    {
      if (scan->Enable)
      {
        int n = WiFi.scanNetworks();
        scan->Count = n;
        scan->NewData = true;
        scan->NewLogData = true;
      }
      scan->Unlock();
    }
    // Wait a bit before scanning again
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}
