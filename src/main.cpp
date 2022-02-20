#include <Arduino.h>
#include "buttons.h"
#include "WifiScanner.h"
#include "AudioFFT.h"
#include "BLEScanner.h"
#include "Environment.h"

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
BLEScanner *ble;

// FFT
AudioFFT *audioFFT;

// Environmental - eCO2, Temperature, Pressure
EnvironmentSensor *environmentSensor;

// Display Constants
const int TextHeight = 7;
const int HeaderBottom = TextHeight + 2;
const int DataStart = HeaderBottom + 2;
const int DataHeight = 176 - DataStart;

/*
 * Display Layout
 * 
 * Height = 176, Width = 264
 * 
 * |----------------------------------------|
 * | Title area - y -> 0-HeaderBottom       |
 * |----------------------------------------|
 * | Data area  - y -> DataStart-DataHeight |
 * |              x -> 0-264                |
 * |                                        |
 * ...                                    ...  
 * |                                        |
 * ------------------------------------------
 * 
 * Title area
 * - Has the name of the sensor/display that's running
 * - May also have additional data, like WiFi network count
 * 
 * Data area
 * - Entirely up to the sensor/display that's running
 * - Could be a list of stuff - e.g., WiFi networks
 * - Could be a current-state graph - e.g., Audio spectrum
 * - Could be a historical-state graph - e.g., temperature, CO2, etc
 */ 

// UI Globals
int uiMode = 0;         // which screen are we on
bool uiFirst = true;    // Is this the first time through this screen's loop?

void ClearDisplay() {
  display.fillRect(0, 0, display.width(), display.height(), GxEPD_WHITE);
  display.display(false);

  display.setFont(NULL);
  display.setTextColor(GxEPD_BLACK);
}

// UI Code for Wifi Scanner
void loop_wifi(bool first) {
  if (first) {
    // Draw header
    display.setCursor(0, 0);
    display.print("Wifi Scanner");
    display.drawLine(0, HeaderBottom, display.width(), HeaderBottom, GxEPD_BLACK);
    display.display(true);
  }

  int y = TextHeight + 4;
  if (wifi->TryLock()) {
    if (wifi->NewData) {
      wifi->NewData = false;
      // Display number of networks
      display.fillRect(150, 0, display.width() - 150, TextHeight, GxEPD_WHITE);
      display.setCursor(150, 0);
      display.print(wifi->Count);
      display.print(" networks");

      // Display the list of networks
      display.fillRect(0, y, display.width(), DataHeight, GxEPD_WHITE);
      for (int i = 0; i < wifi->Count; i++) {
        display.setCursor(0, y);
        display.print(WiFi.SSID(i));
        display.print(" ");
        display.print(WiFi.RSSI(i));
        y += TextHeight + 2;
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
    display.drawLine(0, HeaderBottom, display.width(), HeaderBottom, GxEPD_BLACK);
    display.display(true);
  }

  if (audioFFT->TryLock()) {
    if (audioFFT->NewData) {
      // Display FFT results
      double fftMax = (double)(DataHeight);
      display.fillRect(0, TextHeight + 4, display.width(), fftMax, GxEPD_WHITE);
      for (int i = 2; i < SAMPLE_BUFFER_SIZE / 2; i++) {
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
    display.drawLine(0, HeaderBottom, display.width(), HeaderBottom, GxEPD_BLACK);
    display.display(true);
  }
  int y = TextHeight + 4;
  int x = 0;
  //  display.setCursor(0, y);
  //  display.print("Not implemented");
  //  display.display(true);

  if (ble->TryLock()) {
    if (ble->NewData) {
      display.fillRect(150, 0, display.width() - 150, TextHeight, GxEPD_WHITE);
      display.setCursor(150, 0);
      display.print(ble->FoundDevices.size());
      display.print(" devices");

      // Display the list of networks
      display.fillRect(0, y, display.width(), display.height() - TextHeight, GxEPD_WHITE);
      for (auto s : ble->FoundDevices) {
        display.setCursor(x, y);
        display.print(s);
        y += TextHeight + 2;
        if (y > (display.height() - TextHeight)) {
          y = TextHeight + 4;
          x += display.width()/2;
        }
      }
      // Update the display
      display.display(true);
      display.hibernate();

      ble->NewData = false;
    }
    ble->Unlock();
  }

}

const int co2_offset = 380;
const int co2_scale = 10;
void loop_co2(bool first) {
  if (first) {
    // Draw header
    display.setCursor(0, 0);
    display.print("eCO2");
    display.drawLine(0, HeaderBottom, display.width(), HeaderBottom, GxEPD_BLACK);
    display.drawLine(203, HeaderBottom, 203, 200, GxEPD_BLACK);

    display.display(true);
  }
  
  if (environmentSensor->TryLock()) {
    if (environmentSensor->NewData) {
      // Display CO2 results
      display.fillRect(0, TextHeight + 4, 200, DataHeight, GxEPD_WHITE);
      display.fillRect(80, 0, 70, TextHeight, GxEPD_WHITE);
      display.setCursor(80, 0);
      display.print(environmentSensor->co2[199]);
      display.print("ppm");
      for (int i = 0; i < 200; i++) {
        int s = environmentSensor->co2[i];
        s -= co2_offset;
        s /= co2_scale;
        s = min(s, DataHeight);
        display.drawLine(i, display.height() - s, i, display.height(), GxEPD_BLACK);
      }

      display.display(true);
      display.hibernate();
      environmentSensor->NewData = false;
    }
    environmentSensor->Unlock();
  }
}

const int temp_offset = 0;
const float temp_scale = 0.5;
const int vCenter = DataHeight / 2 + DataStart;
void loop_temp(bool first) {
  if (first) {
    // Draw header
    display.setCursor(0, 0);
    display.print("Temperature");
    display.drawLine(0, HeaderBottom, display.width(), HeaderBottom, GxEPD_BLACK);
    display.drawLine(203, HeaderBottom, 203, 200, GxEPD_BLACK);

    display.display(true);
  }
  
  if (environmentSensor->TryLock()) {
    if (environmentSensor->NewData) {
      // Display temperature results
      display.fillRect(0, TextHeight + 4, 200, DataHeight, GxEPD_WHITE);
      display.fillRect(80, 0, 70, TextHeight, GxEPD_WHITE);
      display.setCursor(80, 0);
      display.print(environmentSensor->temp[199]);
      display.print(" C");
      for (int i = 0; i < 200; i++) {
        float s = environmentSensor->temp[i];
        s -= temp_offset;
        s /= temp_scale;
        s = max(min((int)s, DataHeight/2), -DataHeight/2);
        display.drawLine(i, vCenter, i, vCenter - s, GxEPD_BLACK);
      }

      display.display(true);
      display.hibernate();
      environmentSensor->NewData = false;
    }
    environmentSensor->Unlock();
  }
}

const int pres_offset = 80000;
const int pres_scale = 1000;
void loop_pres(bool first) {
  if (first) {
    // Draw header
    display.setCursor(0, 0);
    display.print("Pressure");
    display.drawLine(0, HeaderBottom, display.width(), HeaderBottom, GxEPD_BLACK);
    display.drawLine(203, HeaderBottom, 203, 200, GxEPD_BLACK);

    display.display(true);
  }
  
  if (environmentSensor->TryLock()) {
    if (environmentSensor->NewData) {
      // Display CO2 results
      display.fillRect(0, TextHeight + 4, 200, DataHeight, GxEPD_WHITE);
      display.fillRect(80, 0, 70, TextHeight, GxEPD_WHITE);
      display.setCursor(80, 0);
      display.print(environmentSensor->pres[199]);
      display.print(" hPa");
      for (int i = 0; i < 200; i++) {
        int s = environmentSensor->pres[i];
        s -= pres_offset;
        s /= pres_scale;
        s = min(s, DataHeight);
        display.drawLine(i, display.height() - s, i, display.height(), GxEPD_BLACK);
      }

      display.display(true);
      display.hibernate();
      environmentSensor->NewData = false;
    }
    environmentSensor->Unlock();
  }
}

void setup()
{
  // Set WiFi to station mode and disconnect from an AP if it was previously connected
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  Wire.begin();

  Serial.begin(115200);

  display.init();
  display.setRotation(1);
  ClearDisplay();
  display.hibernate();



  // Create the various tasks we require
  btns = new Buttons();
  wifi = new WifiScanner();
  audioFFT = new AudioFFT();
  //ble = new BLEScanner();
  environmentSensor = new EnvironmentSensor();

  uiMode = 0;
  uiFirst = true;
}



// UI Loop
void loop()
{
  // Go to the next screen
  if (btns->btn1SingleClick) {
    uiMode++;
    if (uiMode > 4) uiMode = 0;
    ClearDisplay();
    uiFirst = true;

    wifi->Enable = false;
    //ble->Enable = false;
  }

  if (btns->btn2SingleClick) {
    Serial.println(ESP.getFreeHeap());
  }

  // We've read the buttons
  btns->Reset();

  // Dispatch to the appropriate UI loop
  switch (uiMode) {
    case 0:
      wifi->Enable = true;
      loop_wifi(uiFirst);
      break;
    case 1:
      loop_fft(uiFirst);
      break;
    case 2:
      loop_co2(uiFirst);
      break;
    case 3:
      loop_temp(uiFirst);
      break;
    case 4:
      loop_pres(uiFirst);
      break;
    default:
      break;
  }
  uiFirst = false;
}