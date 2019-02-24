# Send sensor

Read and digitlal sensor and send the information by MQTT.

This project is based on [get started project template](https://github.com/espressif/ESP8266_RTOS_SDK/tree/master/examples/get-started) for ESP8266_RTOS_SDK.

## Requirements
### Install ESP8266_RTOS_SDK

Follow the steps to install from github:
https://github.com/espressif/ESP8266_RTOS_SDK

Follow all the steps to compile a project with the ESP8266_RTOS_SDK, and **be sure the toolchains xtensa-lx106-elf-\* are available on $PATH after you install ESP8266_RTOS_SDK**.

## Build and Deploy

First, make sure you configure compile options for the project. Be sure you review the "Custom Project Configuration" section to **configure Wifi and MQTT Connections**:

```bash
$ make menuconfig
```

You'll see now a `sdkconfig` file on your project root. It contains the definitions for many things (flash, ESP82266 states, etc), including the WIFI and MQTT definitions we're going to use to compile the project.

Build and flash the project to ESP8266. Of course, be sure to have connected the chip in flash mode.

```bash
# (Optional) Be sure you have the IDF_PATH env variable configured
export IDF_PATH=/opt/esp/ESP8266_RTOS_SDK # Use your custom path

$ make # Compile the project
$ make flash # Flash on the chip (ESP8266)
```
