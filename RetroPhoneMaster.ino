// Wire Master Writer
// by Nicholas Zambetti <http://www.zambetti.com>

// Demonstrates use of the Wire library
// Writes data to an I2C/TWI slave device
// Refer to the "Wire Slave Receiver" example for use with this

// Created 29 March 2006

// This example code is in the public domain.

// TWI: A4 or SDA pin and A5 or SCL pin.

#include <Wire.h>

#define N_DIGIT                                       20  // # of digits (or char's)

void setup()
{
  Serial.begin(9600);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("PhoneControler");
  
  Wire.begin(); // join i2c bus (address optional for master)
}

void loop()
{
  if (Serial.available())
  {
    // get the new byte:
    char inChar = (char)Serial.read();
    Serial.println(inChar);
    
    Wire.beginTransmission(1); // transmit to device #1
    Wire.write(inChar);        // sends bytes
    Wire.endTransmission();    // stop transmitting

    delay(1000);
    
    Serial.println("Expect 21 bytes from i2c read");

    for (int i = 0; i < N_DIGIT+1; i++)
    {
      Wire.requestFrom(1, 1);// request 1 byte from slave device #1
      if(Wire.available())    // slave may send less than requested
      { 
        byte c = Wire.read();    // receive a byte as character
        Serial.print((char) c);
      }
    }
    
  } // if (Serial.available)
  
} // void loop ()



