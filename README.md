# E-Paper Tricorder

The goal of this project is to put together a bunch of sensors - temperature, pressure, humidity, CO2, etc - into a small package, much like the Star Trek Tricorder.

This is built based on a LilyGo T5-V2.8 - [AliExpress](https://www.aliexpress.com/item/32867880658.html).  The display is a GDEW027W3 which, contrary to LilyGo's documentation, is supported by GxEPD2.

# Sensors

Currently this supports:

## FFT Audio Levels

This E-Paper board includes a ICS43434 I2S microphone.  The Audio Level sensor samples the microphone, feeds it into ArduinoFFT, and displays the results.

## 2.4GHz Wifi Networks

This sensor scans for new Wifi networks every five seconds and displays the ones it found and their signal strength.

## BLE Devices

This sensor scans for BLE devices and displays what it found.

# General Program Design

This program makes use of a FreeRTOS task for each sensor.  This allows all of the sensors to operate at the same time, though it doesn't currently display all of them at the same time.  There is also a FreeRTOS task dedicated to gathering input from the buttons, this is because some of the sensor displays (looking at you, FFT) take enough time to update that the buttons don't behave reasonably.

Each Sensor/Task is encapsulated by a C++ object.  Apart from the Button task, each object also includes a semaphore and utility methods for guarding access to the sensor's output.

The "main" Arduino task (which runs `loop()`) is used exclusively for runing the E-Paper display and handling user-interactions from the Button task.
