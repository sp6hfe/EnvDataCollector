# EnvDataCollector

## Introduction
Environmental Data Collector is an embedded device cyclically gathering measurements from connected sensors which pushes measurement data to remote storage using the selected method. It has a unified way data is sent allowing for easier management of incoming measurements on the receive side.

## Motivation
The motivation for this project was to enable interested hobbyists and scientists to focus on collected environmental data analysis rather than spending time battling with an embedded code development (or money for the devices that don't offer any modifications to their functionality). Code and instructions available in this repository should allow for shortening of the HW bring up time and limit the amount of code (requiring implementation) needed to run efficient environmental data collection.

## Code architecture
Code is written in a modular fashion allowing for its portability. HW-depended code is decoupled from the application code (with the use of dedicated interfaces) allowing for adding both: new sensors and new HW-platform support mostly by writing wrapper classes.

## Development suite
PlatformIO was selected as a development suite. PlatformIO - being a plugin for the Visual Studio Code - allows for improved development experience when compared to simple Arduino IDE. Having a project set up in PlatformIO we get platform independence as well as the ability to debug the code, version management of external libraries, and the possibility to specify compilation environments. PlatformIO allows for usage of the Arduino framework which makes it a good choice for a vast majority of the people from the makers' movement providing an augmented development environment.

## HW support
Currently, while the core code is still under development, HW listed below is already supported (meaning there is a dedicated PratformIO build environment prepared):
### HW-platforms
* NodeMCU v3 (ESP8266)
### Sensors
* Bosch BME280

## Data uploading support
Currently supported data upload mechanisms are:
* HTTP POST over WiFi