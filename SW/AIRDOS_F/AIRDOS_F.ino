// 3 mesice s LS 33600 = 7.6 mA
/*
  AIRDOS with RTC
 
Compiled with: Arduino 1.8.5

ISP
---
PD0     RX
PD1     TX
RESET#  through 50M capacitor to RST#

SDcard
------
DAT3   SS   4 B4
CMD    MOSI 5 B5
DAT0   MISO 6 B6
CLK    SCK  7 B7

ANALOG
------
+      A0  PA0
-      A1  PA1
RESET  0   PB0

LED
---
LED_yellow  23  PC7         // LED for Dasa


                     Mighty 1284p    
                      +---\/---+
           (D 0) PB0 1|        |40 PA0 (AI 0 / D24)
           (D 1) PB1 2|        |39 PA1 (AI 1 / D25)
      INT2 (D 2) PB2 3|        |38 PA2 (AI 2 / D26)
       PWM (D 3) PB3 4|        |37 PA3 (AI 3 / D27)
    PWM/SS (D 4) PB4 5|        |36 PA4 (AI 4 / D28)
      MOSI (D 5) PB5 6|        |35 PA5 (AI 5 / D29)
  PWM/MISO (D 6) PB6 7|        |34 PA6 (AI 6 / D30)
   PWM/SCK (D 7) PB7 8|        |33 PA7 (AI 7 / D31)
                 RST 9|        |32 AREF
                VCC 10|        |31 GND
                GND 11|        |30 AVCC
              XTAL2 12|        |29 PC7 (D 23)
              XTAL1 13|        |28 PC6 (D 22)
      RX0 (D 8) PD0 14|        |27 PC5 (D 21) TDI
      TX0 (D 9) PD1 15|        |26 PC4 (D 20) TDO
RX1/INT0 (D 10) PD2 16|        |25 PC3 (D 19) TMS
TX1/INT1 (D 11) PD3 17|        |24 PC2 (D 18) TCK
     PWM (D 12) PD4 18|        |23 PC1 (D 17) SDA
     PWM (D 13) PD5 19|        |22 PC0 (D 16) SCL
     PWM (D 14) PD6 20|        |21 PD7 (D 15) PWM
                      +--------+
*/

#include <SD.h>             // Tested with version 1.2.2.
#include "wiring_private.h"
#include <Wire.h>           // Tested with version 1.0.0.
#include "RTClib.h"         // Tested with version 1.5.4.

#define LED_yellow  23 // PC7
#define RESET     0    // PB0
#define SDpower1  1    // PB1
#define SDpower2  2    // PB2
#define SDpower3  3    // PB3
#define SS        4    // PB4
#define MOSI      5    // PB5
#define MISO      6    // PB6
#define SCK       7    // PB7
#define INT       20   // PC4

#define CHANNELS 512 // number of channels in buffer for histogram, including negative numbers

uint16_t count = 0;

RTC_Millis rtc;

void setup()
{

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) 
  {
  ; // wait for serial port to connect. Needed for Leonardo only?
  }

  Serial.println("#Cvak...");
  
// Read Analog Differential without gain (read datashet of ATMega1280 and ATMega2560 for refference)
// Use analogReadDiff(NUM)
//   NUM	|	POS PIN	  	        |	NEG PIN		        | 	GAIN
//	0	|	A0			|	A1			|	1x
//	1	|	A1			|	A1			|	1x
//	2	|	A2			|	A1			|	1x
//	3	|	A3			|	A1			|	1x
//	4	|	A4			|	A1			|	1x
//	5	|	A5			|	A1			|	1x
//	6	|	A6			|	A1			|	1x
//	7	|	A7			|	A1			|	1x
//	8	|	A8			|	A9			|	1x
//	9	|	A9			|	A9			|	1x
//	10	|	A10			|	A9			|	1x
//	11	|	A11			|	A9			|	1x
//	12	|	A12			|	A9			|	1x
//	13	|	A13			|	A9			|	1x
//	14	|	A14			|	A9			|	1x
//	15	|	A15			|	A9			|	1x
  #define pin 0
  uint8_t analog_reference = INTERNAL2V56; // DEFAULT, INTERNAL, INTERNAL1V1, INTERNAL2V56, or EXTERNAL

  ADMUX = (analog_reference << 6) | ((pin | 0x10) & 0x1F);
  
  ADCSRB = 0;               // Switching ADC to Free Running mode
  sbi(ADCSRA, ADATE);       // ADC autotrigger enable (mandatory for free running mode)
  sbi(ADCSRA, ADSC);        // ADC start the first conversions
  sbi(ADCSRA, 2);           // 0x100 = clock divided by 16, 62.5 kHz, 208 us for 13 cycles of one AD conversion, 24 us fo 1.5 cycle for sample-hold
  cbi(ADCSRA, 1);        
  cbi(ADCSRA, 0);        

  pinMode(RESET, OUTPUT);   // reset for peak detetor

  pinMode(INT, INPUT);      // interrupt from lightning detector
     
  pinMode(SDpower1, OUTPUT);  // SDcard interface
  pinMode(SDpower2, OUTPUT);     
  pinMode(SDpower3, OUTPUT);     
  pinMode(SS, OUTPUT);     
  pinMode(MOSI, INPUT);     
  pinMode(MISO, INPUT);     
  pinMode(SCK, OUTPUT);  

  PORTB = 0b00000000;  // SDcard Power OFF

  pinMode(LED_yellow, OUTPUT);
  digitalWrite(LED_yellow, HIGH);  
  digitalWrite(RESET, LOW);  
  
  //!!! Wire.setClock(100000);

  for(int i=0; i<5; i++)  
  {
    delay(50);
    digitalWrite(LED_yellow, LOW);  // Blink for Dasa 
    delay(50);
    digitalWrite(LED_yellow, HIGH);  
  }
  
  Serial.println("#Hmmm...");
}


void loop()
{
  uint8_t lo, hi;
  uint16_t u_sensor, maximum;
  uint16_t buffer[CHANNELS];
  
  for(int n=0; n<CHANNELS; n++)
  {
    buffer[n]=0;
  }
  
  while (bit_is_clear(ADCSRA, ADIF)); // wait for conversion 
  sbi(ADCSRA, ADIF);                  // reset interrupt flag
  while (bit_is_clear(ADCSRA, ADIF)); // wait for conversion 
  delayMicroseconds(24);              // wait for 1.5 cycle of 62.5 kHz ADC clock for sample/hold
  PORTB = 1;                          // Reset peak detector
  asm("NOP");                         // cca 3 us
  PORTB = 0;
  sbi(ADCSRA, ADIF);        // reset interrupt flag from ADC

  uint16_t suppress = 0;      
  uint8_t stroke_time = 0;      
  //!!! boolean stroke_detected = false;

  // dosimeter integration
  for (uint8_t i=0; i<10; i++)
  {
    for (uint16_t n=0; n<4096; n++) // cca 1 s
    {      
      while (bit_is_clear(ADCSRA, ADIF)); // wait for conversion 
      delayMicroseconds(24);              // wait for 1.5 cycle of 62.5 kHz ADC clock for sample/hold
      PORTB = 1;                          // Reset peak detector
      asm("NOP");                         // cca 3 us
      PORTB = 0;
      sbi(ADCSRA, ADIF);        // reset interrupt flag from ADC
  
      // we have to read ADCL first; doing so locks both ADCL
      // and ADCH until ADCH is read.  reading ADCL second would
      // cause the results of each conversion to be discarded,
      // as ADCL and ADCH would be locked when it completed.
      lo = ADCL;
      hi = ADCH;
  
      // combine the two bytes
      u_sensor = (hi << 7) | (lo >> 1);
  
      // manage negative values
      if (u_sensor <= (CHANNELS/2)-1 ) {u_sensor += (CHANNELS/2);} else {u_sensor -= (CHANNELS/2);}
                
      if (u_sensor > maximum) // filter double detection for pulses between two samples
      {
        maximum = u_sensor;
        suppress++;
      }
      else
      {
        buffer[maximum]++;
        maximum = 0;
      }
    }
    
    stroke_time++;
    if (digitalRead(INT))
    {
      break;
    }
  }  
  // Data out
  {
    DateTime now = rtc.now();

    // make a string for assembling the data to log:
    String dataString = "$CANDY,";

    dataString += String(count++); 
    dataString += ",";
  
    dataString += String(now.unixtime()); 
    dataString += ",";

    for(int n=CHANNELS/2; n<CHANNELS; n++)  
    {
      dataString += String(buffer[n]); 
      dataString += ",";
    }

    dataString += String(suppress);
    dataString += ",";

    dataString += String(stroke_time);
    dataString += ",";

    {
      uint32_t stroke, stroke_energy;
      int lightning[9];

      delay(2); // minimal delay after stroke interrupt

      Wire.requestFrom((uint8_t)3, (uint8_t)9);    // request 9 bytes from slave device #3

      for (int8_t reg=0; reg<9; reg++)
      { 
        lightning[reg] = Wire.read();    // receive a byte
        dataString += String(lightning[reg], HEX); 
        dataString += ",";
      }
  
      stroke_energy = lightning[4];
      stroke = lightning[5];
      stroke_energy += stroke << 8;
      stroke = lightning[6] & 0b11111;
      stroke_energy += stroke << 16;
  
      dataString += String(stroke_energy);  // Energy of single stroke
      dataString += ",";
    
      dataString += String(lightning[7] & 0b111111);  // Distance from storm
    }
  

    {

      PORTB = 0b00001110;  // SDcard Power ON
      pinMode(MOSI, OUTPUT);     

      // make sure that the default chip select pin is set to output
      // see if the card is present and can be initialized:
      if (!SD.begin(SS)) 
      {
        Serial.println("#Card failed, or not present");
        // don't do anything more:
        return;
      }

      // open the file. note that only one file can be open at a time,
      // so you have to close this one before opening another.
      File dataFile = SD.open("datalog.txt", FILE_WRITE);
    
      // if the file is available, write to it:
      if (dataFile) 
      {
        dataFile.println(dataString);  // write to SDcard
        
        digitalWrite(LED_yellow, LOW);  // Blink for Dasa
        
        dataFile.close();
      }  
      // if the file isn't open, pop up an error:
      else 
      {
        Serial.println("#error opening datalog.txt");
      }

      pinMode(MOSI, INPUT);      
      PORTB = 0b00000000;  // SDcard Power OFF
    }  
        
    Serial.println(dataString);  // print to terminal
    digitalWrite(LED_yellow, HIGH);  
  }    
}









