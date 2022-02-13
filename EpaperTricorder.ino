#include "buttons.h"
#include "WifiScanner.h"
#include "AudioFFT.h"

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

  btns = new Buttons();
  wifi = new WifiScanner();
  audioFFT = new AudioFFT();

}



int uiMode = 0;
bool uiFirst = true;
void loop()
{
  if (btns->btn1SingleClick) {
    uiMode++;
    if (uiMode > 1) uiMode = 0;
    ClearDisplay();
    uiFirst = true;
  }

  btns->Reset();

  switch (uiMode) {
    case 0:
      loop_wifi(uiFirst);
      break;
    case 1:
      loop_fft(uiFirst);
      break;
    default:
      break;
  }
  uiFirst = false;
}

int16_t tbx, tby; uint16_t tbw, tbh;
void ClearDisplay() {
  //display.setFullWindow();
  //display.clearScreen();
  display.fillRect(0, 0, display.width(), display.height(), GxEPD_WHITE);
  display.display(false);
  //display.reset();

  //  display.setFont(&FreeSans9pt7b);
  display.setFont(NULL);
  display.setTextColor(GxEPD_BLACK);
  //display.getTextBounds("M", 0, 0, &tbx, &tby, &tbw, &tbh);
  tbh = 7;
}

void loop_wifi(bool first) {
  if (first) {
    //ClearDisplay();

    display.setCursor(0, 0);
    display.print("Wifi Scanner");
    display.drawLine(0, tbh + 2, display.width(), tbh + 2, GxEPD_BLACK);
    display.display(true);
  }

  int y = tbh + 4;

  if (wifi->TryLock()) {
    if (wifi->NewData) {
      wifi->NewData = false;
      display.fillRect(150, 0, display.width() - 150, tbh, GxEPD_WHITE);
      display.setCursor(150, 0);
      display.print(wifi->Count);
      display.print(" networks");

      display.fillRect(0, y, display.width(), display.height() - tbh, GxEPD_WHITE);
      for (int i = 0; i < wifi->Count; i++) {
        display.setCursor(0, y);
        display.print(WiFi.SSID(i));
        display.print(" ");
        display.print(WiFi.RSSI(i));
        y += tbh + 2;
      }
      display.display(true);
      display.hibernate();
    }
    wifi->Unlock();
  }
}

void loop_fft(bool first) {
  if (first) {
    //ClearDisplay();

    display.setCursor(0, 0);
    display.print("FFT");
    display.drawLine(0, tbh + 2, display.width(), tbh + 2, GxEPD_BLACK);
    display.display(true);

  }

  if (audioFFT->TryLock()) {
    if (audioFFT->NewData) {
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
