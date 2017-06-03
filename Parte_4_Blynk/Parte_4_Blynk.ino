#include <ESP8266WiFi.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Losant.h>
#include <BlynkSimpleEsp8266.h>

//Var
int PinReleState = 0;
int OldPinReleState = 0;
int RelePin = 0;
String ipAddressString;
bool conected = false;

//Authentication for Blynk Apk
char auth[] = "e5782fa3c21a4ac5a1a34c32553f36de";

// Losant credentials.
const char* LOSANT_DEVICE_ID = "58f980758502eb00013b6694";
const char* LOSANT_ACCESS_KEY = "ff8534e5-a6fd-4557-a624-b53e14909ad7";
const char* LOSANT_ACCESS_SECRET = "a7b7833dff2b654c9b373779630f62cb3295e9ebde5bac631858dece10ad4c8d";


WiFiServer server(80);
WiFiClientSecure wifiClient;
WiFiManager wifiManager;
WidgetTerminal terminal(V1);

LosantDevice device(LOSANT_DEVICE_ID);


void setup() {
  /********CONFIG SERIAL***************/
  Serial.begin(115200);
  delay(100);

  /********CONFIG GPIO*****************/
  pinMode(RelePin, OUTPUT);
  digitalWrite(RelePin, 0);

  /********CONFIG WIFI-MANAGER********/
  ConfigWifiManager();

  /******CONFIG BLYNK*****************/
  Blynk.config(auth);

  /********CONNECTING TO LOSANT********/
  ConnectToLosant();
  delay(100);

  /********SERVER START***************/
  server.begin();
  Serial.println("Server started");
}

void loop() {

  Blynk.run();

  bool toReconnectLosant = false;
  bool toReconnectWifi = false;

  if (!device.connected()) {
    Println("Disconnected from Losant");

    Serial.println(device.mqttClient.state());
    toReconnectLosant = true;
  }

  if (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Println("SmartWitch desconectado da rede wi-fi");
    Println("Reiniciando Sistema");
    toReconnectWifi = true;
  }


  if (toReconnectLosant)
    ConnectToLosant();

  if (toReconnectWifi)
    ConfigWifiManager();

  HandlerPinRele();

  device.loop();

  if (conected == true)
  {
    //Mostrar ip no app...
    Blynk.virtualWrite(V0, ipAddressString);
  }

  ServerHandler();

  delay(100);
}

void SendIpToLosant(String ip) {
  Println("Enviando ip para Losant...");

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["ipaddress"] = ip;

  // Send the state to Losant.
  device.sendState(root);
}

void ConnectToLosant()
{

  Print("Connecting to Losant...");

  device.connectSecure(wifiClient, LOSANT_ACCESS_KEY, LOSANT_ACCESS_SECRET);

  while (!device.connected()) {
    delay(500);
    Print(".");
  }

  Println("Connected!");
  Println("This device is now ready for use!");
  SendIpToLosant(ipAddressString);
}

void ConfigWifiManager()
{
  //exit after config instead of connecting
  wifiManager.setBreakAfterConfig(true);

  if (!wifiManager.autoConnect("SmartSwitch"))
  {
    Serial.println("Falha ao conectar.. Resetando sistema");
    delay(3000);
    ESP.reset();
    delay(5000);
  }

  //if you get here you have connected to the WiFi
  Print("connected at: ");
  Println(wifiManager.getSSID());
  IPAddress ip = wifiManager.getIpAddress();

  String buf = "";
  buf += ip[0]; buf += ".";
  buf += ip[1]; buf += ".";
  buf += ip[2]; buf += ".";
  buf += ip[3];
  ipAddressString = buf;

  Print("IP: ");
  Println(ipAddressString);

  conected = true;
}

void ButtonPressed() {
  Println("Button Pressed!");

  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["button"] = true;

  // Send the state to Losant.
  device.sendState(root);
}

void StateReleLosant(bool pinState) {    
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["gpiopin"] = pinState;

  // Send the state to Losant.
  device.sendState(root);
}


void ShowIpDisplay() {
  Print("Conected at: ");
  Println(wifiManager.getSSID());
  Print("IP ADDR: ");
  Println(ipAddressString);
  terminal.flush();
}

void Print(String s) {
  Serial.print(s);
  terminal.print(s);
  terminal.flush();
}

void Println(String s) {
  Serial.println(s);
  terminal.println(s);
  terminal.flush();
}

void ServerHandler()
{
  WiFiClient client = server.available();
  if (client) {

    Println("new client");
    while (!client.available()) {
      delay(1);
    }

    String req = client.readStringUntil('\r');
    //    Serial.println(req);
    client.flush();

    String buf = "";
    buf += "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n";
    buf += "<html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>\r\n";
    buf += "<title>Smart Switch ESP8266</title>";
    buf += "<style>.c{text-align: center;} div,input{padding:10px;font-size:1em;} input{width:80%;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:1.5rem;background-color:#00cca3;color:#fff;line-height:2.4rem;font-size:1.2rem;width:30%;} .q{float: right;width: 64px;text-align: right;}</style>";
    buf += "<h3>Smart Switch ESP8266</h3>";
    buf += "<a href=\"?function=carga_troca\"><button>CARGA SWITCH</button></a>";
    buf += "</html>\n";

    client.print(buf);
    client.flush();

    if (req.indexOf("carga_troca") != -1) {

      PinReleState = digitalRead(0);

      if (PinReleState == LOW) {
        digitalWrite(RelePin, HIGH);
        Println("Rele state HIGH");
        StateReleLosant(true);

      } else {
        digitalWrite(RelePin, LOW);
        Println("Rele state LOW");
        StateReleLosant(false);

      }
      ButtonPressed();
    }
    else {
      Println("invalid request");
      client.stop();
    }
    Println("Client disconnected");
  }
}

void HandlerPinRele()
{
  PinReleState = digitalRead(0);

  if (OldPinReleState != PinReleState)
  {
    OldPinReleState = PinReleState;
    ButtonPressed();
    Println("Alterando estado do relé");
  }
}
