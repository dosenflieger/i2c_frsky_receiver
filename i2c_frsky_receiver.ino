//
// Forward telemetry data received from MultiWii via I2C over an 
// inverted serial connection to FrSky D8R-ii plus receiver using
// an Arduino Mini Pro.
//
// (c) 2015-2016  Lars Wessels <github@dosenflieger.de>
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include <Wire.h>

#define I2C_ADDRESS           0x30
#define I2C_TELEMETRY_HEADER  0x11
#define I2C_TELEMETRY_TAIL    0x12
#define FRSKY_HEADER          0x5e
#define FRSKY_TAIL            0x5e
#define FRSKY_BUFFER          32
#define I2C_BUFFER            32   // must match buffer size in MultiWii/Telemetry.cpp !!!
#define TX_LED                13

//#define SERIAL_DEBUG


static byte frsky_buffer[FRSKY_BUFFER];   // serial output buffer for FrSky frame
static byte i2c_buffer[I2C_BUFFER];       // buffer for incoming i2c frames (max. 4 telemetry frames)
static uint8_t frsky_bytes = 0;   // bytes waiting in serial buffer for FrSky receiver
volatile uint8_t i2c_bytes = 0;   // bytes received via i2c
volatile uint8_t i2c_lock = 0;    // lock i2c buffer when turned into FrSky serial frame


// turn i2c data into FrSky telemetry frame to be sent over serial line
void frskySerialize() {
  uint8_t i = 0, j = 0;
  byte data;
  
  i2c_lock++; // make sure i2c buffer doesn't change during processing
  memset(frsky_buffer, 0, FRSKY_BUFFER);
  frsky_buffer[j++] = FRSKY_HEADER;
    
  // repeat until no more telemetry frames left in i2c buffer
  while (i2c_buffer[i] == I2C_TELEMETRY_HEADER) {
    i2c_buffer[i++] = 0x00;  // to skip if() in loop()    
    frsky_buffer[j++] = i2c_buffer[i++];  // data id

    // handle low and high data bytes and possible byte stuffing 
    while (++i < i2c_bytes && i < FRSKY_BUFFER) {
      data = i2c_buffer[i-1];
      if (data == 0x5e) {
        frsky_buffer[j++] = 0x5d;
        frsky_buffer[j++] = 0x3e;
      } else if (data == 0x5d) {
        frsky_buffer[j++] = 0x5d;
        frsky_buffer[j++] = 0x3d;
      } else {
        frsky_buffer[j++] = data;
      }
      if (i%3 == 1) {
        frsky_buffer[j++] = FRSKY_TAIL;  // terminate current FrSyk frame
        if (i+1 < i2c_bytes) {
          frsky_buffer[j++] = FRSKY_HEADER; // start new FrSky frame
        }
      }
    }
  }
  
  // check for i2c telemetry tail
  if (i2c_buffer[i-1] == I2C_TELEMETRY_TAIL) {
    frsky_buffer[j] = '\0';  // terminate valid FrSyk frame
    frsky_bytes = j;
  } else {
    memset(frsky_buffer, 0, FRSKY_BUFFER);  // discard invalid FrSyk frame
    frsky_bytes = 0;
  }
  
  i2c_lock = 0;  // unlock i2c_buffer
}


void setup() {
  Wire.begin(I2C_ADDRESS);      // join i2c bus
  Wire.onReceive(receiveEvent); // register event handler
  Serial.begin(9600);           // serial output speed to FrSky receiver
  pinMode(TX_LED, OUTPUT);
}

void loop() {
  uint8_t  i,j;
  
  // check for new telemetry data waiting in i2c buffer
  if (i2c_bytes && i2c_buffer[0] == I2C_TELEMETRY_HEADER) {
    frskySerialize();
    memset(i2c_buffer, 0, I2C_BUFFER);
#ifdef SERIAL_DEBUG
    Serial.print("FRSKY("); Serial.print(frsky_bytes); Serial.print("): ");
    for (i = 0; i < frsky_bytes; i++) {
      Serial.print(frsky_buffer[i], HEX);
      Serial.print(" ");
    }
    Serial.println();
#else    
    for (i = 0; i < frsky_bytes; i++) {
      char c = frsky_buffer[i]; // casting byte to char
      Serial.print(c);
    }
#endif
    digitalWrite(TX_LED, HIGH);
    delay(50);
    digitalWrite(TX_LED, LOW);
  }
  
  delay(10);
}

// function that executes whenever i2c data is received 
void receiveEvent(int bytes) {
  uint8_t i = 0;

#ifdef SERIAL_DEBUG
  Serial.print("I2C("); Serial.print(bytes); Serial.print("): ");
  while (Wire.available() && i < I2C_BUFFER) {
    if (!i2c_lock) {
      i2c_buffer[i] = Wire.read();
      Serial.print(i2c_buffer[i++], HEX);
      Serial.print(" ");
    } else {
      Wire.read();
      Serial.print("--");
      Serial.print(" ");
    }
  }
  Serial.println();
#else
  while (Wire.available() && i < I2C_BUFFER) {  // loop through all
    if (!i2c_lock) {  // only update i2c buffer while not being serialized
      i2c_buffer[i++] = Wire.read();  // store in byte arrray
    } else {
      Wire.read(); // discard data but also avoids blocking i2c bus
    }
  }
#endif
  i2c_bytes = bytes;
}
