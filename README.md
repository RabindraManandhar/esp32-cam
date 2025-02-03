# ESP-CAM Image Capture & Upload System
A FastAPI-powered ESP32-CAM project for real-time image capture and wireless transmission.

## Overview
This project utilizes an ESP32-CAM module to capture images and send them wirelessly to a FastAPI server. The server manages image storage, retrieval, and remote access. The system is useful for real-time monitoring, IoT-based vision applications, and AI model training.

## Table of Contents
1. [Features](#features)
2. [Project Structure](#project-structure)
3. [Requirements](#requirements)
4. [Installation and Setup](#installation-and-setup)
5. [Troubleshooting](#troubleshooting)
6. [Acknowledgements](#acknowledgements)

## Features
- ESP32-CAM module configuration
- Wi-Fi connection configuration
- FastAPI server setup
- Capture images usign ESP32-CAM
- Upload images to FastAPI server over Wi-Fi via HTTP POST method

## Project Structure
The project contains one source file in C language main.c. The file is located in folder main.

ESP-IDF projects are built using CMake. The project build configuration is contained in CMakeLists.txt files that provide set of directives and instructions describing the project's source files and targets (executable, library, or both).

Below is short explanation of remaining files in the project folder.

```
ESP32-CAM/
│── components/                 # IDF Components
│   ├── esp32-camera            # esp32-camera component
│   ├── ....                    # Other idf components
│── main/                       # ESP32-CAM firmware code
│   ├── main.c                  # ESP32-CAM main program
│   ├── CMakeLists.txt          # CMake main file
├── .gitignore                  # Ignore unnecessary files
├── CMakeLists.txt              # CMake project file
│── README.md                   # Project documentation
│── requirements.txt            # Python dependencies
├── sdkconfig                   # ESP-IDF configuration
├── sdkconfig.ci                # ESP-IDF configuration
├── sdkconfig.old               # ESP-IDF configuration
│── server.py                   # FastAPI backend script
```

## Requirements
1. Hardware
    - ESP-CAM board with OV2640 camera module
    - micro USB cable
    - Computer running Windows, Linux, or macOS

2. Software
    - Toolchain to compile code for ESP32
    - Build tools - CMake and Ninja to build a full Application for ESP32
    - ESP-IDF that essentially contains API (software libraries and source code) for ESP32 and scripts to   operate the Toolchain
    - Server setup tools - fastAPI and Uvicorn to setup server


## Installation and Setup
1. Install ESP-IDF (ESP32 Development Framework)

    Follow the [official ESP-IDF installation guide](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/index.html) for your OS.

    ```bash
    git clone --recursive https://github.com/espressif/esp-idf.git
    cd esp-idf
    ./install.sh
    ```

    Activate ESP-IDF
    ```bash
    . $HOME/esp/esp-idf/export.sh
    ```

2. Start a Project

    Clone the repository:
    ```bash
    git clone https://github.com/RabindraManandhar/esp32-cam.git
    cd esp32-cam
    ```


3. Connect Device

    Now connect an ESP32 board to the computer and check under which serial port the board is visible.


4. Configure the Project

    Navigate to the project directory, set ESP32 as the target, and run the project configuration utility menuconfig.

    ```bash
    cd ~/esp/esp32-cam
    idf.py set-target esp32
    idf.py menuconfig
    ```

    Use this menu to set up project specific variables, e.g., Wi-Fi network name and password, the processor speed, etc.

5. Build the Project

    Build the project by running:
    ```bash
    idf.py build
    ```

    If there are no errors, the build finishes by generating the firmware binary .bin files.


6. Flash into the Device

    To flash the binaries built for the ESP32 in the previous step, run the following command:
    ```bash
    idf.py -p PORT flash
    ```
    
    Replace `PORT` with ESP32 board's USB port name.
    
7. Monitor the Output

    To monitor the output, run the following command:
    ```bash
    idf.py -p PORT monitor
    ```
    
    Replace `PORT` with ESP32 board's USB port name.


## Troubleshooting
1. Establish Serial Connection with ESP32:
    - For Serial Connection issues, please refer to [Establish Serial Connection with ESP32](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/establish-serial-connection.html) for full details.

1. Program upload failure
    - Hardware connection is not correct: run idf.py -p PORT monitor, and reboot the board to see if there are any output logs.
    - The baud rate for downloading is too high: lower your baud rate in the menuconfig menu, and try again.

2. Flashing Troubleshooting
    - For failed to Connect issues, please refer to [Flashing Troubleshooting](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/get-started/flashing-troubleshooting.html).


## Acknowledgements
- `ESPRESSif` for esp32-cam configuration
- `FastAPI` for server setup
