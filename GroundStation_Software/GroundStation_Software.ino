#include <Cansat_RFM96.h>

Cansat_RFM96 rfm96(433500, false);
unsigned long time_counter=0;

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

  Serial.println("End of setup");
  Serial.println();
}

void loop() {
  bufferCounter = 0;
  // We check if there it something in the buffer
  while (rfm96.available()) 
  {
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
    float latitude;
    float longitude;
    uint16_t altitude;
    float temperature;
    float pressure;
    uint16_t TotalCount;
    uint16_t MuonCount;
    float accel_X;
    float accel_Y;
    float accel_Z;
    float gyro_X;
    float gyro_Y;
    float gyro_Z;
    int8_t crc_GPS;
    int8_t crc_Telemetry;

    uint16_t tempUnsigned;
    int16_t tempSigned;

    memcpy(&frame, &buffer[0], sizeof(uint16_t));
    memcpy(&latitude, &buffer[2], sizeof(float));
    memcpy(&longitude, &buffer[6], sizeof(float));
    memcpy(&altitude, &buffer[10], sizeof(uint16_t));

    memcpy(&tempUnsigned, &buffer[12], sizeof(uint16_t));
    temperature = (float)tempUnsigned;
    temperature /= 10.f;
    
    memcpy(&pressure, &buffer[14], sizeof(float));
    memcpy(&TotalCount, &buffer[18], sizeof(uint16_t));
    memcpy(&MuonCount, &buffer[20], sizeof(uint16_t));
    memcpy(&accel_X, &buffer[22], sizeof(float));
    memcpy(&accel_Y, &buffer[26], sizeof(float));
    memcpy(&accel_Z, &buffer[30], sizeof(float));

    memcpy(&tempSigned, &buffer[34], sizeof(int16_t));
    gyro_X = (float)tempSigned;
    gyro_X /= 10.f;
    memcpy(&tempSigned, &buffer[36], sizeof(int16_t));
    gyro_Y = (float)tempSigned;
    gyro_Y /= 10.f;
    memcpy(&tempSigned, &buffer[38], sizeof(int16_t));
    gyro_Z = (float)tempSigned;
    gyro_Z /= 10.f;


    memcpy(&crc_GPS, &buffer[40], sizeof(int8_t));
    memcpy(&crc_Telemetry, &buffer[41], sizeof(int8_t));

    Serial.print(frame);
    Serial.print(", ");
    Serial.print(latitude);
    Serial.print(", ");
    Serial.print(longitude);
    Serial.print(", ");
    Serial.print(altitude);
    Serial.print(", ");
    Serial.print(temperature); 
    Serial.print(", ");
    Serial.print(pressure); 
    Serial.print(", ");
    Serial.print(TotalCount); 
    Serial.print(", ");
    Serial.print(MuonCount); 
    Serial.print(", ");
    Serial.print(accel_X); 
    Serial.print(", ");
    Serial.print(accel_Y); 
    Serial.print(", ");
    Serial.print(accel_Z);
    Serial.print(", ");
    Serial.print(gyro_X); 
    Serial.print(", ");
    Serial.print(gyro_Y); 
    Serial.print(", ");
    Serial.print(gyro_Z);
    Serial.print(", ");
    Serial.print(crc_GPS); 
    Serial.print(", ");
    Serial.print(crc_Telemetry); 

    Serial.println();
  }

  if (millis()-time_counter > 5000) 
  {
    time_counter = millis();
    Serial.println("We have not received anything in 5 seconds");
  }
}
