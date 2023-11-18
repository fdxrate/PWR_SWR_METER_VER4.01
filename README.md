# PWR_SWR_METER_VER4.01

Digital RF Power and SWR meter in collaboration with R H.

Ver4.01 'Original Commit' is a basic code example to establish the 'workability' of the schematic, pcb and code. Other features to be added in future versions.

High Performance HF Power meter using a Directional Coupler feeding into logarithmic converters (AD3807 log amps) which are read by an Arduino Nano via I2C using 16bit ADC's (ADS1115).

This drevice can measure power over a wide dynamic range (typically for Type43 toroidal cores - the amateur bands 80m through 10m) from 10uW to 1 KW and will derive SWR for a 50 Ohm load. The front end uses broadband Ferrite transformers in a Directional Coupler configuration into AD8307 Logarithmic amplifiers which derive power in a forward and reverse direction. An Arduino Nano 5V board performs calculations and drives a 128 x 64 oled display.
![Pic1_Sml](https://github.com/fdxrate/PWR_SWR_METER_VER4.01/assets/8214848/0f080a8d-94ac-4483-9b44-838d74fa4973)
![Pic3_Sml](https://github.com/fdxrate/PWR_SWR_METER_VER4.01/assets/8214848/a5d977e3-e476-466d-b278-bd86cc66f4f4)
