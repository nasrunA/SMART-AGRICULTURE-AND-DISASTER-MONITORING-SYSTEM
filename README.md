
# Smart Agriculture and Disaster Monitoring System

## Project Overview

The Smart Agriculture and Disaster Monitoring System is an IoT-based project developed using the ESP32 microcontroller to monitor environmental conditions and detect potential disaster risks in agricultural areas.

The system integrates multiple sensors to collect real-time data, including temperature, humidity, gas levels, water levels, and vibration. When sensor readings exceed predefined thresholds, the system activates warning indicators and a buzzer to alert users.

The project uses Wokwi for hardware simulation, Arduino IDE for programming, and Blynk for remote monitoring.

## Objectives

- Monitor environmental conditions in agricultural areas.
- Detect potential hazards such as flooding, gas leaks, excessive temperatures, and abnormal vibrations.
- Provide real-time sensor monitoring.
- Generate automatic alerts when sensor readings exceed predefined thresholds.
- Support safer and more efficient agricultural monitoring through IoT technology.

## Technologies Used

- **Microcontroller:** ESP32
- **Programming Language:** C++ (Arduino)
- **Simulation Platform:** Wokwi
- **Development Environment:** Arduino IDE
- **IoT Monitoring Platform:** Blynk

## Hardware Components

| Component | Purpose |
|---|---|
| ESP32 | Main microcontroller |
| DHT22 | Temperature and humidity monitoring |
| MPU6050 | Vibration and movement detection |
| MQ Gas Sensor | Gas level detection |
| HC-SR04 | Water-level measurement |
| LEDs | Visual warning indicators |
| Buzzer | Audio warning alerts |
| OLED Display | Sensor information display |

## Key Features

- Real-time temperature and humidity monitoring.
- Water-level and flood detection.
- Gas level monitoring.
- Vibration and earthquake detection.
- Automatic LED warning indicators.
- Buzzer alerts when abnormal conditions are detected.
- Remote sensor monitoring through Blynk.

## Project Simulation

The system was developed and tested using Wokwi to simulate the ESP32 and connected sensors without requiring physical hardware.

## How to Run

1. Clone or download this repository.
2. Open the `sketch.ino` file using Arduino IDE.
3. Install the required libraries.
4. Open the Wokwi project files to run the simulation.
5. Configure the Blynk credentials if required.
6. Start the simulation and monitor the sensor readings.

## Project Status

**Completed**

This project demonstrates the application of IoT technology in environmental monitoring and early disaster detection for agricultural environments.
