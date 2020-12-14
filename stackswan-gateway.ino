/*
 =================================================================================
  Stackswan LoRa p2p - (gateway)
 ---------------------------------------------------------------------------------
  1) Endpoint can use any unique 2 byte address otherthan 0xFF.
  2) 0xFF is reserved for all gateway devices in the network
  3) Sends a message every half second, and polls for incoming messages continually
  4) if message is for this device then process message otherwise retransmit.
  5) For easy star networking all non-gateway devices(endpoints) sends message
     to gateway using 0xFF as target.
  ---------------------------------------------------------------------------------   
  Author: Vikas Singh 
  Date: 12/12/20
  ==================================================================================
*/


#include <SPI.h>              
#include <LoRa.h>             
#include <ArduinoJson.h> //ArduinoJson V6

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

std::string deviceName = "Stackswan Gateway";

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
BLECharacteristic* pCharacteristicR = NULL;

bool deviceConnected = false;
bool oldDeviceConnected = false;

#define SERVICE_UUID        "afa2f203-348f-4718-9cf3-f7ff0de38472"
#define CHARACTERISTIC_UUID "450fcc81-dad5-46a6-959f-c5f41e37b669"

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};


#define CHARACTERISTIC_UUID_RX "10fd2313-62c2-4053-b3af-7e6ca407bf21"
class MyCallbacks: public BLECharacteristicCallbacks {

    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      Serial.println(rxValue[0]);

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");

        for (int i = 0; i < rxValue.length(); i++) {
          Serial.print(rxValue[i]);
        }
        Serial.println();
        Serial.println("*********");
      }

    }
};

const int csPin = 5;          // LoRa radio chip select
const int resetPin = 14;      // LoRa radio reset
const int irqPin = 2;         // change for your board; must be a hardware interrupt pin

String outgoing;              // outgoing message

byte msgCount = 0;            // count of outgoing messages
byte localAddress = 0xFF;     // address of this device(0xFF for all gateway devices)
byte destination = 0xAA;      // destination to send to(unique for each devices)
long lastSendTime = 0;        // last send time
int interval = 2000;          // initial interval between sends

void setup() {
  
  // initialize serial
  Serial.begin(115200); 
  setupBlueTooth();                  
  while (!Serial);
  Serial.println("stackswan P2P");
  
  // override the default CS, reset, and IRQ pins (optional)
  // set CS, reset, IRQ pin
  LoRa.setPins(csPin, resetPin, irqPin);

  // initialize radio at 866MHz (eg: 915E6, 868E6 etc)
  if (!LoRa.begin(866E6)) {             
    Serial.println("LoRa initialization failed. Check your connections.");
    while (true);                       // if failed, do nothing
  }

  Serial.println("LoRa initialization succeeded.");
}

void loop() {
  
  // continously parse for a packet, and call onReceive with the result:
  onReceive(LoRa.parsePacket());
}


//method for sending data (not used in gateway for nows)
void sendMessage(String outgoing) {
  LoRa.beginPacket();                   // start packet
  LoRa.write(destination);              // add destination address
  LoRa.write(localAddress);             // add sender address
  LoRa.write(msgCount);                 // add message ID
  LoRa.write(outgoing.length());        // add payload length
  LoRa.print(outgoing);                 // add payload
  LoRa.endPacket();                     // finish packet and send it
  msgCount++;                           // increment message ID
}



//method for processing incoming messages
void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  // read packet header bytes:
  int recipient = LoRa.read();          // recipient address
  byte sender = LoRa.read();            // sender address
  byte incomingMsgId = LoRa.read();     // incoming msg ID
  byte incomingLength = LoRa.read();    // incoming msg length

  String incoming = "";

  while (LoRa.available()) {
    incoming += (char)LoRa.read();
  }

  if (incomingLength != incoming.length()) {                        // check length for error
    Serial.println("error: data in the received payload is corrupted");
    return;                                                         // skip rest of function
  }

  // if the recipient isn't this device or broadcast,
  if (recipient != localAddress && recipient != 0xFF) {
    Serial.println("This message is not for me.");
    return;                                                   // skip rest of function
  }

  // if message is for this device, or broadcast, print details:
  Serial.println("Received from: 0x" + String(sender, HEX));
  Serial.println("Sent to: 0x" + String(recipient, HEX));
  Serial.println("Message ID: " + String(incomingMsgId));
  Serial.println("Message length: " + String(incomingLength));
  Serial.println("Message: " + incoming);
  Serial.println("RSSI: " + String(LoRa.packetRssi()));
  Serial.println("Snr: " + String(LoRa.packetSnr()));
  Serial.println();

  // JSON modeling the raw data 
  const int bufferSize = 4 *  JSON_OBJECT_SIZE(4);
  StaticJsonDocument<bufferSize> doc;

  doc["SenderID"]  = sender;
  doc["ReceiverID"] = recipient;
  doc["Payload"] = incoming;

  String jsonstat;
  Serial.println(jsonstat);
  serializeJson(doc, jsonstat);

  // Pretty print on Serial
  serializeJsonPretty(doc, Serial);


  size_t dataLen = jsonstat.length();
  pCharacteristic->setValue((uint8_t*)&jsonstat[0], dataLen);
  pCharacteristic->notify();
  
}

/**
   setupBlueTooth
*/
void setupBlueTooth()
{
  // Create the BLE Device
  BLEDevice::init(deviceName);

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  /*
    pCharacteristicR = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID_RX,
                                         BLECharacteristic::PROPERTY_WRITE
                                       );
    pCharacteristicR->setCallbacks(new MyCallbacks());
  */
 
  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  Serial.println("Waiting a client connection to notify...");
}

