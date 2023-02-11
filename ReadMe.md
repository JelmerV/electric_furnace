# Electronic furnace controller

This is an arduino sketch for the controls of an electric furnace/foundry. It uses a K type thermocouple in combination with a MAX6675 IC to check the temperature inside the furnace. Current temperature and the target temperature are displayed on a small 128x64 oled display. An encoder can be used to change the target temperature. 2 solid state relays are used to which the 220V on the heating coils. It switched both wires to ensure there is no live voltage on the coils. 2 switched on the lid detect when the lid is lifted, and it will then immediately switch off the heating wires. This is also indicated on the display by the icon of an open door. Whenever the heating elements are powered, a flaming icon is displayed.

## Wiring

The microcontroller used is an arduino nano. Both relays are controlled by a single output signal. The door switched are wired in series, using the normally closed contact. I am using a small 220vAC-5vDC converter to get the 5v for the arduino. The encoder is attached at pins 2 and 3 for external interrupts. The display uses pins A4 and A5 for SPI communication.

## Libraries

This is a platformio project, so you should be able to just open this in vs code, if you have the platformio extension. If you want to use the arduino ide, then you will need to manually install the required libraries. These are:

* Adafruit_GFX (for the graphics)
* Adafruit_SSD1306 (for the mini oled display)
* Adafruit max6675 (for reading the thermocouple)

# $projectName

A short description of the project goes here.

## Table of Contents

- [Introduction](#introduction)
- [Features](#features)
- [Prerequisites](#prerequisites)
- [Installation](#installation)
- [Usage](#usage)
- [Documentation](#documentation)
- [Troubleshooting](#troubleshooting)

## Introduction

In this section, provide a brief overview of the project, including its purpose and goals.

## Features

List the main features of the project.

## Prerequisites

List any prerequisites that are required for the project, such as software, hardware, libraries, etc.

## Installation

Provide detailed instructions on how to install and set up the project.

## Usage

Describe how to use the project and any relevant usage examples.

## Documentation

Include links to any additional documentation, such as API references or user guides.

## Troubleshooting

Provide tips and guidance on how to troubleshoot common issues that may arise when using the project.
