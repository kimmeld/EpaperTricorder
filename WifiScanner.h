/*
    Wifi Scanner sensor


*/
#ifndef WIFISCANNER_H
#define WIFISCANNER_H

#include <WiFi.h>

class WifiScanner
{
  public:
    WifiScanner();

    bool Enable;

    bool NewData;
    int Count;

    bool TryLock();
    void Unlock();

    static void WifiScannerTask(void * param);
  protected:
    SemaphoreHandle_t sem;
  private:
    TaskHandle_t scannerTask;
};



#endif
