#include <ESP8266WiFi.h>
 
const char* ssid = "HotSpot";
const char* password = "amapa118";
bool interruptor; 
int PinReleState = 0;

int RelePin = 0;

WiFiServer server(80);
 
void setup() {
  Serial.begin(115200);
  delay(10);
 
  // prepare GPIO2
  pinMode(1, OUTPUT);
  digitalWrite(0, 0);
 
  pinMode(RelePin, OUTPUT);
  digitalWrite(RelePin, 0);
 
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");
 
  // Start the server
  server.begin();
  Serial.println("Server started");
  Serial.println(WiFi.localIP());
}
 
void loop() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
 
  Serial.println("new client");
  while(!client.available()){
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

  if (req.indexOf("carga_troca") != -1){
    
    PinReleState = digitalRead(0);
    
    if(PinReleState == LOW){
      interruptor = false;
      digitalWrite(RelePin, HIGH);
    }else{
      interruptor = true;
      digitalWrite(RelePin, LOW);      
    }
  }

  else {
    Serial.println("invalid request");
    client.stop();
  }
  Serial.println("Client disonnected");
}
