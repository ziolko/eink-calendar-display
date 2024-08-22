# E-ink meeting room schedule & photo frame

This repository contains source code, models for 3D printing the case and instructions for creating a wireless e-ink device displaying schedule for a selected meeting room. After office hours the device can be turned into a photo frame featuring photos saved on a micro SD card. This makes it ideal e.g. for home office environment.

The device is based on the [Soldered Inkplate 6](https://soldered.com/product/soldered-inkplate-6-6-e-paper-board/) board that's available for ~110€. The earlier versions of Inkplate 6 (from e-radionica) are not supported.

The product uses [Roombelt](https://roombelt.com) as a backend. This is a paid product but for home office usage you can request a free license from [mateusz@roombelt.com](mailto:mateusz@roombelt.com). See pricing for commercial users [here](https://roombelt.com/#pricing).

## Installing firmware on the Inkplate 6
The firmware is developed with [PlatformIO](https://platformio.org/). To install firmware on your device follow the steps below:
1. Install PlatformIO for Visual Studio code as [documented here](https://platformio.org/install/ide?install=vscode).
2. Clone the repository locally and open it in Visual Studio Code.
3. Connect Inkplate 6 to the computer with a USB cable.
4. Click the "Upload" button to install the firmware. ![image](https://docs.platformio.org/en/latest/_images/platformio-ide-vscode-build-project.png)
## Configuring WiFi settings

## Printing and assembling the case

The case is made of two parts that are assembled using four M2*10 screws:
- Front
- Back cover

Print the parts in the provided orientation using any material with the following print settings:
- Supports: Yes
- Resolution: 0.2
- Infill: 20%


## Turning the device into e-ink photo 
