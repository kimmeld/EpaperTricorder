#include <Arduino.h>
#include "buttons.h"
#include "WifiScanner.h"
#include "AudioFFT.h"
#include "Environment.h"
#include "SDLogger.h"

#include <GxEPD2_GFX.h>
#include <GxEPD2_7C.h>
#include <GxEPD2_EPD.h>
#include <GxEPD2_3C.h>
#include <GxEPD2_BW.h>
#include <GxEPD2.h>

GxEPD2_BW<GxEPD2_270, GxEPD2_270::HEIGHT> display(GxEPD2_270(/*CS=5*/ SS, /*DC=*/17, /*RST=*/16, /*BUSY=*/4));

// Buttons for user interaction
Buttons *btns;

// Wifi Scanner
WifiScanner *wifi;

// FFT
AudioFFT *audioFFT;

// Environmental - eCO2, Temperature, Pressure
EnvironmentSensor *environmentSensor;

// SD Logger
SDLogger *logger;

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
int uiMode = 0;      // which screen are we on
bool uiFirst = true; // Is this the first time through this screen's loop?

void ClearDisplay()
{
  display.fillRect(0, 0, display.width(), display.height(), GxEPD_WHITE);
  display.display(false);

  display.setFont(NULL);
  display.setTextColor(GxEPD_BLACK);
}

// UI Code for Logging Status
void loop_logging(bool first)
{
  // On the logging screen, button 2 toggles logging on and off
  if (btns->btn2SingleClick)
  {
    first = true;  // Force a display redraw
    btns->Reset();
    if (logger->active)
    {
      // stop logging
      logger->end();
    }
    else
    {
      // start logging
      logger->begin();
    }
  }

  if (first)
  {
    // Draw header
    display.setCursor(0, 0);
    display.print("Logging Status");
    display.drawLine(0, HeaderBottom, display.width(), HeaderBottom, GxEPD_BLACK);
    //display.display(true);

    display.fillRect(150, 0, display.width() - 150, TextHeight, GxEPD_WHITE);
    display.setCursor(150, 0);
    if (logger->active)
    {
      display.print("ACTIVE");
    }
    else
    {
      display.print("inactive");
    }

    display.fillRect(0, DataStart, display.width(), DataHeight, GxEPD_WHITE);

    display.setCursor(0, DataStart);
    display.print("SD Card Type: ");
    switch (logger->cardType)
    {
    case CARD_MMC:
      display.println("MMC");
      break;
    case CARD_SD:
      display.println("SD");
      break;
    case CARD_SDHC:
      display.println("SDHC");
      break;
    case CARD_NONE:
      display.println("No card");
      break;
    default:
      display.println("Unknown");
      break;
    }

    display.print("Card size: ");
    display.print(logger->cardSize);
    display.println(" bytes");

    display.display(true);
    display.hibernate();
  }
}

// UI Code for Wifi Scanner
void loop_wifi(bool first)
{
  if (first)
  {
    // Draw header
    display.setCursor(0, 0);
    display.print("Wifi Scanner");
    display.drawLine(0, HeaderBottom, display.width(), HeaderBottom, GxEPD_BLACK);
    display.display(true);
  }

  int y = TextHeight + 4;
  if (wifi->TryLock())
  {
    if (wifi->NewData)
    {
      wifi->NewData = false;
      // Display number of networks
      display.fillRect(150, 0, display.width() - 150, TextHeight, GxEPD_WHITE);
      display.setCursor(150, 0);
      display.print(wifi->Count);
      display.print(" networks");

      // Display the list of networks
      display.fillRect(0, y, display.width(), DataHeight, GxEPD_WHITE);
      for (int i = 0; i < wifi->Count; i++)
      {
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

void loop_fft(bool first)
{
  if (first)
  {
    // Draw header
    display.setCursor(0, 0);
    display.print("FFT");
    display.drawLine(0, HeaderBottom, display.width(), HeaderBottom, GxEPD_BLACK);
    display.display(true);
  }

  if (audioFFT->TryLock())
  {
    if (audioFFT->NewData)
    {
      // Display FFT results
      double fftMax = (double)(DataHeight);
      display.fillRect(0, TextHeight + 4, display.width(), fftMax, GxEPD_WHITE);
      for (int i = 2; i < SAMPLE_BUFFER_SIZE / 2; i++)
      {
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

void display_environment_sensors(bool first)
{
  // This displays the four environmental sensors along the right-hand column
  if (first)
  {
    display.setCursor(204, DataStart);
    display.print("eCO2");

    display.setCursor(204, DataStart + (TextHeight + 2) * 3);
    display.print("Temp");

    display.setCursor(204, DataStart + (TextHeight + 2) * 6);
    display.print("Pres");

    display.setCursor(204, DataStart + (TextHeight + 2) * 9);
    display.print("Samples");

    display.setCursor(204, DataStart + (TextHeight + 2) * 12);
    display.print("Baseline");
  }

  display.fillRect(204, DataStart + (TextHeight + 2) * 1, 60, TextHeight, GxEPD_WHITE);
  display.setCursor(204, DataStart + (TextHeight + 2) * 1);
  display.print(environmentSensor->co2[199]);
  display.print("ppm");

  display.fillRect(204, DataStart + (TextHeight + 2) * 4, 60, TextHeight, GxEPD_WHITE);
  display.setCursor(204, DataStart + (TextHeight + 2) * 4);
  display.print(environmentSensor->temp[199]);
  display.print("C");

  display.fillRect(204, DataStart + (TextHeight + 2) * 7, 60, TextHeight, GxEPD_WHITE);
  display.setCursor(204, DataStart + (TextHeight + 2) * 7);
  display.print(environmentSensor->pres[199]);
  display.print("pa");

  display.fillRect(204, DataStart + (TextHeight + 2) * 10, 60, TextHeight, GxEPD_WHITE);
  display.setCursor(204, DataStart + (TextHeight + 2) * 10);
  display.print(environmentSensor->sample_count);

  display.fillRect(204, DataStart + (TextHeight + 2) * 13, 60, (TextHeight + 2) * 2, GxEPD_WHITE);
  display.setCursor(204, DataStart + (TextHeight + 2) * 13);
  display.print(environmentSensor->ccs811baseline);
  display.setCursor(204, DataStart + (TextHeight + 2) * 14);
  display.printf("0x%04X", environmentSensor->ccs811baseline);
}

const int co2_offset = 380;
const int co2_scale = 10;
void loop_co2(bool first)
{
  display_environment_sensors(first);
  if (first)
  {
    // Draw header
    display.setCursor(0, 0);
    display.print("Environment - eCO2");
    display.drawLine(0, HeaderBottom, display.width(), HeaderBottom, GxEPD_BLACK);
    display.drawLine(202, HeaderBottom, 202, 200, GxEPD_BLACK);
    display.display(true);
  }

  if (environmentSensor->TryLock())
  {
    if (environmentSensor->NewData)
    {
      // Display CO2 results
      display.fillRect(0, TextHeight + 4, 200, DataHeight, GxEPD_WHITE);
      for (int i = 0; i < 200; i++)
      {
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
void loop_temp(bool first)
{
  display_environment_sensors(first);
  if (first)
  {
    // Draw header
    display.setCursor(0, 0);
    display.print("Environment - Temperature");
    display.drawLine(0, HeaderBottom, display.width(), HeaderBottom, GxEPD_BLACK);
    display.drawLine(202, HeaderBottom, 202, 200, GxEPD_BLACK);
    display.display(true);
  }
  if (environmentSensor->TryLock())
  {
    if (environmentSensor->NewData)
    {
      // Display temperature results
      display.fillRect(0, TextHeight + 4, 200, DataHeight, GxEPD_WHITE);
      for (int i = 0; i < 200; i++)
      {
        float s = environmentSensor->temp[i];
        s -= temp_offset;
        s /= temp_scale;
        s = max(min((int)s, DataHeight / 2), -DataHeight / 2);
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
void loop_pres(bool first)
{
  display_environment_sensors(first);
  if (first)
  {
    // Draw header
    display.setCursor(0, 0);
    display.print("Environment - Pressure");
    display.drawLine(0, HeaderBottom, display.width(), HeaderBottom, GxEPD_BLACK);
    display.drawLine(202, HeaderBottom, 202, 200, GxEPD_BLACK);
    display.display(true);
  }

  if (environmentSensor->TryLock())
  {
    if (environmentSensor->NewData)
    {
      // Display CO2 results
      display.fillRect(0, TextHeight + 4, 200, DataHeight, GxEPD_WHITE);
      for (int i = 0; i < 200; i++)
      {
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
  // ble = new BLEScanner();
  environmentSensor = new EnvironmentSensor();

  logger = new SDLogger();
  logger->AddSensor(environmentSensor);
  logger->AddSensor(wifi);
  logger->begin();

  uiMode = 0;
  uiFirst = true;
}

// UI Loop
void loop()
{
  // Go to the next screen
  if (btns->btn1SingleClick)
  {
    btns->Reset();
    uiMode++;
    if (uiMode > 5)
      uiMode = 0;
    ClearDisplay();
    uiFirst = true;

    // Now that logging is a thing, we can't disable any of our sensors when they are unused
    // wifi->Enable = false;
    // ble->Enable = false;
  }

  // Dispatch to the appropriate UI loop
  switch (uiMode)
  {
  case 0:
    loop_logging(uiFirst);
    break;
  case 1:
    loop_wifi(uiFirst);
    break;
  case 2:
    loop_fft(uiFirst);
    break;
  case 3:
    loop_co2(uiFirst);
    break;
  case 4:
    loop_temp(uiFirst);
    break;
  case 5:
    loop_pres(uiFirst);
    break;
  default:
    break;
  }
  uiFirst = false;
}