
#include "AudioFFT.h"

#include "arduinoFFT.h" // Standard Arduino FFT library https://github.com/kosme/arduinoFFT
arduinoFFT FFT = arduinoFFT();

// don't mess around with this
i2s_config_t i2s_config = {
  .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
  .sample_rate = SAMPLE_RATE,
  .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
  .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
  .communication_format = I2S_COMM_FORMAT_I2S,
  .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
  .dma_buf_count = 4,
  .dma_buf_len = 1024,
  .use_apll = false,
  .tx_desc_auto_clear = false,
  .fixed_mclk = 0
};

// and don't mess around with this
i2s_pin_config_t i2s_mic_pins = {
  .bck_io_num = I2S_MIC_SERIAL_CLOCK,
  .ws_io_num = I2S_MIC_LEFT_RIGHT_CLOCK,
  .data_out_num = I2S_PIN_NO_CHANGE,
  .data_in_num = I2S_MIC_SERIAL_DATA
};


AudioFFT::AudioFFT() {
  // start up the I2S peripheral
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &i2s_mic_pins);

  sem = xSemaphoreCreateMutex();

  xTaskCreate(
    AudioFFT::FFTTask,
    "FFT",
    16384,
    (void *)this,
    1,
    &fftTask
  );
}


bool AudioFFT::TryLock() {
  return xSemaphoreTake(sem, 1);
}

void AudioFFT::Unlock() {
  xSemaphoreGive(sem);
}


void AudioFFT::FFTTask(void * param) {
  AudioFFT *fft = (AudioFFT *)param;
  double vReal[SAMPLE_BUFFER_SIZE];
  double vImag[SAMPLE_BUFFER_SIZE];
  int32_t raw_samples[SAMPLE_BUFFER_SIZE];

  for (;;) {
    // Read samples
    size_t bytes_read = 0;
    i2s_read(I2S_NUM_0, raw_samples, sizeof(int32_t) * SAMPLE_BUFFER_SIZE, &bytes_read, portMAX_DELAY);
    int samples_read = bytes_read / sizeof(int32_t);
    for (int i = 0; i < samples_read; i++) {
      vReal[i] = (float)raw_samples[i];
      vImag[i] = 0;
    }

    // FFT
    FFT.Windowing(vReal, samples_read, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
    FFT.Compute(vReal, vImag, samples_read, FFT_FORWARD);
    FFT.ComplexToMagnitude(vReal, vImag, samples_read);

    // Store results for UI thread
    // NOTE:  if we can't take the semaphore right away because the UI thread has it we just
    //        continue on and do another FFT reading.
    if (fft->TryLock()) {
      for (int i = 2; i < samples_read / 2; i++)
      {
        fft->FFTResults[i] = round(vReal[i] / 2147483647.0 * 10000);
      }
      fft->NewData = true;
      fft->Unlock();
    }

    // Wait a bit before scanning again
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}
