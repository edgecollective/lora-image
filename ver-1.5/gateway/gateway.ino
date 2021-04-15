#include <SerialCommand.h>

SerialCommand sCmd(Serial);         // The demo SerialCommand object, initialize with any Stream object

#include "BMP.h"

#include <RHSoftwareSPI.h>  // http://www.airspayce.com/mikem/arduino/RadioHead/RadioHead-1.113.zip
#include <RHRouter.h>
#include <RHMesh.h>
#include <RH_RF95.h>
#include <SPI.h>

#define RF95_FREQ 915.0

#define gatewayNode 1

int remoteNode = 2;

RHMesh *manager;

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

#define numBytes 200

int newImage = 0;

//const int numBytes=200;

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
  pinMode(LED, OUTPUT);
  
  Serial.begin(9600);

  sCmd.addCommand("LATEST",    getLatestImage);          // Turns LED on
  sCmd.addCommand("MARK_READ", markImagesRead);
  sCmd.addCommand("NUM_UNREAD",getNumUnread);
  sCmd.setDefaultHandler(unrecognized);      // Handler for command that isn't matched  (says "What?")

  /*
  while (!Serial) {
    delay(1);
  }
  */
  
  delay(100);

  //Serial.println("Feather LoRa RX Test!");

  // manual reset
  pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

   pinMode(RFM95_RST, OUTPUT);
  digitalWrite(RFM95_RST, HIGH);

  manager = new RHMesh(rf95, gatewayNode);

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
  

  //delay(5000);
}

int i =0;

int sending=0;
int totalChunks = 0;

char matrix[48][numBytes];

void loop()
{

  int num_bytes = sCmd.readSerial();      // fill the buffer
    if (num_bytes > 0){
      sCmd.processCommand();  // process the command
    }
    
  if (sending==0) {

    //Serial.println("listening ...");
    
    //if (rf95.available())

    
    //if (rf95.waitAvailableTimeout(1000))
    //{

    uint8_t buf[sizeof(Metadata)];
  uint8_t len = sizeof(buf);
  uint8_t from;

  //if (rf95.waitAvailableTimeout(1000)) {
  int waitTime = 1000;
  
  if (manager->recvfromAckTimeout((uint8_t *)buf, &len, waitTime, &from)) { 
  

      //uint8_t buf[sizeof(Metadata)];
      //uint8_t len = sizeof(buf);
      remoteNode=from;
      //Serial.print("got header from ");
      //Serial.println(remoteNode);
      
        myMeta = *(Metadata*)buf;

        //Serial.print("totalChunks=");
        //Serial.println(theData.chunks_total);
        totalChunks=myMeta.chunks_total;

        //Serial.write((uint8_t *)myMeta.header,sizeof(myMeta.header));
        
        sending=1;
        i=0;
      
    }
  }
  
  else { // we're receiving an image

    if (i<totalChunks) {
      theData.chunk_num=i;
      //Serial.print("remoteNode:");
      //Serial.println(remoteNode);
      
      uint8_t error = manager->sendtoWait((uint8_t *)&theData, sizeof(theData), remoteNode);
      if (error != RH_ROUTER_ERROR_NONE) {
      //Serial.println();
      //Serial.print(F(" ! "));
      //this_sCmd.println(getErrorString(error));
      }
      else{
        //Serial.println("Sent header!");
         //Serial.print("sent request ");
         //Serial.print (i);
         //Serial.print(" to remote node ");
         //Serial.println(remoteNode);
        //Serial.println(requested_chunk);

         uint8_t buf[sizeof(Payload)];
          uint8_t len = sizeof(buf);
          uint8_t from;
        
          //if (rf95.waitAvailableTimeout(1000)) {
          int waitTime = 4000;
          
          if (manager->recvfromAckTimeout((uint8_t *)buf, &len, waitTime, &from)) 
             {
                theData = *(Payload*)buf;
          
                //Serial.print("got chunk:");
               // Serial.println(theData.chunk_num);
                if(theData.chunk_num==i) {
        
                    memcpy(matrix[i],theData.chunk,sizeof(matrix[i]));      
                    digitalWrite(LED,HIGH);
                    delay(5);
                    digitalWrite(LED,LOW);
                    //Serial.write((uint8_t *) theData.chunk,sizeof(theData.chunk));
                    //delay(300); // let serial port catch up
                  i++;
                }
              
            }
    
      }
      
      //rf95.send((uint8_t *)&theData, sizeof(theData));
      //rf95.waitPacketSent();
      //Serial.print("requesting chunk:");
      //Serial.println(i);
      
 
         
      //delay(10);
    }
    else{ // we're done receiving an image
      sending=0;
      newImage = 1;
      /*
      Serial.write((uint8_t *)myMeta.header,sizeof(myMeta.header));
      for (int j=0;j<48;j++) {
        Serial.write((uint8_t *) matrix[j],sizeof(matrix[j]));
        delay(300);
      }
      */
    }
  
  }
  
}

void getLatestImage(SerialCommand this_sCmd) {
  
  //this_sCmd.println("LED on");
  
  if(sending==0 && newImage > 0) {
    
    Serial.write((uint8_t *)myMeta.header,sizeof(myMeta.header));
    delay(500);
    
        for (int j=0;j<48;j++) {
          Serial.write((uint8_t *) matrix[j],sizeof(matrix[j]));
          delay(500);
    }
  }
}

void markImagesRead(SerialCommand this_sCmd) {
  newImage = 0;
}

void getNumUnread(SerialCommand this_sCmd) {
  if (sending==0) {
    this_sCmd.println(newImage);
  }
  else {
    this_sCmd.println(0);
  }
}

// This gets set as the default handler, and gets called when no other command matches.
void unrecognized(SerialCommand this_sCmd) {
  SerialCommand::CommandInfo command = this_sCmd.getCurrentCommand();
  this_sCmd.print("Did not recognize \"");
  this_sCmd.print(command.name);
  this_sCmd.println("\" as a command.");
}
