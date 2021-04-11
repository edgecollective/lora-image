#include <SerialCommand.h>

SerialCommand sCmd(Serial);         // The demo SerialCommand object, initialize with any Stream object


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




//const int D5 = 13;




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
  int chunk_num;
  int chunks_total;
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

  while (!Serial) {
    ;
  }

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
  
  sCmd.addCommand("TRANSMIT", transmit);
  sCmd.setDefaultHandler(unrecognized);      // Handler for command that isn't matched  (says "What?")

  
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
  camera = new OV7670(OV7670::Mode::QQVGA_RGB565, SIOD, SIOC, VSYNC, HREF, XCLK, PCLK, D0, D1, D2, D3, D4, D5, D6, D7);
  //Serial.println("got here 2");
  BMP::construct16BitHeader(bmpHeader, camera->xres, camera->yres);

  //tft.initR(INITR_BLACKTAB);
  //tft.fillScreen(0);
  //server.begin();
  delay(5000);
}

/*
  void displayY8(unsigned char * frame, int xres, int yres)
  {
  tft.setAddrWindow(0, 0, yres - 1, xres - 1);
  int i = 0;
  for(int x = 0; x < xres; x++)
    for(int y = 0; y < yres; y++)
    {
      i = y * xres + x;
      unsigned char c = frame[i];
      unsigned short r = c >> 3;
      unsigned short g = c >> 2;
      unsigned short b = c >> 3;
      tft.pushColor(r << 11 | g << 5 | b);
    }
  }

  void displayRGB565(unsigned char * frame, int xres, int yres)
  {
  tft.setAddrWindow(0, 0, yres - 1, xres - 1);
  int i = 0;
  for(int x = 0; x < xres; x++)
    for(int y = 0; y < yres; y++)
    {
      i = (y * xres + x) << 1;
      tft.pushColor((frame[i] | (frame[i+1] << 8)));
    }
  }
*/

int16_t packetnum = 0;  // packet counter, we increment per xmission

void loop()
{
  char chunk[numBytes];

  camera->oneFrame();
  //Serial.println("bmp:");
  //Serial.println(BMP::headerSize);

  
  
  //Serial.println(camera->xres * camera->yres * 2);
  //delay(1000);

/*
for (int i = 0; i < BMP::headerSize; i++) {
    Serial.write(bmpHeader[i]);
  }
*/
  
  memcpy(theData.chunk,bmpHeader,sizeof(theData.chunk));

  //send the data
  rf95.send((uint8_t *)&theData, sizeof(theData));

  Serial.println("Waiting for packet to complete..."); 
  delay(10);
  rf95.waitPacketSent();


  int a =0;
  int b = numBytes;

  while (b < camera->xres * camera->yres * 2) {

    int j = 0;
    for (int i = a; i< b; i++) {
      chunk[j]=camera->frame[i];
      j++;
    }

    memcpy(theData.chunk,chunk,sizeof(theData.chunk));

  //send the data
  rf95.send((uint8_t *)&theData, sizeof(theData));

  Serial.println("Waiting for packet to complete..."); 
  delay(10);
  rf95.waitPacketSent();

  /*
    for (int k=0; k< numBytes; k++) {
      Serial.write(chunk[k]);
    }
    */

    a=a+numBytes;
    b=b+numBytes;
    
  }

/*
  for (int i = 0; i < camera->xres * camera->yres * 2; i++) {
    Serial.write(camera->frame[i]);
  }
*/
  
 
  delay(60000);
  //Serial.println();
  //Serial.println();

  //Serial.print(camera->frame);
  //serve();


  //displayRGB565(camera->frame, camera->xres, camera->yres);
}

void transmit(SerialCommand this_sCmd) {

  char chunk[numBytes];
  int i = 0;

  while (i<numBytes){
    //if (Serial.available() > 0) {
      chunk[i]=Serial.read();
      //Serial.print(chunk[i]);
      i++;
   // }
  }
  Serial.print("chunk:");
  Serial.println(chunk);
  theData.chunk_num=0;
  theData.chunks_total=0;
  memcpy(theData.chunk,chunk,sizeof(theData.chunk));

  //send the data
  rf95.send((uint8_t *)&theData, sizeof(theData));

  Serial.println("Waiting for packet to complete..."); 
  //delay(10);
  rf95.waitPacketSent();

  //delay(10);
    
}


// This gets set as the default handler, and gets called when no other command matches.
void unrecognized(SerialCommand this_sCmd) {
  SerialCommand::CommandInfo command = this_sCmd.getCurrentCommand();
  this_sCmd.print("Did not recognize \"");
  this_sCmd.print(command.name);
  this_sCmd.println("\" as a command.");
}
