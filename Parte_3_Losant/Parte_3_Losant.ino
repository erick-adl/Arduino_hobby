#include <ESP8266WiFi.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Losant.h>

int PinReleState = 0;
int RelePin = 0;

String ipAddressString;
WiFiServer server(80);



// Losant credentials.
const char* LOSANT_DEVICE_ID = "58f980758502eb00013b6694";
const char* LOSANT_ACCESS_KEY = "ff8534e5-a6fd-4557-a624-b53e14909ad7";
const char* LOSANT_ACCESS_SECRET = "a7b7833dff2b654c9b373779630f62cb3295e9ebde5bac631858dece10ad4c8d";

WiFiClientSecure wifiClient;
LosantDevice device(LOSANT_DEVICE_ID);


void SendIpToLosant(String ip) {
  Serial.println("Enviando ip para Losant...");
  
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["ipaddress"] = ip;

  // Send the state to Losant.
  device.sendState(root);
}

void ConnectToLosant()
{
  // Connect to Losant.
  Serial.println();
  Serial.print("Connecting to Losant...");

  device.connectSecure(wifiClient, LOSANT_ACCESS_KEY, LOSANT_ACCESS_SECRET);

  while (!device.connected()) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("Connected!");
  Serial.println();
  Serial.println("This device is now ready for use!");
  SendIpToLosant(ipAddressString);
}

void ConfigWifiManager()
{
  WiFiManager wifiManager;
  wifiManager.autoConnect("SmartSwitch");

  Serial.print("connected at: ");
  Serial.println(wifiManager.getSSID());
  Serial.print("IP: ");
  Serial.println(wifiManager.getIpAddress());
  IPAddress ip = wifiManager.getIpAddress(); 
  String buf = "";  
  buf += ip[0];buf += ".";  
  buf += ip[1];buf += ".";
  buf += ip[2];buf += ".";
  buf += ip[3];  
  ipAddressString = buf;
  Serial.print("IP: ");
  Serial.println(ipAddressString);
}

void setup() {
  /*CONFIG SERIAL*/
  Serial.begin(115200);
  delay(100);
  Serial.print("Starting...");


  /*CONFIG GPIO*/
  pinMode(RelePin, OUTPUT);
  digitalWrite(RelePin, 0);

  /*CONFIG WIFI-MANAGER*/
  ConfigWifiManager();

  /*SERVER START*/
  server.begin();
  Serial.println("Server started");

  ConnectToLosant();

}

void buttonPressed(bool b) {
  Serial.println("Button Pressed!");

  // Losant uses a JSON protocol. Construct the simple state object.
  // { "button" : true }
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["button"] = b;

  // Send the state to Losant.
  device.sendState(root);
}

void loop() {

//  bool toReconnectWifi = false;
  bool toReconnectLosant = false;
//
//  if (WiFi.status() != WL_CONNECTED) {
//    Serial.println("Disconnected from WiFi");
//    toReconnectWifi = true;
//  }
  if (!device.connected()) {
    Serial.println("Disconnected from Losant");
    Serial.println(device.mqttClient.state());
    toReconnectLosant = true;
  }
//
//  if (toReconnectWifi)
//    ConfigWifiManager();

  if (toReconnectLosant)
    ConnectToLosant();

  device.loop();

  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  Serial.println("new client");
  while (!client.available()) {
    delay(1);
  }

  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

  String buf = "";


  buf += "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n";
  buf += "<html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>\r\n";
  buf += "<title>ESP8266 Web Server</title>";
  buf += "<style>.c{text-align: center;} div,input{padding:10px;font-size:1em;} input{width:80%;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float: right;width: 64px;text-align: right;}</style>";
  buf += "</head>";
  buf += "<h3>ESP8266 IoT Home</h3>";
  buf += "<a href=\"?function=carga_troca\"><button>CARGA SWITCH</button></a>";
  buf += "</html>\n";

  client.print(buf);
  client.flush();

  if (req.indexOf("carga_troca") != -1) {

    PinReleState = digitalRead(0);

    if (PinReleState == LOW) { 
      digitalWrite(RelePin, HIGH);
      buttonPressed(true);
      Serial.println("Rele state HIGH");
    } else { 
      digitalWrite(RelePin, LOW);
      buttonPressed(false);
      Serial.println("Rele state LOW");
    }
    
  }
  else {
    Serial.println("invalid request");
    client.stop();
  }
  Serial.println("Client disonnected");

  delay(100);
}
