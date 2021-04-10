// Demo Code for SerialCommand Library
// Craig Versek, Jan 2014
// based on code from Steven Cogswell, May 2011

#include <SerialCommand.h>

#define arduinoLED 13   // Arduino LED on board

SerialCommand sCmd(Serial);         // The demo SerialCommand object, initialize with any Stream object


#include <RHSoftwareSPI.h>  // http://www.airspayce.com/mikem/arduino/RadioHead/RadioHead-1.113.zip
#include <RHRouter.h>
#include <RHMesh.h>
#include <RH_RF95.h>
#include <SPI.h>

#define RF95_FREQ 915.0
#define gatewayNode 1

const int numBytes=50;

RHMesh *manager;

typedef struct {
  int chunk_num;
  int chunks_total;
  char chunk[numBytes];
} Payload;

Payload theData;

// Radio pins for feather M0
#define RFM95_CS 8
#define RFM95_RST 4
#define RFM95_INT 3

#define this_node_id 2

#define LED 13

RH_RF95 rf95(RFM95_CS, RFM95_INT);


void setup() {
  pinMode(arduinoLED, OUTPUT);      // Configure the onboard LED for output
  digitalWrite(arduinoLED, LOW);    // default to LED off

  Serial.begin(9600);

  // Setup callbacks for SerialCommand commands
  sCmd.addCommand("ON",    LED_on);          // Turns LED on
  sCmd.addCommand("OFF",   LED_off);         // Turns LED off
  sCmd.addCommand("HELLO", sayHello);        // Echos the string argument back
  sCmd.addCommand("P",     processCommand);  // Converts two arguments to integers and echos them back
  sCmd.addCommand("SEND", sendChunk);
  sCmd.addCommand("TRANSMIT", transmit);
  
  sCmd.setDefaultHandler(unrecognized);      // Handler for command that isn't matched  (says "What?")
  Serial.println("Ready");

  manager = new RHMesh(rf95, this_node_id);

   if (!manager->init()) {
    Serial.println(F("mesh init failed"));
    
  }
  rf95.setTxPower(23, false);
  rf95.setFrequency(915.0);
  rf95.setCADTimeout(500);

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

  
}

void loop() {
  int num_bytes = sCmd.readSerial();      // fill the buffer
  if (num_bytes > 0){
    sCmd.processCommand();  // process the command
  }
  delay(10);
}


void LED_on(SerialCommand this_sCmd) {
  this_sCmd.println("LED on");
  digitalWrite(arduinoLED, HIGH);
}

void LED_off(SerialCommand this_sCmd) {
  this_sCmd.println("LED off");
  digitalWrite(arduinoLED, LOW);
}


void sayHello(SerialCommand this_sCmd) {
  char *arg;
  arg = this_sCmd.next();    // Get the next argument from the SerialCommand object buffer
  if (arg != NULL) {    // As long as it existed, take it
    this_sCmd.print("Hello ");
    this_sCmd.println(arg);
  }
  else {
    this_sCmd.println("Hello, whoever you are");
  }
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
    //RHRouter::RoutingTableEntry *route = manager->getRouteTo(gatewayNode);
    uint8_t error = manager->sendtoWait((uint8_t *)&theData, sizeof(theData), gatewayNode);

    if (error != RH_ROUTER_ERROR_NONE) {
      //Serial.println();
      //Serial.print(F(" ! "));
      this_sCmd.println(getErrorString(error));
    }
    else{
      this_sCmd.println("Sent!");
    }
    
}


void sendChunk(SerialCommand this_sCmd) {
  int chunk_num;
  int chunks_total;
  
  char *arg;
  
  arg = this_sCmd.next();    // Get the next argument from the SerialCommand object buffer
  if (arg != NULL) {    
    this_sCmd.println(arg);
    memcpy(theData.chunk,arg,sizeof(theData.chunk));
  }
  else {
    this_sCmd.println("No arguments");
  }

  arg = this_sCmd.next();
  if (arg != NULL) {
    chunk_num = atol(arg);
    this_sCmd.print("Second argument was: ");
    this_sCmd.println(chunk_num);

    theData.chunk_num=chunk_num;
  }
  else {
    this_sCmd.println("No second argument");
  }

  arg = this_sCmd.next();
  if (arg != NULL) {
    chunks_total = atol(arg);
    this_sCmd.print("Third argument was: ");
    this_sCmd.println(chunks_total);

    theData.chunks_total=chunks_total;
    
    //send the data
    //RHRouter::RoutingTableEntry *route = manager->getRouteTo(gatewayNode);
    uint8_t error = manager->sendtoWait((uint8_t *)&theData, sizeof(theData), gatewayNode);

    if (error != RH_ROUTER_ERROR_NONE) {
      //Serial.println();
      //Serial.print(F(" ! "));
      this_sCmd.println(getErrorString(error));
    }
    else{
      this_sCmd.println("Sent!");
    }
    
  }
  else {
    this_sCmd.println("No third argument");
  }

  
  
}


void processCommand(SerialCommand this_sCmd) {
  int aNumber;
  char *arg;

  this_sCmd.println("We're in processCommand");
  arg = this_sCmd.next();
  if (arg != NULL) {
    aNumber = atoi(arg);    // Converts a char string to an integer
    this_sCmd.print("First argument was: ");
    this_sCmd.println(aNumber);
  }
  else {
    this_sCmd.println("No arguments");
  }

  arg = this_sCmd.next();
  if (arg != NULL) {
    aNumber = atol(arg);
    this_sCmd.print("Second argument was: ");
    this_sCmd.println(aNumber);
  }
  else {
    this_sCmd.println("No second argument");
  }
}

// This gets set as the default handler, and gets called when no other command matches.
void unrecognized(SerialCommand this_sCmd) {
  SerialCommand::CommandInfo command = this_sCmd.getCurrentCommand();
  this_sCmd.print("Did not recognize \"");
  this_sCmd.print(command.name);
  this_sCmd.println("\" as a command.");
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
