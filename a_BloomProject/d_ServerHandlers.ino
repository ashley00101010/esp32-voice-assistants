

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t lenght) { // When a WebSocket message is received
  switch (type) {
    case WStype_DISCONNECTED:             // if the websocket is disconnected
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED: {              // if a new websocket connection is established
        IPAddress ip = webSocket.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
      }
      break;
    case WStype_TEXT:                     // if new text data is received
      Serial.printf("[%u] get Text: %s\n", num, payload);
      String str = (char*)payload;

      String PayloadChecker = str.substring(0, 1);
      String PayloadData = str.substring(2);
        
     if (PayloadChecker == "4") {  
        Serial.println(PayloadData); 
        SPIFFS.remove("/makeURL.txt");      
        File makeURL = SPIFFS.open("/makeURL.txt", FILE_WRITE);
        if (!makeURL) {
          Serial.println("There was an error opening the file for writing");
          return ;
        }
        if (makeURL.print("/" + PayloadData + "/" + mac_address)) {
          Serial.println("File was written");
        } else {
          Serial.println("File write failed");
        }
        makeURL.close();

        File file3 = SPIFFS.open("/makeURL.txt");
   
        if (!file3){
            Serial.println("Failed to open file for reading");
            return;
        }
        
        Serial.println("File Content:");
         
        while(file3.available()){
            Serial.write(file3.read());
        }
        
        file3.close();
     }
     break;
  }
}

