/*
    BLE Scanner sensor


*/
#ifndef BLESCANNER_H
#define BLESCANNER_H

#include <Arduino.h>
#include <vector>
#include <sstream>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#include "SensorBase.h"

class BLEScanner : public SensorBase
{
  public:
    BLEScanner();

    int Count;
    std::vector<String> FoundDevices;

    static void BLEScannerTask(void * param);
};



#endif
