#include <SoftwareSerial.h>
#include <SPI.h>
#include <MFRC522.h>

#define RST_PIN         10
#define SS_PIN          9

MFRC522 mfrc522(SS_PIN, RST_PIN);

MFRC522::MIFARE_Key key;

#define DEBUG true
SoftwareSerial esp8266(2, 3);

const int trigPin = 4;
const int echoPin = 5;
long duration; int distance;

byte a = 0xD7;
byte b = 0x61;
byte charan = 0xC5;
byte c = 0x50;
byte rc = 0x00;
byte prc = 0x00;
int flag = 1;
int perc;

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  esp8266.begin(115200);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);

  sendData("AT+RST\r\n", 2000, DEBUG);
  sendData("AT+CWMODE=2\r\n", 1000, DEBUG);
  sendData("AT+CIFSR\r\n", 1000, DEBUG);
  sendData("AT+CIPMUX=1\r\n", 1000, DEBUG);
  sendData("AT+CIPSERVER=1,80\r\n", 1000, DEBUG);

  SPI.begin();
  mfrc522.PCD_Init();
}

void loop()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration / 29 / 2;
  perc = 100 - (distance*100)/26;
  if(perc>100 || perc < 0){perc = 0;}
  if(distance > 23) perc = 0;
  if(distance < 6) perc = 100;

  Serial.println(distance);
  delay(1000);

  if (esp8266.available())
  {
    while (esp8266.find("+IPD,"))
    {
      int connectionId = esp8266.read() - 48;

      String webpage = "<p><center><h1>Wastebin Monitoring System</h1>";
      webpage += "<h2>Bin 1<br> Bin is ";
      webpage += perc;
      webpage += "% filled<br>";
      webpage += "<a href=\"https://goo.gl/maps/GPwo8uqhViP2\">Location of the bin</a><br>";
      
      if(flag == 0)
      {
        webpage += "Cleaning in progress by ";
        //webpage += "";
        if (rc == a) {
          webpage += "Narayana<br>";
        }
        else if (rc == b || rc == charan) {
          webpage += "Charan<br>";
        }
        else if (rc == c) {
          webpage += "Rahul<br>";
        }
      }
      else{
      webpage += "Last cleaned by ";

      if (prc == a) {
        webpage += "Narayana";
      }
      else if (prc == b || prc == charan) {
        webpage += "Charan";
      }
      else if (prc == c) {
        webpage += "Rahul";
      }
      else {
        webpage += "none";
      }
      }
      webpage += "</h2></center></p>";
      String cipSend = "AT+CIPSEND=";
      cipSend += connectionId;
      cipSend += ",";
      cipSend += webpage.length();
      cipSend += "\r\n";
      sendData(cipSend, 100, DEBUG);
      sendData(webpage, 100, DEBUG);
    }
  }

  if ( ! mfrc522.PICC_IsNewCardPresent())
    return;

  if ( ! mfrc522.PICC_ReadCardSerial())
    return;

  byte readCard[4] ;

  for (int i = 0; i < mfrc522.uid.size; i++) {
    readCard[i] = mfrc522.uid.uidByte[i];
    Serial.print(readCard[i], HEX);
  }
  Serial.println("");

  rc = readCard[0];
  if (flag == 0)
  {
    flag = 1;
    prc = rc;
  }
  else if (flag == 1)
  {
    flag = 0;
  }
}

String sendData(String command, const int timeout, boolean debug)
{
  String response = "";
  esp8266.print(command);
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (esp8266.available())
    {
      char c = esp8266.read();
      response += c;
    }
  }
  if (debug)
  {
    Serial.println(response);
  }
  return response;
}
