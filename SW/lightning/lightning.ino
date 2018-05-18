// AS3935 Lightning Detector Example

#include "wiring_private.h"
#include <Wire.h>

#define STROKE 20 // PC4 ATmega 1248
//!!! #define STROKE 8 // ATmega 328P

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

  pinMode(STROKE, INPUT);

  Wire.setClock(100000);

/*
  Wire.beginTransmission(3); // transmit to device #44 (0x2c)
  Wire.write(0);             // sends value byte  
  Wire.write(0x24);             // sends value byte  
  Wire.endTransmission();     // stop transmitting

  Wire.beginTransmission(3); // transmit to device #44 (0x2c)
  Wire.write(1);             // sends value byte  
  Wire.write(0x22);             // sends value byte  
  Wire.endTransmission();     // stop transmitting

  Wire.beginTransmission(3); // transmit to device #44 (0x2c)
  Wire.write(1);             // sends value byte  
  Wire.write(0b0100000);             // sends value byte  
  Wire.endTransmission();     // stop transmitting
  Wire.beginTransmission(3); // transmit to device #44 (0x2c)
//  Wire.write(8);             // sends value byte  
//  Wire.write(0b10000000);             // sends value byte  
// Wire.endTransmission();     // stop transmitting
*/
  Serial.println("#Hmmm...");
}

int lightning[9];
int32_t stroke, stroke_energy;
int32_t counter=0;

void loop()
{             
  while(digitalRead(STROKE))
  {
    delay(2);
    
    // make a string for assembling the data to log:
    String dataString = "$lightning,";
    dataString += String(counter++);  
    dataString += ",";

    Wire.requestFrom(3, (uint8_t)9);    // request 9 bytes from slave device #3

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
    
    Serial.println(dataString);  // print to terminal          
  }   
}










