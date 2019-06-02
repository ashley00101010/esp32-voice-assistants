void AmazonAlexaEvents(){
  
    // By default, fauxmoESP creates it's own webserver on the defined port
    // The TCP port must be 80 for gen3 devices (default is 1901)
    // This has to be done before the call to enable()
    fauxmo.createServer(false); // not needed, this is the default value
    fauxmo.setPort(80); // required for gen3 devices

    // You have to call enable(true) once you have a WiFi connection
    // You can enable or disable the library at any moment
    // Disabling it will prevent the devices from being discovered and switched
    fauxmo.enable(true);

  // Add virtual devices
  fauxmo.addDevice(BLOOM_DEVICE);

  fauxmo.onSetState([](unsigned char device_id, const char * device_name, bool state, unsigned char value) {
    Serial.printf("[MAIN] Device #%d (%s) state: %s\n", device_id, device_name, state ? "ON" : "OFF", value);
    Serial.printf("value: " + value);
    
    if ( (strcmp(device_name, BLOOM_DEVICE) == 0) ) {
        Serial.println("RELAY 1 switched by Alexa");
      //digitalWrite(BLOOM_DEVICE, !digitalRead(BLOOM_COMMAND));
      ledcWrite(ledChannel, value);
      if (state) {
         ledcWrite(ledChannel, 255);
      } else {
         ledcWrite(ledChannel, 0);
      }
    }
   
  });

}

// MQTT Callback
void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Write the message to the Serial for debugging purpose
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
      // If the payload is 1 then turn on the Relay and LED, otherwise turn it off
    String mqttSucces = mqttUrl + "/succes";
    String mqttCheck = mqttUrl + "/check-result";

     if ((char)payload[0] == '2') {
      File checkState = SPIFFS.open("/checkState.txt");
   
      if (!checkState){
          Serial.println("Failed to open file for reading");
          return;
      }
      
      String checkRead = checkState.readString();
      client.publish(mqttCheck.c_str(), checkRead.c_str());
      
      checkState.close();
    }
    
    if ((char)payload[0] == '1') {
     ledcWrite(ledChannel, 255);
     client.publish(mqttSucces.c_str(),"1");
     SPIFFS.remove("/checkState.txt");
     File checkState = SPIFFS.open("/checkState.txt", FILE_WRITE);
      if (!checkState) {
        Serial.println("There was an error opening the file for writing");
        return ;
      }
      if (checkState.print("1")) {
        Serial.println("File was written");
      } else {
        Serial.println("File write failed");
      }
      checkState.close();
    } 
    
    if ((char)payload[0] == '0') {
     ledcWrite(ledChannel, 0);
     client.publish(mqttSucces.c_str(),"0");
     SPIFFS.remove("/checkState.txt");
     File checkState = SPIFFS.open("/checkState.txt", FILE_WRITE);
      if (!checkState) {
        Serial.println("There was an error opening the file for writing");
        return ;
      }
      if (checkState.print("0")) {
        Serial.println("File was written");
      } else {
        Serial.println("File write failed");
      }
      checkState.close();
    }

   
}

// Connect MQTT
void mqttConnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client";
   
    clientId += String(random(0xffff), HEX);
    
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      Serial.print("#MQTT URL2: ");
      Serial.print(mqttUrl.c_str());
      const char* url = mqttUrl.c_str();
      String b = mqttUrl + "/check";
      const char* c = b.c_str();
      Serial.print(c);
      client.subscribe(c);
      client.subscribe(url);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void WifiManager(){
    DNSServer dns;
    AsyncWiFiManager wifiManager(&server,&dns);
    //wifiManager.resetSettings();//If using this. Comment out the (ESP.restart()) in de WiFiManager library.


  if (!wifiManager.autoConnect("Bloom", "Protected")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    //ESP.restart();
    delay(5000);
  }else{
    Serial.println("BLOOM GESTART");
    //ESP.restart();
  }
}


void startWebSocket() { // Start a WebSocket server
  webSocket.begin();                          // start de websocket server
  webSocket.onEvent(webSocketEvent);          // if there's an incomming websocket message, go to function 'webSocketEvent'
  Serial.println("WebSocket server started.");
}


void startFileSystem(){
  if(!SPIFFS.begin(true)){
  Serial.println("An Error has occurred while mounting SPIFFS");
  return;
  }
  
  File addUser = SPIFFS.open("/addUser.txt", FILE_WRITE);
  if (!addUser) {
    Serial.println("There was an error opening the file for writing");
    return ;
  }
  if (addUser.print(mac_address)) {
    Serial.println("File was written");
  } else {
    Serial.println("File write failed");
  }
  addUser.close();

  File file3 = SPIFFS.open("/makeURL.txt");
   
  if (!file3){
      Serial.println("Failed to open file for reading");
      return;
  }
  
  Serial.println("File Content:");
  
  mqttUrl = file3.readString();
  Serial.print("#MQTT URL: ");
  Serial.print(mqttUrl.c_str());
  Serial.print("  ");
  
  file3.close();

  
}

void startMDNS() {
  WiFi.setHostname("bloom");
  if (MDNS.begin("bloom")) {              // Start de mDNS responder voor bloom.local
    MDNS.addService("_osc", "_udp", 4500); 
    MDNS.addServiceTxt("_osc", "_udp", "does", "it"); 
    MDNS.addServiceTxt("_osc", "_udp", "work", "?"); 
    MDNS.addServiceTxt("_osc", "_udp", "yes", "!");
    Serial.println("mDNS responder started");
  } else {
    Serial.println("Error setting up MDNS responder!");
  }
}

void startServer() {

  server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", String(ESP.getFreeHeap()));
  });

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  
   server.on("/update", HTTP_POST, [](AsyncWebServerRequest * request) {
    // the request handler is triggered after the upload has finished... 
    // create the response, add header, and send response
    AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", (Update.hasError())?"FAIL":"OK");
    response->addHeader("Connection", "close");
    response->addHeader("Access-Control-Allow-Origin", "*");
    restartRequired = true;  // Tell the main loop to restart the ESP
    request->send(response);
  }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final){
     if(!index){ // if index == 0 then this is the first frame of data
      Serial.printf("UploadStart: %s\n", filename.c_str());
      Serial.setDebugOutput(true);
      
      // calculate sketch space required for the update
      //uint32_t maxSketchSpace = (ESP.getFreeHeap() - 0x1000) & 0xFFFFF000;
      if(!Update.begin(UPDATE_SIZE_UNKNOWN)){//start with max available size
        Update.printError(Serial);
      }
      Update.begin(true); // tell the updaterClass to run in async mode
    }

    //Write chunked data to the free sketch space
    if(Update.write(data, len) != len){
        Update.printError(Serial);
    }
    
    if(final){ // if the final flag is set then this is the last frame of data
      if(Update.end(true)){ //true to set the size to the current progress
          Serial.printf("Update Success: %u B\nRebooting...\n", index+len);
        } else {
          Update.printError(Serial);
        }
        Serial.setDebugOutput(false);
    }
    
  });

  server.onNotFound([](AsyncWebServerRequest *request){
    String body = (request->hasParam("body", true)) ? request->getParam("body", true)->value() : String();
        if (fauxmo.process(request->client(), request->method() == HTTP_GET, request->url(), body)) return;
    Serial.printf("NOT_FOUND: ");
    if(request->method() == HTTP_GET)
      Serial.printf("GET");
    else if(request->method() == HTTP_POST)
      Serial.printf("POST");
    else if(request->method() == HTTP_DELETE)
      Serial.printf("DELETE");
    else if(request->method() == HTTP_PUT)
      Serial.printf("PUT");
    else if(request->method() == HTTP_PATCH)
      Serial.printf("PATCH");
    else if(request->method() == HTTP_HEAD)
      Serial.printf("HEAD");
    else if(request->method() == HTTP_OPTIONS)
      Serial.printf("OPTIONS");
    else
      Serial.printf("UNKNOWN");
    Serial.printf(" http://%s%s\n", request->host().c_str(), request->url().c_str());

    if(request->contentLength()){
      Serial.printf("_CONTENT_TYPE: %s\n", request->contentType().c_str());
      Serial.printf("_CONTENT_LENGTH: %u\n", request->contentLength());
    }

    int headers = request->headers();
    int i;
    for(i=0;i<headers;i++){
      AsyncWebHeader* h = request->getHeader(i);
      Serial.printf("_HEADER[%s]: %s\n", h->name().c_str(), h->value().c_str());
    }

    int params = request->params();
    for(i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isFile()){
        Serial.printf("_FILE[%s]: %s, size: %u\n", p->name().c_str(), p->value().c_str(), p->size());
      } else if(p->isPost()){
        Serial.printf("_POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
      } else {
        Serial.printf("_GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      }
    }



    request->send(404);
  });

  server.onRequestBody([](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total){
    if (fauxmo.process(request->client(), request->method() == HTTP_GET, request->url(), String((char *)data))) return;
    if(!index)
      Serial.printf("BodyStart: %u\n", total);
    Serial.printf("%s", (const char*)data);
    if(index + len == total)
      Serial.printf("BodyEnd: %u\n", total);
  }); 

  
  server.begin();
}





