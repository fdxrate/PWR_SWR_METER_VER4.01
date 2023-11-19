#include <Arduino.h>
#include <Adafruit_ADS1x15.h>
#include <Wire.h>
#include <math.h>
//#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MemoryFree.h>

#define SCREEN_WIDTH 128                          // OLED display width, in pixels
#define SCREEN_HEIGHT 64                          // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// ------------------------------------------------------------------------------
#define OLED_RESET -1                             // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
// ------------------------------------------------------------------------------
  

Adafruit_ADS1115 ads;                             // 16-bit version
// Adafruit_ADS1015 ads;                          // 12-bit version

void oledDisplay(void);
void serialDataOut(void);

// Test Github changes. Library links added.
 
// Variables.
// ------------------------------------------------------------------------------
  const int numReadings = 10;
  int readIndexFWD = 0;            // the index of the current reading
  int readIndexRFL = 0;

  /**
   * An array of floating point values used to store the forward power readings.
   */
  float readingsFWD[numReadings];
  float readingsRFL[numReadings];
  
  int readIndexMax = 0;             // The index number of the maximum reading.

  float totalFWD = 0.000;           // Running total of forward power reading.
  float totalRFL = 0.000;           // Running total of reflected power reading.
  
  float averageFWD = 0.000;         // The average of the forward power readings.
  float averageRFL = 0.000;         // The average of the reflected power readings.
  int i;
// ------------------------------------------------------------------------------


// Power & SWR Calc Variables.
// ------------------------------------------------------------------------------
float SlopeAD8307 = 0.025;                          // V/dBm
float SlopeADS1115 = 0.125;                         // mV/step using GAIN=ONE;
// float SlopeADS1015 = 0.002;                      // mV/step using GAIN=ONE;

// Calculate the slope of the AD8307 sensor in units of dBm per step, and convert it to units of mV per step for the ADS1115 ADC.
float SlopeCalc = SlopeAD8307*1000/SlopeADS1115;    // 200 steps/dBm
// float SlopeCalc = SlopeAD8307/SlopeADS1015;

/**
 * The Zero Intercept is the power level at which the power meter reads zero.
 * It is measured in dBm (decibel-milliwatts).
 */
int ZeroIntercept = -94;                // dBm - Determined by testing and calibrating the AD8307 sensor.

int AttenPCB = 50;                      // dB = 22.4dB for the PCB attenuator + 27.6dB for the transformer.
int TotalAtten = ZeroIntercept + AttenPCB;  // ZeroIntercept + AttenPCB. (dBm)

float peakFWD = 0.000, peakRFL = 0.000, maxFWD = 0.000;

// float ma_peak_FWD = 0.000;           // dBm

float PeakPower_dBm_FWD = 0.000;        // dBm
float PeakPower_dBm_RFL = 0.000;        // dBm
float AveragePower_dBm_FWD = 0.000000;  // dBm
float AveragePower_dBm_RFL = 0.000;     // dBm
int Power_Watts_FWD = 0.000;            // Watts
int Power_Watts_RFL = 0.000;            // Watts
float PeakPower_Watts_FWD = 0.000;
float PeakPower_Watts_RFL=0.000;
float AvPower_Watts_FWD = 0.000;
float PeakSWR = 0.000;
float maxPower_dBm_FWD = 0.000;
float max_Power_Watts_FWD = 0.000;
int AverageSWR = 0.000;

// float TEST_WATTS = 0.000;
// ------------------------------------------------------------------------------
 
 
void setup() {

  Serial.begin(9600);
  delay(100);

// Setting the I2C clock speed to 400kHz
  Wire.setClock(400000);

  Serial.println(F("iMagiWorks Power/SWR Meter"));
  Serial.print("freeMemory()=");
  Serial.println(freeMemory());     // Display free memory. Needs to be over 1000 to work.
  delay(1000);

  
// SSD1306 SETUP
// ------------------------------------------------------------------------------
// SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {    // Address 0x3C for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);                          // Don't proceed, loop forever
  }
  Serial.println(F("SSD1306 allocation successful"));

// Optional
// ------------------------------------------------------------------------------
  display.clearDisplay();             // Clear the buffer
  display.setRotation(2);
  display.drawRect(0, 0, display.width(), display.height(), WHITE);
  display.setTextSize(2);             // Normal 1:1 pixel scale
  display.setTextColor(WHITE);        // Draw white text
  display.setCursor(4,15);
  display.print(F("iMagiWorks"));
  display.setTextSize(1);
  display.setCursor(14,44);
  display.print("Power/SWR Meter");
  display.display();
  
  delay(1000);
// ------------------------------------------------------------------------------
  
  
// ADS1115 SETUP
// ------------------------------------------------------------------------------
  ads.setGain(GAIN_ONE);        // ADS1115 - 4.096V / 0.125mV = 32,768 steps
  ads.begin();                  // ADS1015 - 4.096V / 2.000mV =  2,048 steps
  
// Set the sample rate to 860 SPS for ADS1115
// The default sample rate for the ADS1115 ADC is 128 samples per second (SPS).
  ads.setDataRate(RATE_ADS1115_860SPS);
// ------------------------------------------------------------------------------
  

// initialize all the readings to 0
// ------------------------------------------------------------------------------
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readingsFWD[thisReading] = 0;
  }
  for (int thisReading = 0; thisReading < numReadings; thisReading++) {
    readingsRFL[thisReading] = 0;
  }
}
// ------------------------------------------------------------------------------



// ------------------------------------------------------------------------------
void loop()
{
  totalFWD = 0;
  totalRFL = 0;
  peakFWD = 0;
  peakRFL = 0;
    
// Read ADCs - FWD & RFL. The units in this section are all ADS1115 Steps. (32,768 max)
// ------------------------------------------------------------------------------ 
// Fill the arrays with data as fast as possible to maximise 'peak' power capture.
  for (i = 0; i < numReadings; i++){
  readingsFWD[i] = ads.readADC_SingleEnded(0);
  readingsRFL[i] = ads.readADC_SingleEnded(1);
  }
  
// Get MAX readingsFWD and MAX readingsRFL and store in peakFWD and peakRFL.
  for (i = 0; i < numReadings; i++){
    if(readingsFWD[i] > peakFWD) {
      peakFWD = readingsFWD[i];
      peakRFL = readingsRFL[i];

        if(peakFWD > maxFWD) {  // maxFWD is used to calculate the..
          maxFWD = peakFWD;     // ...persistant Max Power reading.
          }
      }                    
    totalFWD = totalFWD + readingsFWD[i];
    totalRFL = totalRFL + readingsRFL[i];
  }
  
// Calc 'average' readings
  averageFWD = totalFWD/numReadings;
  averageRFL = totalRFL/numReadings;
// ------------------------------------------------------------------------------



// Convert ADC Steps to dBm.
// ------------------------------------------------------------------------------ 
PeakPower_dBm_FWD = (peakFWD/SlopeCalc)+TotalAtten;
PeakPower_dBm_RFL = (peakRFL/SlopeCalc)+TotalAtten;
maxPower_dBm_FWD = (maxFWD/SlopeCalc)+TotalAtten;

AveragePower_dBm_FWD = (averageFWD/SlopeCalc)+TotalAtten;
AveragePower_dBm_RFL = (averageRFL/SlopeCalc)+TotalAtten;
// ------------------------------------------------------------------------------ 


// Calculate Power(W) and SWR.
// ------------------------------------------------------------------------------
PeakPower_Watts_FWD = pow(10, PeakPower_dBm_FWD/10.0)/1000;
PeakPower_Watts_RFL = pow(10, PeakPower_dBm_RFL/10.0)/1000;

AvPower_Watts_FWD = pow(10, AveragePower_dBm_FWD/10.0)/1000;

max_Power_Watts_FWD = pow(10, maxPower_dBm_FWD/10.0)/1000;

PeakSWR = (1+sqrt(PeakPower_Watts_RFL/PeakPower_Watts_FWD))/(1-sqrt(PeakPower_Watts_RFL/PeakPower_Watts_FWD));
// ------------------------------------------------------------------------------ 


// Data Display conditions.
// ------------------------------------------------------------------------------
  if(averageFWD < 8800) {   // 8800 is equivalent to approx 0.001 Watts.
    max_Power_Watts_FWD = 0.0;
    }
  if(averageFWD < 9000) {
    PeakPower_Watts_FWD = 0.0;
    PeakSWR = 0.0;
    AvPower_Watts_FWD = 0.0;  
    }
// ------------------------------------------------------------------------------

 oledDisplay();
// serialDataOut();
  
delay(1000);
}
// ------------------------------------------------------------------------------



// Send serial data to PC.
// ------------------------------------------------------------------------------
 void serialDataOut(void) {

  Serial.print(F("averageFWD "));
  Serial.println(averageFWD, 1);
  Serial.print(F("averageRFL "));
  Serial.println(averageRFL, 1);
  Serial.print(F("maxFWD "));
  Serial.println(maxFWD,1);
  Serial.println();

  Serial.print(F("AveragePower_dBm_FWD "));
  Serial.println(AveragePower_dBm_FWD,6);
  Serial.print(F("AveragePower_dBm_RFL "));
  Serial.println(AveragePower_dBm_RFL,6);
  Serial.print(F("AveragePower_Watts_FWD "));
  Serial.println(AvPower_Watts_FWD,6);
  Serial.println();

  Serial.print(F("PeakPower_Watts_FWD "));
  Serial.println(PeakPower_Watts_FWD,6);
  Serial.print(F("PeakPower_Watts_RFL "));
  Serial.println(PeakPower_Watts_RFL,6);
  Serial.print(F("PeakSWR "));
  Serial.println(PeakSWR,6);
  Serial.println();

  Serial.print(F("MAX_Power_Watts_FWD "));
  Serial.println(max_Power_Watts_FWD,6);
  Serial.print(F("maxPower_dBm_FWD "));
  Serial.println(maxPower_dBm_FWD,6);
  Serial.print(F("==========================================="));
  Serial.println();
  Serial.println();
 }
 
// ------------------------------------------------------------------------------

// Display data on OLED.
// ------------------------------------------------------------------------------
 void oledDisplay(void) {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  
  display.setCursor(42,2);
  display.print(F("Peak   Average"));
  display.setCursor(0,14);
  display.print(F("PWR"));
  display.setCursor(45,14);
  display.print(PeakPower_Watts_FWD,3);
  display.setCursor(95,14);
  display.print(AvPower_Watts_FWD,3);
  
  display.setTextSize(2);
  display.setCursor(0,27);
  display.print(F("SWR:"));
  display.setCursor(65,27);
  display.print(PeakSWR,3);
  display.setCursor(0,50);
  display.print(F("PWR:"));
  display.setCursor(65,50);
  display.print(max_Power_Watts_FWD,3);
  
  display.display();
 }
// ------------------------------------------------------------------------------