/*
    Wifi Scanner sensor


*/
#include "BLEScanner.h"

BLEScanner::BLEScanner() {
  Count = -1;

  Serial.printf("BLE Scanner Task constructor\n");

  xTaskCreate(
    BLEScanner::BLEScannerTask,
    "BLEScanner",
    16384,
    (void *)this,
    1,
    &SensorTask
  );
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

  BLEDevice::init("");
  for (;;)
  {
    if (scan->Lock()) {
      if (scan->Enable) {

        BLEScan *pBLEScan = BLEDevice::getScan(); //create new scan
        pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(scan));
        pBLEScan->setActiveScan(false); // Passive scan doesn't seem to interfere with the Wifi scanner
//        pBLEScan->setInterval(0x50);
//        pBLEScan->setWindow(0x30);

        // Scan for five seconds then stop
        scan->FoundDevices.clear();
        pBLEScan->start(5);
        pBLEScan->stop();
        //BLEDevice::deinit(true);

        scan->NewData = true;
      }
      scan->Unlock();
    }
    // Wait a bit before scanning again
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}
