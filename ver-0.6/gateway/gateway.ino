#include <SerialCommand.h>

SerialCommand sCmd(Serial);         // The demo SerialCommand object, initialize with any Stream object

#include "BMP.h"

#include <SPI.h>
#include <RH_RF95.h>

// Radio pins for mothbot
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

// Blinky on receipt
#define LED 13


const int numBytes=200;

typedef struct {
  int chunk_num;
  int chunks_total;
  char chunk[numBytes];
} Payload;

Payload theData;

void setup()
{
  pinMode(LED, OUTPUT);
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  Serial.begin(9600);

  /*
  while (!Serial) {
    delay(1);
  }
  */
  
  delay(100);

  //Serial.println("Feather LoRa RX Test!");

  // manual reset
  digitalWrite(RFM95_RST, LOW);
  delay(10);
  digitalWrite(RFM95_RST, HIGH);
  delay(10);

  while (!rf95.init()) {
    //Serial.println("LoRa radio init failed");
    //Serial.println("Uncomment '#define SERIAL_DEBUG' in RH_RF95.cpp for detailed debug info");
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

  delay(5000);
}

int i =-1;

void loop()
{

  
  if (i<48) {
    theData.chunk_num=i;
    rf95.send((uint8_t *)&theData, sizeof(theData));
    rf95.waitPacketSent();
    //Serial.print("requesting chunk:");
    //Serial.println(i);
    
    if (rf95.waitAvailableTimeout(1000))
  { 
    uint8_t buf[sizeof(Payload)];
    uint8_t len = sizeof(buf);
    
    // Should be a reply message for us now   
    if (rf95.recv(buf, &len))
   {
      theData = *(Payload*)buf;

      //Serial.print("got chunk:");
     // Serial.println(theData.chunk_num);
      if(theData.chunk_num==i) {

        if (i==-1) {
          //Serial.write((uint8_t *)theData.chunk,BMP::headerSize);
          Serial.write((uint8_t *) theData.chunk,sizeof(theData.chunk));
        }
        else {
          Serial.write((uint8_t *) theData.chunk,sizeof(theData.chunk));
        }
        i++;
      }
    }
    else
    {
      //Serial.println("Receive failed");
    }
  }
  else
  {
    //Serial.println("No reply, is there a listener around?");
  }
       
    delay(10);
  }
  
  
}
