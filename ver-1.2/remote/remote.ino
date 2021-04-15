//#include <SerialCommand.h>

//SerialCommand sCmd(Serial);         // The demo SerialCommand object, initialize with any Stream object


#include "OV7670.h"

#include <SPI.h>
#include <RH_RF95.h>

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

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiClient.h>
#include "BMP.h"

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


#define interval_sec 60


//const int TFT_DC = 2;
//const int TFT_CS = 5;
//DIN <- MOSI 23
//CLK <- SCK 18

#define ssid1        "Fios-7S5yS"
#define password1    "whose44gel62has"
//#define ssid2        ""
//#define password2    ""

//Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, 0/*no reset*/);
OV7670 *camera;

WiFiMulti wifiMulti;
WiFiServer server(80);

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


void serve()
{
  WiFiClient client = server.available();
  if (client)
  {
    //Serial.println("New Client.");
    String currentLine = "";
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        //Serial.write(c);
        if (c == '\n')
        {
          if (currentLine.length() == 0)
          {
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();
            client.print(
              "<style>body{margin: 0}\nimg{height: 100%; width: auto}</style>"
              "<img id='a' src='/camera' onload='this.style.display=\"initial\"; var b = document.getElementById(\"b\"); b.style.display=\"none\"; b.src=\"camera?\"+Date.now(); '>"
              "<img id='b' style='display: none' src='/camera' onload='this.style.display=\"initial\"; var a = document.getElementById(\"a\"); a.style.display=\"none\"; a.src=\"camera?\"+Date.now(); '>");
            client.println();
            break;
          }
          else
          {
            currentLine = "";
          }
        }
        else if (c != '\r')
        {
          currentLine += c;
        }

        if (currentLine.endsWith("GET /camera"))
        {
          client.println("HTTP/1.1 200 OK");
          client.println("Content-type:image/bmp");
          client.println();

          client.write(bmpHeader, BMP::headerSize);
          client.write(camera->frame, camera->xres * camera->yres * 2);
        }
      }
    }
    // close the connection:
    client.stop();
    //Serial.println("Client Disconnected.");
  }
}

void setup()
{
  Serial.begin(115200);


  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);


  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

    while (!rf95.init()) {
    Serial.println("LoRa radio init failed");
    Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
    while (1);
  }
  //Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    //Serial.println("setFrequency failed");
    while (1);
  }
  //Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);
  
   // Setup callbacks for SerialCommand commands
  
//  sCmd.addCommand("TRANSMIT", transmit);
  //sCmd.setDefaultHandler(unrecognized);      // Handler for command that isn't matched  (says "What?")

  
  wifiMulti.addAP(ssid1, password1);
  //wifiMulti.addAP(ssid2, password2);
  //Serial.println("Connecting Wifi...");
  if (wifiMulti.run() == WL_CONNECTED) {
    //Serial.println("");
    //Serial.println("WiFi connected");
    //Serial.println("IP address: ");
    //Serial.println(WiFi.localIP());
  }
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
  
  Serial.write((uint8_t *) myMeta.header,sizeof(myMeta.header));
  
  //Serial.println("sent header");
  
  rf95.send((uint8_t *)&myMeta, sizeof(myMeta));
  rf95.waitPacketSent();

  lastMeasureTime = millis();

}

  if (rf95.waitAvailableTimeout(1000))
    {
// Should be a message for us now
    uint8_t buf[sizeof(Payload)];
    uint8_t len = sizeof(buf);

    if (rf95.recv(buf, &len))
    {

    theData = *(Payload*)buf;

    //Serial.print("requested: ");
    //Serial.println(theData.chunk_num);

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
      Serial.write((uint8_t *) chunk,sizeof(chunk));
      
      //Serial.print("sent ");
      //Serial.println(requested_chunk);
    
    
    rf95.send((uint8_t *)&theData, sizeof(theData));
    rf95.waitPacketSent();
    //Serial.println("replied!");

    
    }
  }
}
