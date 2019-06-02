//#include <Arduino.h>
//Libraries needed for WiFiManager (Captive Portal)
#include <ESPAsyncWiFiManager.h>
#include <SPIFFS.h>
#include "FS.h"
#include <ESPmDNS.h>
#include <WebSocketsServer.h>

//Libraries needed for Alexa
#include <fauxmoESP.h>
#include <ESPAsyncWebServer.h>

//Libraries needed for mqtt
#include <WiFi.h>
#include <PubSubClient.h>

//TEST UPDATE SERVER
#include <Update.h>
bool restartRequired = false;  // Set this flag in the callbacks to restart ESP in the main loop

const char* host = "bloom";
String  mqttUrl = "";

// MQTT Server, for this demo purpose I am using Eclipse MQTT Sandbox, you
// can change it to any MQTT server. Also for this demo 
const char* mqtt_server = "";


#define LED_PIN 2
#define BLOOM_DEVICE "Bloom"

int freq = 5000;
int ledChannel = 0;
int resolution = 8;

fauxmoESP fauxmo;

AsyncWebServer server(80); 
WebSocketsServer webSocket = WebSocketsServer(81); 

String mac_address;

WiFiClient espClient;
PubSubClient client(espClient);

/*__________________________________________________________SETUP__________________________________________________________*/
void setup(void) {

  
  Serial.begin(115200);
  Serial.println();

  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(LED_PIN, ledChannel);
  LedsOn();
  WifiManager();

  mac_address = WiFi.macAddress();
  Serial.println(mac_address);

  startFileSystem();
  
  startMDNS(); 

  startServer();

  startWebSocket();

  //Serial.println("SETUP OTA SERVER");
  //setupFunctionsOTA();
  
  AmazonAlexaEvents();

  // Setup PubSubClient
  client.setServer(mqtt_server, 1883);
  client.setCallback(mqttCallback);
  
  Serial.println("SETUP COMPLETE!");
}

/*__________________________________________________________LOOP__________________________________________________________*/

void loop(void) {

  if (restartRequired){  // check the flag here to determine if a restart is required
    Serial.printf("Restarting ESP\n\r");
    restartRequired = false;
    ESP.restart();
  }
 
  // If PubSubClient is not connected then connect it
  if(!Update.isRunning()){
    webSocket.loop();
     if (!client.connected()) {
        mqttConnect();
    }
    
    // fauxmoESP uses an async TCP server but a sync UDP server
    // Therefore, we have to manually poll for UDP packets
    fauxmo.handle();
  }
  client.loop();
  
}
