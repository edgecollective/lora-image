#include <SerialCommand.h>

SerialCommand sCmd(Serial);         // The demo SerialCommand object, initialize with any Stream object

#include <SPI.h>
#include <RH_RF95.h>

// Radio pins for feather M0
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

// Change to 434.0 or other frequency, must match RX's freq!
#define RF95_FREQ 915.0

// Singleton instance of the radio driver
RH_RF95 rf95(RFM95_CS, RFM95_INT);

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
  /*while (!Serial) {
    delay(1);
  }
  */

  delay(100);

  Serial.println("Feather LoRa TX Test!");

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
  Serial.println("LoRa radio init OK!");

  // Defaults after init are 434.0MHz, modulation GFSK_Rb250Fd250, +13dbM
  if (!rf95.setFrequency(RF95_FREQ)) {
    Serial.println("setFrequency failed");
    while (1);
  }
  Serial.print("Set Freq to: "); Serial.println(RF95_FREQ);
  
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then 
  // you can set transmitter powers from 5 to 23 dBm:
  rf95.setTxPower(23, false);


   // serialCommand
   
  // Setup callbacks for SerialCommand commands
  sCmd.addCommand("ON",    LED_on);          // Turns LED on
  sCmd.addCommand("OFF",   LED_off);         // Turns LED off
  sCmd.addCommand("TRANSMIT", transmit);
  sCmd.setDefaultHandler(unrecognized);      // Handler for command that isn't matched  (says "What?")

  
}

int16_t packetnum = 0;  // packet counter, we increment per xmission

void loop()
{

  int num_bytes = sCmd.readSerial();      // fill the buffer
  if (num_bytes > 0){
    sCmd.processCommand();  // process the command
  }
  delay(10);
  
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
  delay(10);
  digitalWrite(LED, HIGH);
  rf95.waitPacketSent();
  digitalWrite(LED,LOW);

  //delay(10);
    
}

void LED_on(SerialCommand this_sCmd) {
  this_sCmd.println("LED on");
  digitalWrite(LED, HIGH);
}

void LED_off(SerialCommand this_sCmd) {
  this_sCmd.println("LED off");
  digitalWrite(LED, LOW);
}

// This gets set as the default handler, and gets called when no other command matches.
void unrecognized(SerialCommand this_sCmd) {
  SerialCommand::CommandInfo command = this_sCmd.getCurrentCommand();
  this_sCmd.print("Did not recognize \"");
  this_sCmd.print(command.name);
  this_sCmd.println("\" as a command.");
}
