# UProgrammer-Firmware
Firmware for universal programmer based on ESP-12F and Espressif API

Version  00I

There is a menu that works over the serial port at 115200 baud

This version fixes a bug that caused crashes when testing the switched boost Circuit and SigmaDelta .
I have crippled several functions that I plan to put back soon.
Updated simple serial menu in the file simple_serial.c.  
Consolidated code from uart.c into simple_serial.c and removed uart.c from the project

Version I restores crippled functions and is built on NONOS SDK V2.0.0

The code is very poorly documented and a only few helper functions have been abstracted.
