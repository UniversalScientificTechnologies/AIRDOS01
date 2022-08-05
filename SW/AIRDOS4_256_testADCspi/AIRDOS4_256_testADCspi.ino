// 3 mesice s LS 33600 = 7.6 mA
/*
  CANDY based on Mighty 1284p with RTC
 
Compiled with: Arduino 1.8.3

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

SERIAL 2 (not necessary to connect)
--------
RX 18 PC2
TX 19 PC3

ANALOG
------
+      A0  PA0
-      A1  PA1
RESET  0   PB0

RELE
----
RELE_ON   11    PD3
RELE_OFF  12    PD4

LED
---
LED_yellow  23  PC7         // LED pro Dasu
LED_green   TIMEPULSE GPS   // LED pro pilota

The following needs to be added to the line mentioning the atmega644 in
/usr/share/arduino/libraries/SD/utility/Sd2PinMap.h:

    || defined(__AVR_ATmega1284P__)

so that it reads:

    #elif defined(__AVR_ATmega644P__) || defined(__AVR_ATmega644__)|| defined(__AVR_ATmega1284P__) 
    
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

#include "wiring_private.h"
//#include <SoftwareSerial.h>


#define MSG_NO 20    // number of logged NMEA messages

#define RELE_ON     11    // PD3
#define RELE_OFF    12    // PD4
#define LED_yellow  23    // PC7

#define CONV  1
#define SCK   2
#define SDI   3

#define CHANNELS 512 // number of channels in buffer for histogram, including negative numbers

//SoftwareSerial swSerial(18, 19); // RX, TX

const int chipSelect = 4;
int RESET = 0;
uint16_t count = 0;


void setup()
{

  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) 
  {
  ; // wait for serial port to connect. Needed for Leonardo only?
  }

  //swSerial.begin(115200);
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

  pinMode(RESET, OUTPUT);     
  pinMode(RELE_ON, OUTPUT);
  pinMode(RELE_OFF, OUTPUT);
  pinMode(LED_yellow, OUTPUT);
  digitalWrite(LED_yellow, HIGH);  
  
  digitalWrite(RELE_ON, LOW);  // Rele switch OFF
  digitalWrite(RELE_OFF, HIGH);  
  delay(500);
  digitalWrite(RELE_OFF, LOW);

  pinMode(CONV, OUTPUT);
  pinMode(SCK, OUTPUT);
  pinMode(SDI, INPUT);
  digitalWrite(CONV, LOW);
  digitalWrite(SCK, HIGH);

  for(int i=0; i<20; i++)  
  {
    delay(50);
    digitalWrite(LED_yellow, LOW);  // Blink for Dasa 
    delay(50);
    digitalWrite(LED_yellow, HIGH);  
  }
  
  Serial.println("#Hmmm...");
}

#define MEASUREMENTS  25  // cca 5 minutes of radiation measurement

void loop()
{
  for(int x=0; x<MEASUREMENTS; x++)  
  {
    uint8_t lo, hi;
    uint16_t u_sensor;
    //int16_t s_sensor;
    uint16_t buffer[CHANNELS];
    
    for(int n=0; n<CHANNELS; n++)
    {
      buffer[n]=0;
    }
    
    digitalWrite(RESET, HIGH);   // Reset peak detector
    digitalWrite(RESET, HIGH);   // Reset peak detector
    digitalWrite(RESET, HIGH);   // Reset peak detector
    digitalWrite(RESET, HIGH);   // Reset peak detector
    digitalWrite(RESET, HIGH);   // Reset peak detector
    digitalWrite(RESET, HIGH);   // Reset peak detector
    digitalWrite(RESET, HIGH);   // Reset peak detector
    digitalWrite(RESET, HIGH);   // Reset peak detector
    digitalWrite(RESET, HIGH);   // Reset peak detector
    digitalWrite(RESET, HIGH);   // Reset peak detector
    for (int i=0; i<20; i++) {digitalWrite(RESET, LOW);} // compensate first data aquisition cca 100 us

    uint16_t maximum = 0;  
    uint16_t supress = 0;      

    //!!! for(uint16_t n=0; n<65535; n++) // cca 12 s
    for(uint16_t n=0; n<65535; n++)
    {
      for (int i=0; i<15; i++) {digitalWrite(RESET, LOW);} // whole integration cca 200 us

      // Hold peak detector
      //pinMode(RESET, OUTPUT);     
      //digitalWrite(RESET, LOW); 
      //digitalWrite(RESET, HIGH); 

      digitalWrite(CONV, HIGH);
      delayMicroseconds(5);
      digitalWrite(CONV, LOW);
      digitalWrite(RESET, HIGH); 
      digitalWrite(RESET, LOW); 

      u_sensor = 0;
      digitalWrite(SCK, LOW);
      digitalWrite(SCK, HIGH);
      if (digitalRead(SDI)) bitSet(u_sensor, 8);
      digitalWrite(SCK, LOW);
      digitalWrite(SCK, HIGH);
      if (digitalRead(SDI)) bitSet(u_sensor, 7);
      digitalWrite(SCK, LOW);
      digitalWrite(SCK, HIGH);
      if (digitalRead(SDI)) bitSet(u_sensor, 6);
      digitalWrite(SCK, LOW);
      digitalWrite(SCK, HIGH);
      if (digitalRead(SDI)) bitSet(u_sensor, 5);
      digitalWrite(SCK, LOW);
      digitalWrite(SCK, HIGH);
      if (digitalRead(SDI)) bitSet(u_sensor, 4);
      digitalWrite(SCK, LOW);
      digitalWrite(SCK, HIGH);
      if (digitalRead(SDI)) bitSet(u_sensor, 3);
      digitalWrite(SCK, LOW);
      digitalWrite(SCK, HIGH);
      if (digitalRead(SDI)) bitSet(u_sensor, 2);
      digitalWrite(SCK, LOW);
      digitalWrite(SCK, HIGH);
      if (digitalRead(SDI)) bitSet(u_sensor, 1);
      digitalWrite(SCK, LOW);
      digitalWrite(SCK, HIGH);
      if (digitalRead(SDI)) bitSet(u_sensor, 0);


      // manage negative values
      //if (u_sensor <= 511 ) {u_sensor += 512;} else {u_sensor -= 512;}
      //if (u_sensor <= (CHANNELS/2)-1 ) {u_sensor += (CHANNELS/2);} else {u_sensor -= (CHANNELS/2);}
      

           
      if (u_sensor > maximum) // suppress double detection for long pulses
      {
        maximum = u_sensor;
        supress++;
      }
      else
      {
        buffer[maximum]++;
        maximum = 0;
      }
    }
    
  
    {

      // make a string for assembling the data to log:
      String dataString = "$CANDY,";


      dataString += String(count++); 
      dataString += ",";
    
      for(int n=0; n<CHANNELS; n++)  
      {
        dataString += String(buffer[n]); 
        dataString += ",";
      }
      dataString += String(supress);
    
      {
        Serial.println(dataString);  // print to terminal
        
        digitalWrite(LED_yellow, LOW);  // Blink for Dasa
        delay(10);
        digitalWrite(LED_yellow, HIGH);  
        
      }  
    }  
  }    
  
}









