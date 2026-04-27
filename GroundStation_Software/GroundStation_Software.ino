#include <Cansat_RFM96.h>

#include <SPI.h>
#include <SD.h>

Cansat_RFM96 rfm96(433000, false);
unsigned long time_counter=0;

File LogFile;

uint8_t buffer[512];
int bufferCounter = 0;

void setup() {
  Serial.begin(9600);
  while(!Serial);

  Serial.println("Starting setup");
  
  if (!rfm96.init()) {
    Serial.println("Init of radio failed, stopping");
    while(1);
  }

  Serial.println("Found RFM96 radio, and it is working as expected");
  
  rfm96.setTxPower(20); // +5 dBm, approx 3 mW, which is quite low

  SD.begin();
  LogFile = SD.open("Log.txt", FILE_WRITE);

  Serial.println("End of setup");
  Serial.println();
}

void loop() {
  bufferCounter = 0;
  // We check if there it something in the buffer
  while (rfm96.available()) {
    // We keep track of the time since we last received something,
    // so the user can get feedback if the radio does not receive
    // something
    time_counter = millis();
    
    // Read it into a variable. Here we could just directly use:
    // Serial.write(rfm96.read());
    buffer[bufferCounter] = rfm96.read();
    bufferCounter++;

    // Write it to file. We do not use Serial.print, because the
    // conversion to readable ASCII has already been done when
    // we send it
  }

  if (bufferCounter > 0){
    //Serial.write(buffer, bufferCounter);
    uint16_t frame;
    int16_t temperature;
    float pressure;
    float accel_X;
    float accel_Y;
    float accel_Z;

    memcpy(&frame, &buffer[0], sizeof(uint16_t));
    memcpy(&temperature, &buffer[2], sizeof(int16_t));
    memcpy(&pressure, &buffer[4], sizeof(float));
    memcpy(&accel_X, &buffer[8], sizeof(float));
    memcpy(&accel_Y, &buffer[12], sizeof(float));
    memcpy(&accel_Z, &buffer[16], sizeof(float));

    Serial.println(bufferCounter);
    Serial.print(frame);
    Serial.print(", ");
    Serial.print(temperature); 
    Serial.print(", ");
    Serial.print(pressure); 
    Serial.print(", ");
    Serial.print(accel_X); 
    Serial.print(", ");
    Serial.print(accel_Y); 
    Serial.print(", ");
    Serial.print(accel_Z); 

    Serial.println();
  }

  if (millis()-time_counter > 5000) {
    time_counter = millis();
    Serial.println("We have not received anything in 5 seconds");
  }
}
