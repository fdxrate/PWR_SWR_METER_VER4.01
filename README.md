# PWR_SWR_METER_VER4.01

Digital RF Power and SWR meter in collaboration with Richard Hosking.

High Performance HF Power meter using a Directional Coupler feeding into logarithmic converters (AD3807 log amps) which are read by an Arduino Nano via I2C using 16bit ADC's (ADS1115).

This drevice can measure power over a wide dynamic range (typically for Type43 toroidal cores - the amateur bands 80m through 10m) from 10uW to 1 KW and will derive SWR for a 50 Ohm load. The front end uses broadband Ferrite transformers in a Directional Coupler configuration into AD8307 Logarithmic amplifiers which derive power in a forward and reverse direction. An Arduino Nano 5V board performs calculations and drives a 128 x 64 oled display.
