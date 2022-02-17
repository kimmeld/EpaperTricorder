/*
    Wifi Scanner sensor


*/
#ifndef WIFISCANNER_H
#define WIFISCANNER_H

#include <WiFi.h>
#include "SensorBase.h"

class WifiScanner : public SensorBase
{
  public:
    WifiScanner();
    int Count;


    static void WifiScannerTask(void * param);
};



#endif