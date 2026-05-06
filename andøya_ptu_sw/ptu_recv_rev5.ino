#include <Cansat_RFM96.h>

Cansat_RFM96 rfm96(433500, false);//, 10, 8, 9); // Teensy LC for now
uint8_t rx_array[100], rx_counter=0;

float gps_lat, gps_lng, pressure;
uint16_t frame_counter=0, ext_NTC, gps_alt, humidity;
uint8_t gps_numsats, analogBatt, int_NTC, crc_checks;
char tmp;
unsigned long tid_= 0;
unsigned long tid_i = 0;
unsigned long tid = 0;

// CRC lookup table
static const uint8_t CRC_TABLE[256] = {
    0x00, 0x07, 0x0E, 0x09, 0x1C, 0x1B, 0x12, 0x15,
    0x38, 0x3F, 0x36, 0x31, 0x24, 0x23, 0x2A, 0x2D,
    0x70, 0x77, 0x7E, 0x79, 0x6C, 0x6B, 0x62, 0x65,
    0x48, 0x4F, 0x46, 0x41, 0x54, 0x53, 0x5A, 0x5D,
    0xE0, 0xE7, 0xEE, 0xE9, 0xFC, 0xFB, 0xF2, 0xF5,
    0xD8, 0xDF, 0xD6, 0xD1, 0xC4, 0xC3, 0xCA, 0xCD,
    0x90, 0x97, 0x9E, 0x99, 0x8C, 0x8B, 0x82, 0x85,
    0xA8, 0xAF, 0xA6, 0xA1, 0xB4, 0xB3, 0xBA, 0xBD,
    0xC7, 0xC0, 0xC9, 0xCE, 0xDB, 0xDC, 0xD5, 0xD2,
    0xFF, 0xF8, 0xF1, 0xF6, 0xE3, 0xE4, 0xED, 0xEA,
    0xB7, 0xB0, 0xB9, 0xBE, 0xAB, 0xAC, 0xA5, 0xA2,
    0x8F, 0x88, 0x81, 0x86, 0x93, 0x94, 0x9D, 0x9A,
    0x27, 0x20, 0x29, 0x2E, 0x3B, 0x3C, 0x35, 0x32,
    0x1F, 0x18, 0x11, 0x16, 0x03, 0x04, 0x0D, 0x0A,
    0x57, 0x50, 0x59, 0x5E, 0x4B, 0x4C, 0x45, 0x42,
    0x6F, 0x68, 0x61, 0x66, 0x73, 0x74, 0x7D, 0x7A,
    0x89, 0x8E, 0x87, 0x80, 0x95, 0x92, 0x9B, 0x9C,
    0xB1, 0xB6, 0xBF, 0xB8, 0xAD, 0xAA, 0xA3, 0xA4,
    0xF9, 0xFE, 0xF7, 0xF0, 0xE5, 0xE2, 0xEB, 0xEC,
    0xC1, 0xC6, 0xCF, 0xC8, 0xDD, 0xDA, 0xD3, 0xD4,
    0x69, 0x6E, 0x67, 0x60, 0x75, 0x72, 0x7B, 0x7C,
    0x51, 0x56, 0x5F, 0x58, 0x4D, 0x4A, 0x43, 0x44,
    0x19, 0x1E, 0x17, 0x10, 0x05, 0x02, 0x0B, 0x0C,
    0x21, 0x26, 0x2F, 0x28, 0x3D, 0x3A, 0x33, 0x34,
    0x4E, 0x49, 0x40, 0x47, 0x52, 0x55, 0x5C, 0x5B,
    0x76, 0x71, 0x78, 0x7F, 0x6A, 0x6D, 0x64, 0x63,
    0x3E, 0x39, 0x30, 0x37, 0x22, 0x25, 0x2C, 0x2B,
    0x06, 0x01, 0x08, 0x0F, 0x1A, 0x1D, 0x14, 0x13,
    0xAE, 0xA9, 0xA0, 0xA7, 0xB2, 0xB5, 0xBC, 0xBB,
    0x96, 0x91, 0x98, 0x9F, 0x8A, 0x8D, 0x84, 0x83,
    0xDE, 0xD9, 0xD0, 0xD7, 0xC2, 0xC5, 0xCC, 0xCB,
    0xE6, 0xE1, 0xE8, 0xEF, 0xFA, 0xFD, 0xF4, 0xF3
};

void setup() {
  Serial.begin(9600);
  //while(!Serial);
  Serial1.begin(9600);
  
  if (!rfm96.init()) {
    Serial.println("Init failed, but let's go anyways");
    //while(1);
  }

  rfm96.setModem(6, 4, 12);
  
  tid_i = millis();
}

void loop() {
  if (rfm96.available()) {
    tid_ = millis();
    
    rx_counter = 0;
    while (rfm96.available())
      rx_array[rx_counter++] = rfm96.read();
    //Serial.println(rx_counter);
    prepare_rx_data(rx_counter);
	
    tid = (millis()-tid_i)/1000;

    Serial.print(tid);
    Serial.print(", ");
    Serial.print(frame_counter);
    Serial.print(", ");
    Serial.print(gps_lat,8);
    Serial.print(", ");
    Serial.print(gps_lng, 8);
    Serial.print(", ");
    Serial.print(gps_alt/2.0,2);
    Serial.print(", ");
    Serial.print(gps_numsats);
    Serial.print(", ");
    Serial.print(ext_NTC/16.0,3);
    Serial.print(", ");
    Serial.print(analogBatt);
    Serial.print(", ");
    Serial.print(int_NTC);
    Serial.print(", ");
    Serial.print(pressure, 2);
    Serial.print(", ");
    Serial.print(humidity*100.0/65535, 2);
    Serial.print(", ");
    Serial.print(rfm96.last_RSSI());
    Serial.print(", ");
    Serial.print(crc_checks);

    Serial.println();
  }

  if (millis()-tid_ > 5000) {
    tid_ = millis();
    
	tid = (millis()-tid_i)/1000;
	
    Serial.print(tid);
	Serial.println(",,,,,,,,,,,, -1"); // Print empty data line
  }
}

void prepare_rx_data(uint8_t len_) {
  uint8_t store_array_counter=0;

  memcpy(&frame_counter, &rx_array[store_array_counter], sizeof(frame_counter)); store_array_counter += sizeof(frame_counter);
  memcpy(&gps_lat, &rx_array[store_array_counter], sizeof(gps_lat)); store_array_counter += sizeof(gps_lat);
  memcpy(&gps_lng, &rx_array[store_array_counter], sizeof(gps_lng)); store_array_counter += sizeof(gps_lng);
  memcpy(&gps_alt, &rx_array[store_array_counter], sizeof(gps_alt)); store_array_counter += sizeof(gps_alt);
  memcpy(&gps_numsats, &rx_array[store_array_counter], sizeof(gps_numsats)); store_array_counter += sizeof(gps_numsats);
  memcpy(&ext_NTC, &rx_array[store_array_counter], sizeof(ext_NTC)); store_array_counter += sizeof(ext_NTC);
  memcpy(&analogBatt, &rx_array[store_array_counter], sizeof(analogBatt)); store_array_counter += sizeof(analogBatt);
  memcpy(&int_NTC, &rx_array[store_array_counter], sizeof(int_NTC)); store_array_counter += sizeof(int_NTC);
  memcpy(&pressure, &rx_array[store_array_counter], sizeof(pressure)); store_array_counter += sizeof(pressure);
  memcpy(&humidity, &rx_array[store_array_counter], sizeof(humidity)); store_array_counter += sizeof(humidity);
  
  double large_number = 4294967040.0-1;
  if ((gps_lat > large_number) || (gps_lat < -1*large_number))
    gps_lat = -1.0;

  if ((gps_lng > large_number) || (gps_lng < -1*large_number))
    gps_lng = -1.0;

  if ((pressure > 150000) || (pressure < 0))
    pressure = -1.0;

  crc_checks = 0;
  uint8_t crc_tmp;
  crc_tmp = crc8ccitt(rx_array+2, 4+4+2);
  if (crc_tmp == rx_array[store_array_counter])
    crc_checks |= 1;
  store_array_counter += 1;

  crc_tmp = crc8ccitt(rx_array+2+4+4+2, 2+3*1+4+2);
  if (crc_tmp == rx_array[store_array_counter])
    crc_checks |= 2;
  store_array_counter += 1;
}

// CRC-8 public domain library found at:
// https://3dbrew.org/wiki/CRC-8-CCITT
uint8_t crc8ccitt(const void * data, size_t size) {
  uint8_t val = 0;

  uint8_t * pos = (uint8_t *) data;
  uint8_t * end = pos + size;

  while (pos < end) {
    val = CRC_TABLE[val ^ *pos];
    pos++;
  }

  return val;
}
