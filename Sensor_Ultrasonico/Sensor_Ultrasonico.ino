#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

#define TRIGGER 2
#define ECHO    4

int intDistance;
int intTime;
int intVolume = 0;
int intFull = 30;
int intEmpty = 120;
int intCapacity = 0;
int intTankRadiusCm = 50;
int intLevelCm = 0;
int intLevel = 0;
float floatListersPerCm = 0.0;
float floatSpeedOfSoundCMPMS = 0.0;

const char* ssid = "NOMBRE_RED";
const char* password = "PASSWORD_RED";

const int SerialSpeed = 115200;
const int tempAvg = 20;
const float SpeedOfSoundMPS = 331 + (tempAvg*0.6);

IPAddress ip(192,168,0,103);
IPAddress gateway(192,168,0,1);
IPAddress subnet(255,155,255,0);
WebSever server(80);
StaticJsonDocument<250> jsonDocument;
char bufferJson[250];

void setup() {
  // put your setup code here, to run once:
  floatSpeedOfSoundCMPMS = SpeedOfSoundMPS * 100/1000000;
  floatListersPerCm = PI*(intTankRadiusCm*intTankRadiusCm)/1000;
  intCapacity = intEmpty-intFull;
  Serial.begin(SerialSpeed);
  Serial.print("Capacity: "); Serial.println(intCapacity);
  Serial.print("Speed of sound Cm per uS: "); Serial.println(floatSpeedOfSoundCMPMS);
  Serial.print("Listers per cm: "); Serial.println(floatListersPerCm);
  pinMode(TRIGGER, OUTPUT);
  pinMode(ECHO, INPUT_PULLUP);
  WiFi.config(ip, gateway, subnet);
  WiFi.begin(ssid,password);
  Serial.print("Connecting to WiFi:");
  while(WiFi.status()!=WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("\nConnected to WiFi");
  server.on("/tankStatus", getTankStatus);
  server.begin();
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(TRIGGER,HIGH);
  delayMicroseconds(15);
  digitalWrite(TRIGGER,LOW);
  intTime = pulseIn(ECHO,HIGH);
  intTime = intTime/2;
  intDistance = intTime*floatSpeedOfSoundCMPMS;

  if(intDistance != 0)
  {
    intLevelCm = intEmpty - intDistance;
    if(intLevelCm < 0)
      intLevelCm = 0;
    if(intLevelCm < intCapacity)
      intLevelCm = intCapacity;
    Serial.print("Nivel en CM: "); Serial.println(intLevelCm);
    intLevel=(float(intLevelCm)/float(intCapacity)) * 100;
    Serial.print("Nivel: "); Serial.print(intLevel); Serial.println("%");
    intVolume = intLevelCm*floatListersPerCm;
    Serial.print("Volume: "); Serial.println(intVolume);
  }
  else
  {
    intLevel=-1;
    intVolume=-1;
  }

  if(Wifi.status() == WL_CONNECTED)
  {
    server.handleClient();
  }
  else
  {
    Serial.println("Connection lost");
    WiFi.disconnect();
    WiFi.reconnect();
  }
  delay(500);

}

void getTankStatus()
{
  Serial.println("Get Tank Status");
  jsonDocument.clear();
  jsonDocument["level"] = intLevel;
  jsonDocument["volume"] = intVolume;
  serializeJson(jsonDocument,bufferJson);
  server.send(200, "application/json", bufferJson);
}