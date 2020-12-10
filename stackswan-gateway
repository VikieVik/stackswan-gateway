/**
 * @file PapaDuck.ino
 * @brief Uses built-in PapaDuck from the SDK to create a WiFi enabled Papa Duck
 * 
 * This example will configure and run a Papa Duck that connects to the cloud
 * and forwards all messages (except  pings) to the cloud.
 * 
 * @date 2020-11-10
 * 
 * @copyright Copyright (c) 2020
 * ClusterDuck Protocol 
 * 
 * ==================================================================
 * Stackswan :
 * Uses CDP to communicate over LoRa ad-hoc
 * PAPA is upate to work offline
 * TODO: Add stackswan(bluebird) BLE code into this to 
 * log all data on stackswan over BLE
 * ==================================================================
 * 
 */

#include <PapaDuck.h>

#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include "timer.h"

#define SSID ""
#define PASSWORD ""

// Used for Mqtt client connection
// Provided when a Papa Duck device is created in DMS
#define ORG         ""
#define DEVICE_ID   ""
#define DEVICE_TYPE ""
#define TOKEN       ""

// Use pre-built papa duck: the duck ID is provided by DMS
PapaDuck duck = PapaDuck(DEVICE_ID);

char server[]           = ORG ".messaging.internetofthings.ibmcloud.com";
char authMethod[]       = "use-token-auth";
char token[]            = TOKEN;
char clientId[]         = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;

// Set this to false if testing quickstart on IBM IoT Platform
bool use_auth_method = true;

auto timer = timer_create_default(); // create a timer with default settings

WiFiClientSecure wifiClient;
PubSubClient client(server, 8883, wifiClient);

bool retry = true;

// LORA RF CONFIG
#define LORA_FREQ 868.0 // Frequency Range. Set for US Region 915.0Mhz
#define LORA_TXPOWER 20 // Transmit Power
// LORA HELTEC PIN CONFIG
#define LORA_CS_PIN 5
#define LORA_DIO0_PIN 2
#define LORA_DIO1_PIN -1 // unused
#define LORA_RST_PIN 14

void setup() {
  

  /**
   * duck.setupSerial()
   * duck.setupRadio();
   * duck.setupWifi("PapaDuck Setup"); 
   * duck.setupDns();
   * duck.setupInternet(SSID, PASSWORD);
   * duck.setupWebServer(false);
   * duck.setupOTA();
   */ 

    // initialize the serial component with the hardware supported baudrate
  duck.setupSerial(115200);
  // initialize the LoRa radio with specific settings. This will overwrites settings defined in the CDP config file cdpcfg.h
  duck.setupRadio(LORA_FREQ, LORA_CS_PIN, LORA_RST_PIN, LORA_DIO0_PIN, LORA_DIO1_PIN, LORA_TXPOWER);
  // initialize the wifi access point with a custom AP name
   duck.setupWifi("PapaDuck Setup"); 
  // initialize DNS
  duck.setupDns();
  // initialize web server, enabling the captive portal with a custom HTML page
//  duck.setupWebServer(true, HTML);
  duck.setupWebServer(true);
  // initialize Over The Air firmware upgrade
  duck.setupOTA();

  // the default setup is equivalent to the above setup sequence
  //duck.setupWithDefaults(SSID, PASSWORD);

  
  // register a callback to handle incoming data from duck in the network
  duck.onReceiveDuckData(handleDuckData);
  
  Serial.println("[PAPA] Setup OK!");
}

// The callback method simply takes the incoming packet and
// converts it to a JSON string, before sending it out over WiFi
void handleDuckData(Packet packet) {
  quackJson(packet);
}

void loop() {

/**
  if (!duck.isWifiConnected() && retry) {
    String ssid = duck.getSsid();
    String password = duck.getPassword();
    
    Serial.print("[PAPA] WiFi disconnected, reconnecting to local network: ");
    Serial.print(ssid);

    int err = duck.reconnectWifi(ssid, password);

    if (err != DUCK_ERR_NONE) {
      retry = false;
      timer.in(5000, enableRetry);
    }
  }
  if (duck.isWifiConnected()) {
    setup_mqtt(use_auth_method);
  }
*/
  duck.run();
  timer.tick();
}

/**
void setup_mqtt(bool use_auth)
{
  bool connected = client.connected();
  if (connected) {
    return;
  }
  
  for (;;) {
    if (use_auth) {
      connected = client.connect(clientId, authMethod, token);
    } else {
      connected = client.connect(clientId);
    }
    if (connected) {
      Serial.println("[PAPA] mqtt client is connected!");
      break;
    }
    retry_mqtt_connection(500);
  }
}

*/
void quackJson(Packet packet) {

/*
  if (packet.topic == "ping") {
    return;
  }
*/
 // Log RAW payload on serial monitor
  Serial.println(packet.payload);
  /*
  const int bufferSize = 4 *  JSON_OBJECT_SIZE(4);
  StaticJsonDocument<bufferSize> doc;

  doc["DeviceID"]  = packet.senderId;
  doc["MessageID"] = packet.messageId;
  doc["Payload"].set(packet.payload);

  // FIXME: Path shouldn't be altered by the application
  // It should done in the library PapaDuck component
  doc["path"].set(packet.path + "," + DEVICE_ID);

  String loc = "iot-2/evt/" + packet.topic + "/fmt/json";
  Serial.print(loc);
  // add space for null char
  int len = loc.length() + 1;

  char topic[len];
  loc.toCharArray(topic, len);

  String jsonstat;
  serializeJson(doc, jsonstat);

  if (client.publish(topic, jsonstat.c_str())) {
    serializeJsonPretty(doc, Serial);
    Serial.println("");
    Serial.println("[PAPA] Publish ok");
  }
  else {
    Serial.println("[PAPA] Publish failed");
  }
} */

}
