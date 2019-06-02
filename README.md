# IOT project ESP32

With this project you can add a Firebase user account and specify it to a specific ESP32. This happens in the local webserver on the ESP32, the ESP32 will add it's Wi-Fi MAC address to the realtime database on Firebase and the will get the UID from a Firebase account and store it on the ESP32. This not only wil make a specific user link, but it wil also make it possible to add more ESP32 to a single account.

#Get started

download and add all the libraries below into your project.

```cpp
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

```

- Use a MQTT broker like Eclipse and paste the link into your project.

```cpp
const char* mqtt_server = "linktoeclipse";

```

- Specify the right LED channel

```cpp
int ledChannel = 0;

```

- In this example the password to connect to the captive portal is: Protected.

You can specify another one here:

```cpp
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

```

- Make a firebase account

Store the credentials from the firebase account in the data folder in the index.html file.

```cpp
  var config = {
    apiKey: "",
    authDomain: "",
    databaseURL: "",
    projectId: "",
    storageBucket: "",
    messagingSenderId: ""
  };
  firebase.initializeApp(config);
  
```

