# UProgrammer-Firmware
Firmware for universal programmer based on ESP-12E and Espressif API

Version  00E

There is a menu that works over the serial port at 115200 baud

This version adds functionality to test Switched boost Circuit and SigmaDelta .
Updated simple serial menu in the file simple_serial.c.  The new code puts up adds a few commands to the simple menu.
Cleaned up some of the code to make it easier to read.
Consolidated code from uart.c into simple_serial.c and removed uart.c from the project

The code is very poorly documented and a only few helper functions have been abstracted.
