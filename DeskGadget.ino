#include <String.h>
#include <Arduino.h>
#include <U8g2lib.h>

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
#include "OpenWeatherMap.h"


const char *ow_key      = "1999df78b50d8d6cad6efef4a8e0ac2a";
const char *nodename    = "esp8266-weather";
const char *wifi_ssid   = "VM7430922";
const char *wifi_passwd = "8dqkvzYTjchy";   

WiFiClient client;

// Open Weather Map API server name
const char server[] = "api.openweathermap.org";
 
U8G2_ST7565_NHD_C12832_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ D3, /* data=*/ D4, /* cs=*/ D6, /* dc=*/ D7, /* reset=*/ D8);

//==============================================================================================================

typedef enum wifi_s {
  W_AP = 0, W_READY, W_TRY
} WifiStat;

const char *countryCode = "gb";
const char *cityName = "Stockton-On-Tees";


OWMconditions      owCC;
OWMfiveForecast    owF5;
WifiStat           WF_status;

void printWiFiStatus() {  
  u8g2.clearBuffer();          // clear the internal memory
  u8g2.setFont(u8g2_font_open_iconic_www_2x_t);
  u8g2.drawGlyph(0, 18, 81);
  u8g2.setFont(u8g2_font_bitcasual_t_all);
  u8g2.setCursor(19, 14);
  u8g2.print(WiFi.SSID());
  u8g2.drawStr(0,30,"Connected!");  // write something to the internal memory
  u8g2.sendBuffer();          // transfer internal memory to the display
  
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}

void connectWiFiInit(void) {
  WiFi.hostname(nodename);
  String ssid   = wifi_ssid;
  String passwd = wifi_passwd;
  WiFi.begin(ssid.c_str(), passwd.c_str());

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_open_iconic_www_2x_t);
  u8g2.drawGlyph(0, 18, 81);
  u8g2.setFont(u8g2_font_bitcasual_t_all);
  u8g2.setCursor(19, 14);
  u8g2.print(ssid);
  u8g2.setCursor(0, 30);
  u8g2.print("Connecting");
  u8g2.sendBuffer();  

  int dotCount = 0;
  while (WiFi.status() != WL_CONNECTED) {
    dotCount++;

    Serial.print(".");
    u8g2.print(".");
    u8g2.sendBuffer();

    if (dotCount >= 6) {
       dotCount = 0;
       u8g2.setDrawColor(0);
       u8g2.drawBox(0, 18, u8g2.getDisplayWidth()-1, u8g2.getDisplayHeight()-1);
       u8g2.setDrawColor(1);
       u8g2.setCursor(0, 30);
       u8g2.print("Connecting");
    }
     
    delay(100);
  }

  printWiFiStatus();

  delay(1000);
}

String dateTime(String timestamp) {
  time_t ts = timestamp.toInt();
  char buff[30];
  sprintf(buff, "%2d:%02d %02d-%02d-%4d", hour(ts), minute(ts), day(ts), month(ts), year(ts));
  return String(buff);
}

void setup() {
  Serial.begin(115200);
  Serial.println("\n\n\n\n");
  u8g2.begin();  
  u8g2.enableUTF8Print();

  connectWiFiInit();
  WF_status  = W_TRY;
}

void currentConditions(void) {
  OWM_conditions *ow_cond = new OWM_conditions;
  owCC.updateConditions(ow_cond, ow_key, countryCode, cityName, "metric");
  Serial.print("Latitude & Longtitude: ");
  Serial.print("<" + ow_cond->longtitude + " " + ow_cond->latitude + "> @" + dateTime(ow_cond->dt) + ": ");
  Serial.println("icon: " + ow_cond->icon + ", " + " temp.: " + ow_cond->temp + ", press.: " + ow_cond->pressure);
  Serial.println("Descr: " + ow_cond->description);
  delete ow_cond;
}

void fiveDayFcast(void) {
  OWM_fiveForecast *ow_fcast5 = new OWM_fiveForecast[40];
  byte entries = owF5.updateForecast(ow_fcast5, 40, ow_key, countryCode, cityName, "metric");
  Serial.print("Entries: "); Serial.println(entries+1);
  for (byte i = 0; i <= entries; ++i) { 
    Serial.print(dateTime(ow_fcast5[i].dt) + ": icon: ");
    Serial.print(ow_fcast5[i].icon + ", temp.: [" + ow_fcast5[i].t_min + ", " + ow_fcast5[i].t_max + "], press.: " + ow_fcast5[i].pressure);
    Serial.println(", descr.: " + ow_fcast5[i].description + ":: " + ow_fcast5[i].cond + " " + ow_fcast5[i].cond_value);
  }
  delete[] ow_fcast5;
}

void loop() {
  if (WF_status == W_TRY) {
    if (WiFi.status() == WL_CONNECTED) {
      MDNS.begin(nodename);
      WF_status = W_READY;
      Serial.println("Current Conditions: ");
      currentConditions();
      Serial.println("Five days forecast: ");
      fiveDayFcast();
    }
  }
  delay(1000);
}
