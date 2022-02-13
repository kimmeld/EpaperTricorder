#include "buttons.h"
#include "WifiScanner.h"
#include "AudioFFT.h"
#include "BLEScanner.h"

#include <GxEPD2_GFX.h>
#include <GxEPD2_7C.h>
#include <GxEPD2_EPD.h>
#include <GxEPD2_3C.h>
#include <GxEPD2_BW.h>
#include <GxEPD2.h>

GxEPD2_BW<GxEPD2_270, GxEPD2_270::HEIGHT> display(GxEPD2_270(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4));

// Buttons for user interaction
Buttons *btns;

// Wifi Scanner
WifiScanner *wifi;

// BLE Scanner
// BLEScanner *ble;

// FFT
AudioFFT *audioFFT;

void setup()
{
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  Serial.begin(115200);

  display.init();
  display.setRotation(1);
  ClearDisplay();
  display.hibernate();

  // Creat the various tasks we require
  btns = new Buttons();
  wifi = new WifiScanner();
  audioFFT = new AudioFFT();
  //ble = new BLEScanner();

}



// UI Loop
int uiMode = 0;         // which screen are we on
bool uiFirst = true;    // Is this the first time through this screen's loop?
void loop()
{
  // Go to the next screen
  if (btns->btn1SingleClick) {
    uiMode++;
    if (uiMode > 2) uiMode = 0;
    ClearDisplay();
    uiFirst = true;
  }

  // We've read the buttons
  btns->Reset();

  // Dispatch to the appropriate UI loop
  switch (uiMode) {
    case 0:
      loop_wifi(uiFirst);
      break;
    case 1:
      loop_fft(uiFirst);
      break;
    case 2:
      loop_ble(uiFirst);
      break;
    default:
      break;
  }
  uiFirst = false;
}

int16_t tbx, tby; uint16_t tbw, tbh;  // text metrics
void ClearDisplay() {
  display.fillRect(0, 0, display.width(), display.height(), GxEPD_WHITE);
  display.display(false);

  display.setFont(NULL);
  display.setTextColor(GxEPD_BLACK);
  tbh = 7;
}

// UI Code for Wifi Scanner
void loop_wifi(bool first) {
  if (first) {
    // Draw header
    display.setCursor(0, 0);
    display.print("Wifi Scanner");
    display.drawLine(0, tbh + 2, display.width(), tbh + 2, GxEPD_BLACK);
    display.display(true);
  }

  int y = tbh + 4;
  if (wifi->TryLock()) {
    if (wifi->NewData) {
      wifi->NewData = false;
      // Display number of networks
      display.fillRect(150, 0, display.width() - 150, tbh, GxEPD_WHITE);
      display.setCursor(150, 0);
      display.print(wifi->Count);
      display.print(" networks");

      // Display the list of networks
      display.fillRect(0, y, display.width(), display.height() - tbh, GxEPD_WHITE);
      for (int i = 0; i < wifi->Count; i++) {
        display.setCursor(0, y);
        display.print(WiFi.SSID(i));
        display.print(" ");
        display.print(WiFi.RSSI(i));
        y += tbh + 2;
      }
      // Update the display
      display.display(true);
      display.hibernate();
    }
    wifi->Unlock();
  }
}

void loop_fft(bool first) {
  if (first) {
    // Draw header
    display.setCursor(0, 0);
    display.print("FFT");
    display.drawLine(0, tbh + 2, display.width(), tbh + 2, GxEPD_BLACK);
    display.display(true);
  }

  if (audioFFT->TryLock()) {
    if (audioFFT->NewData) {
      // Display FFT results
      double fftMax = (double)(display.height() - (tbh + 4));
      display.fillRect(0, tbh + 4, display.width(), fftMax, GxEPD_WHITE);
      for (int i = 0; i < SAMPLE_BUFFER_SIZE / 2; i++) {
        int s = min(audioFFT->FFTResults[i], fftMax);
        display.drawLine(i, display.height() - s, i, display.height(), GxEPD_BLACK);
      }
      display.display(true);
      display.hibernate();
      audioFFT->NewData = false;
    }
    audioFFT->Unlock();
  }
}


void loop_ble(bool first) {
  if (first) {
    // Draw header
    display.setCursor(0, 0);
    display.print("BLE Scanner");
    display.drawLine(0, tbh + 2, display.width(), tbh + 2, GxEPD_BLACK);
    display.display(true);
  }
  int y = tbh + 4;

  display.setCursor(0, y);
  display.print("Not implemented");
  display.display(true);

  /*if (ble->TryLock()) {
    if (ble->NewData) {
      display.fillRect(150, 0, display.width() - 150, tbh, GxEPD_WHITE);
      display.setCursor(150, 0);
      display.print(ble->Count);
      display.print(" devices");

      // Display the list of networks
      display.fillRect(0, y, display.width(), display.height() - tbh, GxEPD_WHITE);
      for (int i = 0; i < ble->Count; i++) {
        BLEAdvertisedDevice d = ble->FoundDevices->getDevice(i);

        display.setCursor(0, y);
        display.print(d.getName().c_str());
        y += tbh + 2;
      }
      // Update the display
      display.display(true);
      display.hibernate();

      ble->NewData = false;
    }
    ble->Unlock();
    }
  */
}
