# SigGen

# Simple Waveform Generator

# This is a waveform generator project that uses STM32F072RB nucleo development board.

# The waveform generator supports Sine, Sawtooth, Triangular, and Square wavee.

# Current specification supports 1Hz to 2000Hz and amplitude of 1.0v to 3.3v.

# The waveform selection is using the serial UART port in the nucleo development board.

# The output channel for the waveform on Pin PA_4 according to the pinout diagram of STM32F072RB nucleo development board.

# System Default:

	# The serial terminal menu is printed using 115200 baud rate.
	# The Pin PA_4 will output 1kHz Sine Wave with 3.3V amplitude when the board startup.
