# Electronic furnace controller

This project contains the source code for the controller of an electric furnace, cad files for the enclosure, and some images of the system.

This is an arduino sketch for the controls of an electric furnace/foundry. It uses a K type thermocouple in combination with a MAX6675 IC to check the temperature inside the furnace. Current temperature and the target temperature are displayed on a small 128x64 oled display. An encoder can be used to change the target temperature. 2 relays are used to which the 220V on the heating coils. It switches both wires to ensure there is no live voltage on the coils. 2 switches are used to monitor when the lid gets lifted, in which case the voltage is immediately switched off. This is also indicated on the display by the icon of an open door. Whenever the heating elements are powered, a flaming icon is displayed.

## Wiring

The microcontroller used is an arduino nano. Both relays are controlled by a single output signal. The door switches are wired in series, using the normally closed contact. I am using a small 220vAC-5vDC converter to get the 5v for the arduino. The encoder is attached at pins 2 and 3 for external interrupts. The display uses pins A4 and A5 for SPI communication.

## Libraries

This is a platformio project, so you should be able to just open this in vs code if you have the platformio extension. If you want to use the arduino ide, then you will need to manually install the required libraries. The source code is [here](./Code/electric_furnace/src/main.cpp), and should also work as ini file. The required libraries are:

* Adafruit_GFX (for the graphics)
* Adafruit_SSD1306 (for the mini oled display)
* Adafruit max6675 (for reading the thermocouple)

## hardware

My furnace is made of firebricks and aerated concrete blocks. I used kanthal A1 wire to wind some coils totaling around 18 ohm, so at 220v it pulls around 12 amps, so 2600 Watts. THe 3d printed electronics enclosure is mounted on the side. The final system can be seen in the following image:

![final system](./Images/powered_up.jpg)
