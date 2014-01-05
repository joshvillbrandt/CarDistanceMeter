// I2C SRF10 or SRF08 Devantech Ultrasonic Ranger Finder
// by Nicholas Zambetti <http://www.zambetti.com>
// and James Tichenor <http://www.jamestichenor.net>

// Demonstrates use of the Wire library reading data from the
// Devantech Utrasonic Rangers SFR08 and SFR10

// Created 29 April 2006

// This example code is in the public domain.

#define MIN_READING 1 // in; throw out values smaller than this
#define MAX_READING 249 // in; throw out values bigger than this
#define MIN_PARK 28 // in; beyond this bound and we might run into the wall!
#define MAX_PARK 44 // in; beyond this bound and we might get hit by the garage door!
#define MIN_PARK_AWESOME 34 // in; sweet spot
#define MAX_PARK 38 // in; sweet spot
#define ACTIVATE_READING 125 // in; when to turn on the output
#define ACTIVE_TIMEOUT 30000 // ms; when to turn the display back off

#include <Wire.h>
#include <Serial7.h>

Serial7 serial7(Serial);
bool active = false;
int reading = 0;
unsigned long activate_start = 0;
int max_reading = 0;

void setup()
{
  // use analog pins 2 and 3 for power
  #define pwrpin PORTC3
  #define gndpin PORTC2
  DDRC |= _BV(pwrpin) | _BV(gndpin);
  PORTC &=~ _BV(gndpin);
  PORTC |=  _BV(pwrpin);
  delay(100);  // wait for things to stabilize  

  Wire.begin(); // join i2c bus (address optional for master)
  Serial.begin(9600);
  
  activate();
}

void loop()
{
  getReading();
  max_reading = max(reading, max_reading);
  
  if(active && (activate_start + ACTIVE_TIMEOUT) < millis()) deactivate();
  
  // get rid of noise
  if(reading > MIN_READING && reading < MAX_READING) {
    if(max_reading > ACTIVATE_READING && reading < ACTIVATE_READING) activate();
    
    if(active)
      serial7.print(reading);   // print the reading
    //Serial.print(reading);
  }
}

int getReading() {
  // step 1: instruct sensor to read echoes
  Wire.beginTransmission(112); // transmit to device #112 (0x70)
  // the address specified in the datasheet is 224 (0xE0)
  // but i2c adressing uses the high 7 bits so it's 112
  Wire.write(byte(0x00));      // sets register pointer to the command register (0x00)
  Wire.write(byte(0x50));      // command sensor to measure in "inches" (0x50)
  // use 0x51 for centimeters
  // use 0x52 for ping microseconds
  Wire.endTransmission();      // stop transmitting

  // step 2: wait for readings to happen
  delay(70);                   // datasheet suggests at least 65 milliseconds

  // step 3: instruct sensor to return a particular echo reading
  Wire.beginTransmission(112); // transmit to device #112
  Wire.write(byte(0x02));      // sets register pointer to echo #1 register (0x02)
  Wire.endTransmission();      // stop transmitting

  // step 4: request reading from sensor
  Wire.requestFrom(112, 2);    // request 2 bytes from slave device #112

  // step 5: receive reading from sensor
  if (2 <= Wire.available())   // if two bytes were received
  {
    reading = Wire.read();  // receive high byte (overwrites previous reading)
    reading = reading << 8;    // shift high byte to be high 8 bits
    reading |= Wire.read(); // receive low byte as lower 8 bits
  }
  
  return reading;
}

void activate() {
  serial7.reset();
  serial7.brightness(0);
  activate_start = millis();
  active = true;
  max_reading = 0;
}

void deactivate() {
  serial7.reset();
  serial7.brightness(254); // not needed now that we are sending blanks
  active = false;
  Serial.write(0x78); // blank characters
  Serial.write(0x78);
  Serial.write(0x78);
  Serial.write(0x78);
  Serial.write(0x77); // decimal points
  Serial.write(0x00); // all off
}

