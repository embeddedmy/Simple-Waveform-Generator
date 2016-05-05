
# Simple_Waveform_Generator

## Introduction

This is a signal generator which can generate 4 different waveforms with
configurable frequencies and amplitudes. Configurations are done through
a serial interface using a serial communication software.

This project is developed for the STM32 NUCLEO-FO72RB using Keil.

## System Specifications

1. Required hardware
 * STM32 NUCLEO-F072RB

2. Output pin
 * pin PA4
 
3. Supported waveforms
 * Sine
 * Square
 * Triangle
 * Sawtooth

4. Frequency range
 * Maximum frequency:	2kHz
 * Minimum frequency:	1Hz

5. Amplitude range
 * Maximum amplitude:	3.3V
 * Minimum amplitude:	1.0V
 
## Usage

1. Download the code [here](https://github.com/embeddedmy/SigGen.git).

2. Open the Keil Project File.

3. Compile and download the code onto the STM NUCLEO-FO72RB.

4. Use Putty or any serial communication software to interact with the
device. Set baud rate of the serial communication software to 115200.

5. Output waveform is observable on pin PA4.

## Source code

Download from [github](https://github.com/embeddedmy/SigGen.git).

## Submodules

1. Apptree
 * The apptree is a tree-based application display framework.
 * Source code available [here](https://github.com/cmlaw1993/apptree.git)

## Authors and Contributors

1. Tan Leong Kit (leongkeit91@gmail.com)
2. Tiong Joon Kiat Patrick (tiongpatrick86@gmail.com)
3. Yew Chung Law (lawyewchung@gmail.com)
2. Law Chiw Ming Dennis (cmlaw1993@gmail.com)
