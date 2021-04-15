//#include <SerialCommand.h>

//SerialCommand sCmd(Serial);         // The demo SerialCommand object, initialize with any Stream object


#include "OV7670.h"

#include <RHSoftwareSPI.h> // http://www.airspayce.com/mikem/arduino/RadioHead/RadioHead-1.113.zip
#include <RHRouter.h>
#include <RHMesh.h>
#include <SPI.h>
#include <RH_RF95.h>

#define this_node_id 4
#define gatewayNode 1

RHMesh *manager;

// Radio pins for feather M0
#define RFM95_CS 21
#define RFM95_RST 13
#define RFM95_INT 25

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);


//#include <Adafruit_GFX.h>    // Core graphics library
//#include <Adafruit_ST7735.h> // Hardware-specific library


#include "BMP.h"

#define waitTime 1000

// feather 32 huzzah
// 34,35,36,39 read only

const int SIOC = 22; //SCL
const int SIOD = 23; //SDA

//read only
const int VSYNC = 34; //A2
const int HREF = 39; //

const int PCLK = 33; //
const int XCLK = 32; //

const int D7 = 4; //
const int D6 = 26; //

//const int D5 = 21;
const int D5 = 36;


const int D4 = 14; //
const int D3 = 15; //
const int D2 = 16; //
const int D1 = 17; //
const int D0 = 27;//


#define interval_sec 300


//const int TFT_DC = 2;
//const int TFT_CS = 5;
//DIN <- MOSI 23
//CLK <- SCK 18


//Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, 0/*no reset*/);
OV7670 *camera;



unsigned char bmpHeader[BMP::headerSize];

const int numBytes=200;

typedef struct {
  int chunks_total;
  char header[BMP::headerSize];
} Metadata;

Metadata myMeta;

typedef struct {
  int chunk_num;
  char chunk[numBytes];
} Payload;

Payload theData;



void setup()
{
  Serial.begin(115200);


  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  manager = new RHMesh(rf95, this_node_id);

  
   if (!manager->init()) {
    Serial.println(F("mesh init failed"));
    //u8x8.setCursor(0,4); 
    //u8x8.print("LoRa fail!");
  } else {
    //u8x8.setCursor(0,4); 
    //u8x8.print("LoRa working!");
    delay(1000);
  }
  rf95.setTxPower(23, false);
  rf95.setFrequency(915.0);
  rf95.setCADTimeout(500);

  // Possible configurations:
  // Bw125Cr45Sf128 (the chip default)
  // Bw500Cr45Sf128
  // Bw31_25Cr48Sf512
  // Bw125Cr48Sf4096

  // long range configuration requires for on-air time
  boolean longRange = false;
  if (longRange) {
    RH_RF95::ModemConfig modem_config = {
      0x78, // Reg 0x1D: BW=125kHz, Coding=4/8, Header=explicit
      0xC4, // Reg 0x1E: Spread=4096chips/symbol, CRC=enable
      0x08  // Reg 0x26: LowDataRate=On, Agc=Off.  0x0C is LowDataRate=ON, ACG=ON
    };
    rf95.setModemRegisters(&modem_config);
    if (!rf95.setModemConfig(RH_RF95::Bw125Cr48Sf4096)) {
      Serial.println(F("set config failed"));
    }
  }

  Serial.println("RF95 ready");




  
  
  //Serial.println("got here 1");
  camera = new OV7670(OV7670::Mode::QQQVGA_RGB565, SIOD, SIOC, VSYNC, HREF, XCLK, PCLK, D0, D1, D2, D3, D4, D5, D6, D7);
  //Serial.println("got here 2");
  BMP::construct16BitHeader(bmpHeader, camera->xres, camera->yres);

  //tft.initR(INITR_BLACKTAB);
  //tft.fillScreen(0);
  //server.begin();

  delay(1000);
  
  
    
  /*Serial.print("totalBytes:");
  Serial.println(totalBytes);
  Serial.print("numChunks:");
  Serial.println(numChunks);
  */
  //delay(5000);
}


int16_t packetnum = 0;  // packet counter, we increment per xmission

int firstLoop = 1;

long lastMeasureTime = 0;  // the last time the output pin was toggled

long measureDelay = interval_sec*1000;

void loop()
{

if (  ( (millis() - lastMeasureTime) > measureDelay) || firstLoop) {

  firstLoop = 0;
  
  camera->oneFrame();

  int totalBytes = camera->xres * camera->yres * 2;
  int numChunks = totalBytes/numBytes;

  myMeta.chunks_total = numChunks;
  memcpy(myMeta.header,bmpHeader,sizeof(bmpHeader));
  
  //Serial.write((uint8_t *) myMeta.header,sizeof(myMeta.header));
  
  //send the data
  //RHRouter::RoutingTableEntry *route = manager->getRouteTo(gatewayNode);

  int success = 0;
  while (success == 0) {
  uint8_t error = manager->sendtoWait((uint8_t *)&myMeta,sizeof(myMeta), gatewayNode);

  if (error != RH_ROUTER_ERROR_NONE) {
    Serial.println();
    Serial.print(F(" ! "));
    //this_sCmd.println(getErrorString(error));
  }
  else{
    Serial.println("Sent header!");
    success=1;
  }
  }
  
  //rf95.send((uint8_t *)&myMeta, sizeof(myMeta));
  //rf95.waitPacketSent();

  lastMeasureTime = millis();

}

uint8_t buf[sizeof(Payload)];
  uint8_t len = sizeof(buf);
  uint8_t from;


  //if (rf95.waitAvailableTimeout(1000)) {

  if (manager->recvfromAckTimeout((uint8_t *)buf, &len, waitTime, &from)) { 
// Should be a message for us now


    theData = *(Payload*)buf;

    Serial.print("requested: ");
    Serial.println(theData.chunk_num);

    int requested_chunk = theData.chunk_num;


    theData.chunk_num=theData.chunk_num;

      char chunk[numBytes];
      
      int a = requested_chunk*numBytes;
      int b = requested_chunk*numBytes+numBytes;
      
      int j = 0;
        for (int i = a; i< b; i++) {
          chunk[j]=camera->frame[i];
          j++;
        }

      memcpy(theData.chunk,chunk,sizeof(theData.chunk));      
      //Serial.write((uint8_t *) chunk,sizeof(chunk));
      
     
    
    
    //rf95.send((uint8_t *)&theData, sizeof(theData));
    //rf95.waitPacketSent();
    uint8_t error = manager->sendtoWait((uint8_t *)&theData, sizeof(theData), gatewayNode);
    if (error != RH_ROUTER_ERROR_NONE) {
    Serial.println();
    Serial.print(F(" ! "));
    //this_sCmd.println(getErrorString(error));
    }
    else{
      //Serial.println("Sent header!");
       Serial.print("sent chunk: ");
      Serial.println(requested_chunk);
    }
    //Serial.println("replied!");

    
    
  }
}

const __FlashStringHelper* getErrorString(uint8_t error) {
  switch(error) {
    case 1: return F("invalid length");
    break;
    case 2: return F("no route");
    break;
    case 3: return F("timeout");
    break;
    case 4: return F("no reply");
    break;
    case 5: return F("unable to deliver");
    break;
  }
  return F("unknown");
}
