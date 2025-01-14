/*
Code: Kevin Blackistone 2025
From work designed to 
  read EMG signals from the AD8232 boards
  send those signals over OSC port 9999
  receive shock trigger signals: OSC port 8888
Shock signals are limited in time and auto-off/fail safe
 
Sends: 2x AD8232 EMG signals
Receives: 1x shock trigger -> relay ->  ems device
*/


#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include <OSCBundle.h>
#include <OSCData.h>

char ssid[] = "XXXXXXXXX";          // your network SSID (name)
char pass[] = "XXXXXXXXX";          // your network password

WiFiUDP Udp;
const IPAddress outIp(###,###,###,###);        // remote IP (not needed for receive)
const unsigned int outPort = 9999;          // remote port (not needed for receive)
const unsigned int localPort = 8888;        // local port to listen for UDP packets (here's where we send the packets)


/**********************************/
#define SHOCKPIN1 4 
#define LEDPIN 32 
// CHANGE THIS WHEN YOU SORT IT!!!!!!!
/**********************************/

OSCErrorCode error;
unsigned int ledState = LOW;              // LOW means led is *on*
unsigned int emsState = 0;
bool shockOn = false;
unsigned long shockBeginTime = 0;
const unsigned long shockDuration = 500;



#ifndef BUILTIN_LED
#ifdef LED_BUILTIN
#define BUILTIN_LED LED_BUILTIN
#else
#define BUILTIN_LED 13
#endif
#endif

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, ledState);    // turn *on* led

  Serial.begin(115200);
  delay(1000);
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  WiFi.mode(WIFI_STA); //Optional
  WiFi.disconnect(); 
  Serial.print("Connecting to ");
  Serial.println(ssid);
  delay(1000);
  
  WiFi.begin(ssid, pass);

  uint32_t notConnectedCounter = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    Serial.print(".");
    notConnectedCounter++;
    if(notConnectedCounter > 150) { // Reset board if not connected after 15s
        Serial.println("Resetting due to Wifi not connecting...");
        ESP.restart();
    }
  }
  Serial.println("");

  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
#ifdef ESP32
  Serial.println(localPort);
#else
  Serial.println(Udp.localPort());
#endif

  Serial.println("Setting Pin Modes...");

/*
  Serial.println("A0");
  // Setup ECG 1
  pinMode(A0, INPUT);
  Serial.println("9");
  //pinMode(9, INPUT);   // U1 RX Setup for leads off detection LO +
  Serial.println("10");
  pinMode(10, INPUT);  // U1 TX Setup for leads off detection LO -

  // Setup ECG 2

  Serial.println("A3");
  pinMode(A3, INPUT);
  Serial.println("3");
  // pinMode(3, INPUT);   // U0 RX Setup for leads off detection LO +
  Serial.println("1");
  pinMode(1, INPUT);   // U0 TX Setup for leads off detection LO -

*/
  Serial.println("shocker");
  // setup shocker
  pinMode(SHOCKPIN1, OUTPUT);
  pinMode(LEDPIN, OUTPUT);
  // digitalWrite(SHOCKPIN, LOW); // just in case

  Serial.println("Setup Complete");
}

// ems ==> EMS (Electro Muscular Stimulation... ie. the shocker)
void ems(OSCMessage &msg) {

  emsState = msg.getInt(0);               // get the ems OSC
  if ((emsState == 1) && (!shockOn)){     // if it's 1
    shockOn = true;
    shockBeginTime = millis();
    Serial.println("SHOCK!!");
    digitalWrite(SHOCKPIN1, HIGH);
    digitalWrite(LEDPIN, HIGH);
  }
  else if (emsState == 0){

    digitalWrite(SHOCKPIN1, LOW);
    digitalWrite(LEDPIN, LOW);
    Serial.println("ABORT!");
  }
  Serial.print("/ems: ");
  Serial.println(emsState);
}

void loop() {
  
  OSCMessage msg;
  int size = Udp.parsePacket();

  if (size > 0) {
    while (size--) {
      msg.fill(Udp.read());
    }
    if (!msg.hasError()) {
      msg.dispatch("/ems", ems);
    } else {
      error = msg.getError();
      Serial.print("error: ");
      Serial.println(error);
    }
  }

  if (shockOn){
    if (millis() - shockBeginTime >= shockDuration){
      digitalWrite(SHOCKPIN1, LOW);
    digitalWrite(LEDPIN, LOW);
      Serial.println("Shock over...");
      shockOn = false;
    }
  }

  int32_t ecgVal1 = analogRead(A0);
  int32_t ecgVal2 = analogRead(A3);
  // Serial.println(ecgVal1, ecgVal2);

  OSCMessage ecgMsg("/ecg/0");

  ecgMsg.add(ecgVal1);
  ecgMsg.add(ecgVal2);

  Udp.beginPacket(outIp, outPort);
  ecgMsg.send(Udp);
  Udp.endPacket();
  ecgMsg.empty();

}
