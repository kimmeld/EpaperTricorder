/*
    Wifi Scanner sensor


*/
#include "BLEScanner.h"

BLEScanner::BLEScanner() {
  NewData = false;
  Enable = false;
  Count = -1;
  sem = xSemaphoreCreateMutex();

  Serial.printf("BLE Scanner Task constructor\n");

  xTaskCreate(
    BLEScanner::BLEScannerTask,
    "BLEScanner",
    16384,
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

  class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    public:
      BLEScanner* scan;
      MyAdvertisedDeviceCallbacks(BLEScanner *_scan) {
        scan = _scan;
      }
      void onResult(BLEAdvertisedDevice advertisedDevice) {
        Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
        if (scan->FoundDevices.size() < 30) {
          std::stringstream ss;
          ss << advertisedDevice.getAddress().toString() << ": " << advertisedDevice.getName() << " " << (int)advertisedDevice.getTXPower();
          scan->FoundDevices.push_back(ss.str().c_str());
        }
      }
  };


  for (;;)
  {
    if (xSemaphoreTake(scan->sem, portMAX_DELAY)) {
      if (scan->Enable) {
        BLEDevice::init("");
        BLEScan *pBLEScan = BLEDevice::getScan(); //create new scan
        pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(scan));
        pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
        pBLEScan->setInterval(0x50);
        pBLEScan->setWindow(0x30);

        // Scan for five seconds then stop
        scan->FoundDevices.clear();
        pBLEScan->start(5);
        pBLEScan->stop();
        BLEDevice::deinit(false);

        scan->NewData = true;
      }
      scan->Unlock();
    }
    // Wait a bit before scanning again
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}
