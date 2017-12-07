/*
  CANDY based on Mighty 1284p
 
Compiled with: https://github.com/MCUdude/MightyCore.git

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

#include <SD.h>               // Tested version 1.0.6
#include "wiring_private.h"
#include <SoftwareSerial.h>

#define MSG_NO 20    // number of logged NMEA messages

#define RELE_ON     11    // PD3
#define RELE_OFF    12    // PD4
#define LED_yellow  23    // PC7

#define CHANNELS 512 // number of channels in buffer for histogram, including negative numbers

SoftwareSerial swSerial(18, 19); // RX, TX

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

  swSerial.begin(115200);
  //mySerial.println("#Cvak...");

  swSerial.print("#Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) 
  {
    swSerial.println("#Card failed, or not present");
    // don't do anything more:
    return;
  }
  swSerial.println("#card initialized.");
  
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
  uint8_t analog_reference = EXTERNAL; // DEFAULT, INTERNAL, INTERNAL1V1, INTERNAL2V56, or EXTERNAL

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


  for(int i=0; i<20; i++)  
  {
    delay(50);
    digitalWrite(LED_yellow, LOW);  // Blink for Dasa 
    delay(50);
    digitalWrite(LED_yellow, HIGH);  
  }
}

#define MEASUREMENTS  25   // cca 5 minutes of radiation measurement

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

    for(uint16_t n=0; n<65535; n++) // cca 12 s
    {
      for (int i=0; i<15; i++) {digitalWrite(RESET, LOW);} // whole integration cca 200 us

      // start the conversion
      sbi(ADCSRA, ADSC);          // Sample/Hold

      // delay for hold
      __asm__("nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t"); 
      __asm__("nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t"); 
      __asm__("nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t"); 
      __asm__("nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t"); 
      __asm__("nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t"); 
      __asm__("nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t"); 
      __asm__("nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t"); 
      __asm__("nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t"); 
      __asm__("nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t"); 
      __asm__("nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t"); 
      __asm__("nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t"); 
      __asm__("nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t"); 
      __asm__("nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t"); 
      __asm__("nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t"); 
      __asm__("nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t nop\n\t"); 
      digitalWrite(RESET, HIGH);  // Reset peak detector (start next measurement)
      digitalWrite(RESET, LOW); 

      // ADSC is cleared when the conversion finishes
      while (bit_is_set(ADCSRA, ADSC));  // conversion cca 100 us

      //  digitalWrite(LED_yellow, LOW);  // Blink for debug
      //  digitalWrite(LED_yellow, HIGH);  
      
      // we have to read ADCL first; doing so locks both ADCL
      // and ADCH until ADCH is read.  reading ADCL second would
      // cause the results of each conversion to be discarded,
      // as ADCL and ADCH would be locked when it completed.
      lo = ADCL;
      hi = ADCH;

      // combine the two bytes
      u_sensor = (hi << 7) | (lo >> 1);

      // manage negative values
      //if (u_sensor <= 511 ) {u_sensor += 512;} else {u_sensor -= 512;}
      if (u_sensor <= (CHANNELS/2)-1 ) {u_sensor += (CHANNELS/2);} else {u_sensor -= (CHANNELS/2);}
      

      //buffer[u_sensor]++;

            
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

      #define NOISE (CHANNELS/2) + 2
      #define BACKGROUND 7
        
      int loDose = 0;
      for(int n=NOISE; n<(NOISE+BACKGROUND); n++)
      {
        loDose += buffer[n];
      }
    
      int hiDose=0;
      for(int n=(NOISE+BACKGROUND); n<CHANNELS ; n++)
      {
        hiDose += buffer[n];
      }
      
      int noisenoise=0;
      for(int n=0; n<NOISE ; n++)
      {
        noisenoise += buffer[n];
      }

      int dose = loDose+hiDose;
      dataString += String(dose) + "," + String(loDose) + "," + String(hiDose) + "," + String(noisenoise) + "," + String(supress) + "," + String(dose+noisenoise+supress);
    
      // open the file. note that only one file can be open at a time,
      // so you have to close this one before opening another.
      File dataFile = SD.open("datalog.txt", FILE_WRITE);
    
      // if the file is available, write to it:
      if (dataFile) 
      {
        dataFile.println(dataString);  // write to SDcard
        swSerial.println(dataString);  // print to terminal
        
        digitalWrite(LED_yellow, LOW);  // Blink for Dasa
        delay(10);
        digitalWrite(LED_yellow, HIGH);  
        if (hiDose > 0)
        {
          delay(150);
          digitalWrite(LED_yellow, LOW);  // Blink for Dasa + zaric
          delay(10);
          digitalWrite(LED_yellow, HIGH);  
        }
        
        dataFile.close();
      }  
      // if the file isn't open, pop up an error:
      else 
      {
        swSerial.println("#error opening datalog.txt");
      }
    }  
  }    
  
  {
    digitalWrite(RELE_OFF, LOW);  // Rele switch ON
    digitalWrite(RELE_ON, HIGH);  
    delay(200);
    digitalWrite(RELE_ON, LOW);
    
    delay(1000);  // Start GPS 

    {
      // Switch off Galileo and GLONASS; 68 configuration bytes
      const char cmd[0x3C + 8]={0xB5, 0x62, 0x06, 0x3E, 0x3C, 0x00, 0x00, 0x20, 0x20, 0x07, 0x00, 0x08, 0x10, 0x00, 0x01, 0x00, 0x01, 0x01, 0x01, 0x01, 0x03, 0x00, 0x01, 0x00, 0x01, 0x01, 0x02, 0x04, 0x08, 0x00, 0x00, 0x00, 0x01, 0x01, 0x03, 0x08, 0x10, 0x00, 0x00, 0x00, 0x01, 0x01, 0x04, 0x00, 0x08, 0x00, 0x00, 0x00, 0x01, 0x03, 0x05, 0x00, 0x03, 0x00, 0x00, 0x00, 0x01, 0x05, 0x06, 0x08, 0x0E, 0x00, 0x00, 0x00, 0x01, 0x01, 0x53, 0x1F};
      for (int n=0;n<(0x3C + 8);n++) Serial.write(cmd[n]); 
    }
          
    {
      // airborne <2g; 44 configuration bytes
      const char cmd[0x24 + 8]={0xB5, 0x62 ,0x06 ,0x24 ,0x24 ,0x00 ,0xFF ,0xFF ,0x07 ,0x03 ,0x00 ,0x00 ,0x00 ,0x00 ,0x10 ,0x27 , 0x00 ,0x00 ,0x05 ,0x00 ,0xFA ,0x00 ,0xFA ,0x00 ,0x64 ,0x00 ,0x5E ,0x01 ,0x00 ,0x3C ,0x00 ,0x00 , 0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x85 ,0x2A};
      for (int n=0;n<(0x24 + 8);n++) Serial.write(cmd[n]); 
    }
    
    delay(26000);  // Waiting for GPS fix
      
    // make a string for assembling the NMEA to log:
    String dataString = "";
    char incomingByte; 
    
    for(int n=0; n<30000; n++)    // flush USART buffer
    {
      if (Serial.available()) 
      {
        // read the incoming byte:
        incomingByte = Serial.read();
      }
    }
    
    int parse = 0;
    for(int n=0; n<30000; n++)    // Skip first GPRMC
    {
      if (Serial.available()) 
      {
        // read the incoming byte:
        incomingByte = Serial.read();
        switch (incomingByte) 
        {
          case 'C':
            parse=1;
            break;
          case ',':
            if (parse==1) parse=2;
            break;
          default:
            parse=0;
        }       
      }
      if (parse==2) break;
    }
    
    boolean flag = false;
    int messages = 0;
    for(int n=0; n<30000; n++)
    {
      if (Serial.available()) 
      {
        // read the incoming byte:
        incomingByte = Serial.read();
        
        if (incomingByte == '$') {flag = true; messages++;};
        
        // say what you got:
        if (flag && (messages<=MSG_NO)) dataString+=incomingByte;
      }
      for (int i=0; i<50; i++) {digitalWrite(RESET, LOW);} // delay cca 250 us
      if (messages > MSG_NO) break;
    }

    // open the file. note that only one file can be open at a time,
    // so you have to close this one before opening another.
    File dataFile = SD.open("datalog.txt", FILE_WRITE);
  
    // if the file is available, write to it:
    if (dataFile) 
    {
      dataFile.print(dataString);  // write to SDcard
      swSerial.print(dataString);  // print to terminal
      dataFile.close();
    }  
    // if the file isn't open, pop up an error:
    else 
    {
      swSerial.println("#error opening datalog.txt");
    }
    
    digitalWrite(RELE_ON, LOW);  // Rele switch OFF
    digitalWrite(RELE_OFF, HIGH);  
    delay(200);
    digitalWrite(RELE_OFF, LOW);
  }
}









