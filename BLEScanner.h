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

class BLEScanner
{
  public:
    BLEScanner();

    bool Enable;

    bool NewData;
    int Count;
    std::vector<String> FoundDevices;

    bool TryLock();
    void Unlock();

    static void BLEScannerTask(void * param);
  protected:
    SemaphoreHandle_t sem;
  private:
    TaskHandle_t scannerTask;
};



#endif
