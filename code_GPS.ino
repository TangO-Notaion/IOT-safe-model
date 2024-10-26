#include <TinyGPS++.h>
#include <HardwareSerial.h>
#include <TridentTD_LineNotify.h>
#include <WiFi.h>

#define RXPin (16)
#define TXPin (17)

#define LINE_TOKEN "ZEqXoOMFnqVvJtERXQjJVMiDmIaaOSKOJRyUZ01LOFT"
#define SSID "Tang0"             // Change the name of the WiFi you want to connect to.
#define PASSWORD "Tango-051049"  // Change the WiFi password to connect.

static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
HardwareSerial ss(2);

String prevlat = "", prevlng = "", nowlat = "",nowlng = "";

void myDelay(unsigned long del) 
{
  unsigned long myPrevMillis = millis();
  unsigned long myCurrentMillis = myPrevMillis;
  while (myCurrentMillis - myPrevMillis <= del) 
  {
    myCurrentMillis = millis();
  }
  return;
}

void displayInfo() 
{
  Serial.print(F("SOS your safe location has change to : "));
  if (gps.location.isValid()) 
  {
    //Serial.print(gps.location.lat(), 6);
    Serial.print(nowlat);
    Serial.print(F(","));
    //Serial.print(gps.location.lng(), 6);
    Serial.print(nowlng);
  } 
  else 
  {
    Serial.print(F("INVALID"));
  }
  Serial.println();
}

void sendLocationToLine() 
{
  LINE.notify("SOS your safe location has change");
  if (gps.location.isValid()) 
  {
    // String latitude = String(gps.location.lat(), 6);
    // String longitude = String(gps.location.lng(), 6);
    String googleMapLink = "https://www.google.com/maps/place/" + nowlat + "," + nowlng;
    LINE.notify(googleMapLink);
  } 
  else 
  {
    LINE.notify("Invalid GPS Location");
  }
}

void setup() 
{
  Serial.begin(115200);
  ss.begin(GPSBaud, SERIAL_8N1, RXPin, TXPin, false);
  Serial.println(TinyGPSPlus::libraryVersion());
  WiFi.begin(SSID, PASSWORD);
  Serial.printf("WiFi connecting ", SSID);
  while (WiFi.status() != WL_CONNECTED) 
  {
    Serial.print(".");
    myDelay(500);
  }
  Serial.printf("\nWiFi connected\nIP : ");
  Serial.println(WiFi.localIP());
  LINE.setToken(LINE_TOKEN);
  LINE.notify("GPS เองจ้า");
}

void loop() 
{
  while (ss.available() > 0)
  {
    if (gps.encode(ss.read()))
    {
      Serial.print(F("Number of satellites in use = "));
      Serial.println(gps.satellites.value());

      if (gps.location.isValid()) 
      {
        // if (gps.location.isUpdated()) //too sensitive 
        // {
        //   Serial.print("upLAT="); Serial.print(gps.location.lat(), 6);
        //   Serial.println(" upLNG="); Serial.println(gps.location.lng(), 6);
        //   Serial.println("");
        // }
        Serial.print("prev coor : ");
        Serial.print(prevlat);
        Serial.print(F(","));
        Serial.println(prevlng); 
        nowlat = String(gps.location.lat(), 5);
        nowlng = String(gps.location.lng(), 5);
        Serial.print("now coor : ");
        Serial.print(nowlat);
        Serial.print(F(","));
        Serial.println(nowlng); 
        if (prevlat != nowlat || prevlng != nowlng) 
        {
          displayInfo();
          sendLocationToLine();
        }
        prevlat = nowlat;
        prevlng = nowlng;
      } 
      else 
      {
        Serial.println(F("INVALID"));
      }
      myDelay(1000);
    }
  }

  if (millis() > 5000 && gps.charsProcessed() < 10) 
  {
    Serial.println(F("No GPS detected: check wiring."));
    while (true);
  }
}
