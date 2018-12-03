# WiFi-LED-XMAS-tree
This software is built for a for WEMOS D1 mini (WEMOS) to control the LEDs of modified electronics XMAS tree via WiFi. The tree consists of 2 christmas-tree-shaped PCBs with 36 LEDs (each PBC has 18 or 6 LEDs for each color). The light intensity of each color can be controlled with the WEMOS by using PWM. Additionally the modified version of the tree has a multicolor LED (WS2812B) mounted to the top which can be controlled by the WEMOS too.

## List of known bugs
- [x] christmas tree is stuck in initialization when connection to WiFi fails
- [x] animation of top LED is not restored when setting default values

## Desired features

- [ ] spawn own WiFi network to allow manual configuration if connection to WiFi failed
- [ ] implement updater to download and install new firmware versions
- [x] proper encapsulation of animation functions
- [x] more animations for LEDs
	- [x] simple edgy basic animation
	- [x] smooth sine and cosine wave animation
	- [x] seemingly random Chen attractor function with Marijn
	- [ ] other animations that look nice or are fun to implement
- [ ] show hostname and IP address in small OLED display
- [ ] timer function for automatic wakeup/sleep (maybe based on location and sundown/sunrise?)
- [ ] crowd control of multiple trees at once

[Online documentation in the MuMaLab-Wiki](https://wiki.munichmakerlab.de/wiki/WiFi-LED-XMAS-tree)
