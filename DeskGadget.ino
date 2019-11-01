#include <String.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <U8g2lib.h>
#include <NTPClient.h>

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPClient.h>
#include <Time.h>
#include <TimeLib.h>

const char *nodename    = "esp8266-weather";
const char *wifi_ssid   = "VM7430922";
const char *wifi_passwd = "8dqkvzYTjchy";   

const String APIKEY      = "1999df78b50d8d6cad6efef4a8e0ac2a";
const String countryCode = "gb";
const String cityName = "Stockton-On-Tees";

WiFiClient client;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

// Open Weather Map API server name
const char server[] = "api.openweathermap.org";
 
U8G2_ST7565_NHD_C12832_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ D3, /* data=*/ D4, /* cs=*/ D6, /* dc=*/ D7, /* reset=*/ D8);

//==============================================================================================================

typedef enum wifi_s {
  W_AP = 0, W_READY, W_TRY
} WifiStat;



const int refreshInterval = 60;//seconds
int lastRefresh = 0;

WifiStat           WF_status;

String result;

String weather = "";
String weatherDescription = "";
float intTemperature = 95;
float extTemperature;
float wind;
String timestamp;

void connectWiFiInit(void) {
  WiFi.hostname(nodename);
  String ssid   = wifi_ssid;
  String passwd = wifi_passwd;
  WiFi.begin(ssid.c_str(), passwd.c_str());

  Serial.print("Connecting");

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_open_iconic_www_4x_t);
  u8g2.drawGlyph(0, 32, 81);

  u8g2.setFont(u8g2_font_bitcasual_t_all);
  u8g2.setCursor(38, 14);
  u8g2.print(ssid);
  
  u8g2.setCursor(38, 30);
  u8g2.print("Connecting");
  u8g2.sendBuffer();  
  

  int dotCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
    dotCount++;

    Serial.print(".");
    u8g2.print(".");

    if (dotCount >= 10) {
       dotCount = 0;
       u8g2.setDrawColor(0);
       u8g2.drawBox(38, 18, u8g2.getDisplayWidth()-1, u8g2.getDisplayHeight()-1);
       u8g2.setDrawColor(1);
       u8g2.setCursor(38, 30);
       u8g2.print("Connecting");
    }
    u8g2.sendBuffer();
     
    delay(100);
  }
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());
  
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_open_iconic_www_4x_t);  
  u8g2.drawGlyph(0, 32, 81);
  
  u8g2.setFont(u8g2_font_bitcasual_t_all);
  u8g2.setCursor(38, 14);
  u8g2.print(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  
  //u8g2.drawStr(0,30,"IP: ");  // write something to the internal memory
  u8g2.setCursor(38, 30);
  u8g2.print(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");

  u8g2.sendBuffer();          // transfer internal memory to the display
}

String dateTime(String timestamp) {
  time_t ts = timestamp.toInt();
  char buff[30];
  sprintf(buff, "%2d:%02d %02d-%02d-%4d", hour(ts), minute(ts), day(ts), month(ts), year(ts));
  return String(buff);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n\n");
  u8g2.begin();  
  u8g2.enableUTF8Print();
  u8g2.setContrast(127);

  connectWiFiInit();
  printWiFiStatus();
  MDNS.begin(nodename);
  timeClient.begin();

  delay(1000);
}

void getWeatherData() {
  if (client.connect(server, 80)){ //starts client connection, checks for connection
    client.println("GET /data/2.5/weather?q=" + cityName + "," + countryCode + "&units=metric&APPID=" + APIKEY);
    client.println("Host: api.openweathermap.org");
    client.println("User-Agent: ArduinoWiFi/1.1");
    client.println("Connection: close");
    client.println();
  } else {
    Serial.println("connection failed");        //error message if no client connect
    Serial.println();
  }

  while(client.connected() && !client.available()) 
  delay(1);                                          //waits for data
  while (client.connected() || client.available())    
  {                                             //connected or data available
    char c = client.read();                     //gets byte from ethernet buffer
    result = result+c;
  }

  client.stop();                                      //stop client
  result.replace('[', ' ');
  result.replace(']', ' ');
  //Serial.println(result);
  char jsonArray [result.length()+1];
  result.toCharArray(jsonArray,sizeof(jsonArray));
  jsonArray[result.length() + 1] = '\0';
  StaticJsonDocument<1024> doc;

  DeserializationError error = deserializeJson(doc, jsonArray);

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }
    
  float _temperature = doc["main"]["temp"];
  String _weather = doc["weather"]["main"];
  String _description = doc["weather"]["description"];
  float _wind = doc["wind"]["speed"];
  String _timestamp = doc["dt"];
  timestamp = _timestamp;
  weatherDescription = _description;
  weatherDescription.setCharAt(0, weatherDescription.charAt(0) - 32);
  weather = _weather;
  extTemperature = _temperature;
  wind = _wind;
}

void displayConditions(String weather, float extTemperature, float intTemperature, float wind)
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_open_iconic_www_2x_t);
  u8g2.setCursor(0,12);
  u8g2.print((char)223);

  
  u8g2.setFont(u8g2_font_bitcasual_t_all);
  u8g2.setCursor(16, 12);
  u8g2.print(weather);

  u8g2.setCursor(0,30);
  u8g2.print("Ext:"); 
  u8g2.print(extTemperature,1);
  u8g2.print((char)223);
  u8g2.print("C  ");

  u8g2.print("Int:");
  u8g2.print(intTemperature, 1);
  u8g2.print((char)223);
  u8g2.print("C");
                                         
  u8g2.sendBuffer();
}

void displayClock()
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso28_tn);
  u8g2.setCursor(0, 31);
  
  u8g2.print(timeClient.getFormattedTime());
  
  u8g2.sendBuffer();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    timeClient.update();
    displayClock();

    /*
    if (lastRefresh == 0 || lastRefresh > refreshInterval * 1000 && millis() - lastRefresh > refreshInterval * 1000) {
      //Serial.println("Current Conditions: ");
      //getWeatherData();
      //displayConditions(weather, extTemperature, intTemperature, wind);
      
      lastRefresh = millis();
    }
    */
  }
  delay(1000);
}
