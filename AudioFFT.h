/*
    Audio FFT Sensor


*/
#ifndef AUDIOFFT_H
#define AUDIOFFT_H

#include "FreeRTOS.h"
#include "driver/i2s.h"

// https://github.com/G6EJD/ESP32-8-Octave-Audio-Spectrum-Display/blob/master/ESP32_Spectrum_Display_03.ino
// you shouldn't need to change these settings
#define SAMPLE_BUFFER_SIZE 512
#define SAMPLE_RATE 16000

// most microphones will probably default to left channel but you may need to tie the L/R pin low
#define I2S_MIC_CHANNEL I2S_CHANNEL_FMT_ONLY_LEFT
// either wire your microphone to the same pins or change these to match your wiring
#define I2S_MIC_SERIAL_CLOCK 32
#define I2S_MIC_LEFT_RIGHT_CLOCK 33
#define I2S_MIC_SERIAL_DATA 27

class AudioFFT
{
  public:
    AudioFFT();

    bool NewData;
    double FFTResults[SAMPLE_BUFFER_SIZE / 2];

    bool TryLock();
    void Unlock();

    static void FFTTask(void * param);
  protected:
    SemaphoreHandle_t sem;
  private:
    TaskHandle_t fftTask;
};


#endif
