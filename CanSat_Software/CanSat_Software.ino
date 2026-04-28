#include <Cansat_RFM96.h>
#include <USBHost_t36.h>
#include <Adafruit_GPS.h>

#define PRINT_DEVICE_INFO
#define USBBAUD 1000000  //115200
#define TimeWait 10000
#define GPSSerial Serial1

//=============================================================================
// USB Objects
//=============================================================================
uint32_t baud = USBBAUD;
uint32_t format = USBHOST_SERIAL_8N1;
USBHost myusb;
USBHub hub1(myusb);
USBSerial_BigBuffer userial(myusb, 1);

char buffer[512];

#ifdef USERIAL_IS_SEREMU
USBHIDParser hid1(myusb);
USBHIDParser hid2(myusb);
USBSerialEmu userial(myusb);
#endif

//=============================================================================
// Other Objects
//=============================================================================
Cansat_RFM96 rfm96(433500, false);
//Adafruit_GPS GPS(&GPSSerial);

uint8_t txBuffer[100];
uint8_t GPSArray[100];
int DataCounter = 0;
uint16_t FrameCounter = 0;
uint16_t MuonCount = 0;
uint16_t TotalCount = 0;

#define GPSECHO  false

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

bool startup = true;
uint32_t timer = 0;
uint32_t TimeAtStart = 0;

#ifdef DEBUG_OUTPUT
#define DBGPrintf Serial.printf
#else
// not debug have it do nothing
inline void DBGPrintf(...) {
}
#endif

//=============================================================================
// Setup - only runs once
//=============================================================================
void setup() 
{
  myusb.begin();

  userial.begin(USBBAUD);
  Serial.begin(USBBAUD);

  if (!rfm96.init()) 
  {
    Serial.println("Failed to initialize RFM96!");
    while(1){};
  } 
  else 
  {
    Serial.println("Radio is initialized");
  }

  rfm96.setTxPower(20);

//  GPS.begin(9600);

//  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
//  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);

//  if (GPS.LOCUS_StartLogger())
//  {
//    Serial.println("GPS initialized");
//  }
//  else 
//  {
//    Serial.println("GPS no response");
//  }

  Serial.println("Starting up!");

  startup = true;
  TimeAtStart = millis();
  timer = 0;
  DataCounter = 0;
  FrameCounter = 0;
  MuonCount = 0;
  TotalCount = 0;
}

//=============================================================================
// loop: continuously called.
//=============================================================================
void loop() 
{
  myusb.Task();

  if (startup == true) 
  {
    startup_Timer();
  } 
  else 
  {
//    int gpsBytes = GPSSerial.available();
//    if (gpsBytes > 0)
//    {
//      //char c = GPSSerial.read();
//      GPSSerial.readBytes(GPSArray, gpsBytes);
//      //Serial.write(GPSArray, gpsBytes);
//    }

    handle_CosmicWatchData();
  }

      // check if the USB virtual serial wants a new baud rate
    // ignore if 0 as current Serial monitor of Arduino sets to 0..
    uint32_t cur_usb_baud = Serial.baud();
    if (cur_usb_baud && (cur_usb_baud != baud)) 
    {
      baud = cur_usb_baud;
      DBGPrintf("DEBUG: baud change: %u\n", baud);
      if (baud == 57600) 
      {
        // This ugly hack is necessary for talking
        // to the arduino bootloader, which actually
        // communicates at 58824 baud (+2.1% error).
        // Teensyduino will configure the UART for
        // the closest baud rate, which is 57143
        // baud (-0.8% error).  Serial communication
        // can tolerate about 2.5% error, so the
        // combined error is too large.  Simply
        // setting the baud rate to the same as
        // arduino's actual baud rate works.
        userial.begin(58824);
      } 
      else 
      {
        userial.begin(baud);
      }
    }
}

void handle_CosmicWatchData() 
{
  // check if any data has arrived on the USBHost serial port
  uint16_t n;
  uint16_t rd = userial.available();

  if (rd > 0) 
  {
    // check if the USB virtual serial port is ready to transmit
    if (rd > 80) 
    {
      rd = 80;
    }
    // read data from the USB host serial port
    n = userial.readBytes((char *)buffer, rd);

    //rfm96.printToBuffer((char *)buffer);
    //rfm96.sendAndWriteToFile();
    Serial.write(buffer, n);
    Serial.println("");

    //Convert the received data into a string to make it easier to do string manipulation to split out the different variables.
    String stringBuffer(buffer);
    String receivedData(stringBuffer.substring(0, n));
    receivedData.replace("\t", " ");

    int index = 0;

    while (index < n) 
    {
      int endOfWord = receivedData.indexOf(" ", index);

      if (endOfWord == -1) 
      {
        
        break;
      }

      prepare_TXBuffer(receivedData.substring(index, endOfWord));

      DataCounter++;
      //rfm96.printToBuffer(receivedData.substring(index, endOfWord));

      if (DataCounter >= 10) 
      {
        Serial.println("");
        //Serial.println("Have gotten the whole message!");
        float test = 0;
        uint16_t testing = 0;
        uint8_t crcTest = 5;
        memcpy(&txBuffer[2], &test, sizeof(float));
        memcpy(&txBuffer[6], &test, sizeof(float));
        memcpy(&txBuffer[10], &testing, sizeof(uint16_t));
        memcpy(&txBuffer[40], &crcTest, sizeof(uint8_t));
        memcpy(&txBuffer[41], &crcTest, sizeof(uint8_t));
        DataCounter = 0;
        FrameCounter++;
        memcpy(&txBuffer[0], &FrameCounter, sizeof(uint16_t));
        memcpy(&txBuffer[18], &TotalCount, sizeof(uint16_t));
        memcpy(&txBuffer[20], &MuonCount, sizeof(uint16_t));
        //rfm96.printToBuffer("1");
        rfm96.add((char*)txBuffer, 42);
        rfm96.sendAndWriteToFile();
      }

      index = endOfWord + 1;
    }
  }
}

void prepare_TXBuffer(String data)
{
  //Serial.print(data);
  //Serial.print(", ");
  int index = 0;
  int endOfWord = 0;

  int16_t tempInt;
  float tempFloat;

  switch (DataCounter)
  {
    case 0:
      Serial.print(data);
      Serial.print(" ");
      break;

    case 1:
      Serial.print(data);
      Serial.print(" ");
      break;

    case 2:
      Serial.print(data);
      Serial.print(" ");
      if (data == '1')
      {
        MuonCount++;
      }
      else 
      {
        TotalCount++;
      }
      break;

    case 3:
      Serial.print(data);
      Serial.print(" ");
      break;

    case 4:
      Serial.print(data);
      Serial.print(" ");
      break;

    case 5:
      Serial.print(data);
      Serial.print(" ");
      break;

    case 6:
      Serial.print(data);
      Serial.print(" ");
      tempInt = (int16_t)(data.toFloat() * 10);
      memcpy(&txBuffer[12], &tempInt, sizeof(int16_t));
      break;
    
    case 7:
      Serial.print(data);
      Serial.print(" ");
      tempFloat = data.toFloat();
      memcpy(&txBuffer[14], &tempFloat, sizeof(float));
      break;
    
    case 8:
      Serial.print(data);
      Serial.print(" ");
      endOfWord = data.indexOf(":", index);

      if(endOfWord == -1)
      {
        break;
      }

      tempFloat = data.substring(index, endOfWord).toFloat();
      memcpy(&txBuffer[22], &tempFloat, sizeof(float));

      index = endOfWord + 1;

      endOfWord = data.indexOf(":", index);

      tempFloat = data.substring(index, endOfWord).toFloat();
      memcpy(&txBuffer[26], &tempFloat, sizeof(float));

      index = endOfWord + 1;

      tempFloat = data.substring(index, data.length()).toFloat();
      memcpy(&txBuffer[30], &tempFloat, sizeof(float));
      break;

    case 9:
      Serial.print(data);
      Serial.print(" ");
            endOfWord = data.indexOf(":", index);

      if(endOfWord == -1)
      {
        break;
      }

      tempInt = (int16_t)(data.substring(index, endOfWord).toFloat() * 10.f);
      Serial.println("");
      Serial.print(tempInt);
      memcpy(&txBuffer[34], &tempInt, sizeof(int16_t));

      index = endOfWord + 1;

      endOfWord = data.indexOf(":", index);

      tempInt = (int16_t)(data.substring(index, endOfWord).toFloat() * 10.f);
      memcpy(&txBuffer[36], &tempInt, sizeof(int16_t));
            Serial.println("");
      Serial.print(tempInt);
      index = endOfWord + 1;

      tempInt = (int16_t)(data.substring(index, data.length()).toFloat() * 10.f);
      memcpy(&txBuffer[38], &tempInt, sizeof(int16_t));
            Serial.println("");
      Serial.print(tempInt);
      break;

    default:
      break;
  }
}

void startup_Timer() 
{
  uint16_t rd;
  uint16_t n;
  //Calculate the time passed since the CanSat was powered up
  timer = millis() - TimeAtStart;

  //Check if the enough time has passed to start reading data from the Cosmic Watch.
  //This is because the Cosmic Watch writes a lot of messages when starting up that we don't want
  if (timer >= TimeWait) 
  {
    startup = false;
    Serial.println("Will now read the data!");
  }

  rd = userial.available();

  //Keep reading data arriving from the Cosmic Watch. This is to prevent it from restarting the Cosmic Watch
  if (rd > 0) 
  {
    n = userial.readBytes((char *)buffer, rd);
    Serial.write(buffer, n);
  }
}