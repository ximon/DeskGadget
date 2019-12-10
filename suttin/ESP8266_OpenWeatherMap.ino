#include <ESP8266WiFi.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
#include <string>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "icons.h"

#define OLED_RESET LED_BUILTIN//4
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16

const char *ssid = "YourSSID";
const char *pass = "Password";
const char *servername = "api.openweathermap.org";
String city = "Nagpur"; //Enter your City Name

// NTP Servers:
static const char ntpServerName[] = "us.pool.ntp.org";
const int timeZone = +530;     // Central European Time
WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

time_t getNtpTime();
void digitalClockDisplay();
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);


unsigned long oldMilli;
unsigned long newMillis;
unsigned long interval = 1000 * 60 * 5;

String old_time = "00";
//String temperature;
//String humidity;
//String icon;
String result;

WiFiClient client;

void setWeather();

void setup() {
  Serial.begin(115200);
  delay(10);
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Wifi Connected!");
  Serial.print("IP = ");
  Serial.println(WiFi.localIP());

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  //display.clearDisplay();
  //display.println(WiFi.localIP());
  //display.clearDisplay();
  //display.drawBitmap(0, 0, myBitmap, 24, 24, WHITE);
  //display.display();
  //delay(2000);
  
  display.clearDisplay();
  display.drawBitmap(48, 16, draw01d, 32, 32, WHITE);
  display.display();
  delay(500);
  
  display.clearDisplay();
  display.drawBitmap(48, 16, draw01n, 32, 32, WHITE);
  display.display();
  delay(500);

  display.clearDisplay();
  display.drawBitmap(48, 16, draw02d, 32, 32, WHITE);
  display.display();
  delay(500);


  display.clearDisplay();
  display.drawBitmap(48, 16, draw02n, 32, 32, WHITE);
  display.display();
  delay(500);


  display.clearDisplay();
  display.drawBitmap(48, 16, draw03d, 32, 32, WHITE);
  display.display();
  delay(500);

  display.clearDisplay();
  display.drawBitmap(48, 16, draw09d, 32, 32, WHITE);
  display.display();
  delay(500);

  display.clearDisplay();
  display.drawBitmap(48, 16, draw11d, 32, 32, WHITE);
  display.display();
  delay(500);

  display.clearDisplay();
  display.drawBitmap(48, 16, draw11d, 32, 32, WHITE);
  display.display();
  delay(500);

  display.clearDisplay();
  display.drawBitmap(48, 16, draw11n, 32, 32, WHITE);
  display.display();
  delay(500);
  
  //delay(2000)
  //display.display();

  setWeather();
  oldMilli = millis();

  Udp.begin(localPort);
  setSyncProvider(getNtpTime);
  setSyncInterval(300);

  
}

String getValue(char *key)
{
  String token;
  volatile int i = result.indexOf(key);
  while ((result[i] <= 47) || (result[i] >= 58 ))
  {
    i++;
  }

  while (result[i] != ',') {
    token += result[i];
    i++;
  }
  return token;
}

time_t prevDisplay = 0; // when the digital clock was displayed
String prevTime = "";
int prevDay = 0;
String prevWeekDay = "";

void loop() {
  newMillis = millis();
  
   if (timeStatus() != timeNotSet) {
    if (minute() != prevDisplay) { //update the display only if time has changed
      prevDisplay = minute();
      digitalClockDisplay();
    }
  }

  if (newMillis - oldMilli > interval)
  {
    setWeather();
    digitalClockDisplay();
  }
}

void digitalClockDisplay()
{
  String currTime;
  String currDate;
  
  if(hour() < 10)
    currTime += "0";
  currTime += String(hour()) + ":";

  if(minute() < 10)
    currTime += "0";
  currTime += String(minute());

  Serial.print("Time = ");
  Serial.println(currTime);
  display.setTextColor(BLACK);
  display.setCursor(0, 0);
  display.print(prevTime);
  display.display();
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.print(currTime);

  if (day() != prevDay)
  {
    //prevDay = day();
    display.setCursor(95, 0);
    display.setTextSize(1);
    display.setTextColor(BLACK);
    display.println(prevDay);
    
    display.setCursor(95, 0);
    display.setTextColor(WHITE);
    
    if(day() < 10)
      currDate += "0";
    currDate += String(day()) + "/";

    if(month() < 10)
      currDate += "0";
    currDate += String(month()) /*+ "/" + String(year())*/;
    display.print(currDate);

    display.setCursor(95, 8);
    display.setTextColor(BLACK);
    display.println(prevWeekDay);
    display.setCursor(95, 8);
    display.setTextColor(WHITE);
    
    switch (weekday()) {

        case 1:
          display.print("Sun");
          prevWeekDay = "Sun";
          break;

        case 2:
          display.print("Mon");
          prevWeekDay = "Mon";
          break;

        case 3:
          display.print("Tue");
          prevWeekDay = "Tue";
          break;

        case 4:
          display.print("Wed");
          prevWeekDay = "Wed";
          break;

        case 5:
          display.print("Thu");
          prevWeekDay = "Thu";
          break;

        case 6:
          display.print("Fri");
          prevWeekDay = "Fri";
          break;

        case 7:
          display.print("Sat");
          prevWeekDay = "Sat";
          break;

        default:
          break;
        
      }
  }
  else {
      display.setTextSize(1);
      display.setCursor(95, 0);
      display.println(currDate);
      display.setCursor(95, 8);
      display.println(prevWeekDay);
      display.setTextSize(2);
  }
  
  display.display();
   display.setTextSize(2);
  
  prevTime = currTime;
}


void setWeather()
{

  String temperature;
  String humidity;
  String icon;
  
  oldMilli = millis();

  client.stop();
  if (client.connect(servername, 80))
  {
    client.println("GET /data/2.5/weather?q="+ city + "&units=metric&APPID=<your api key>&mode=json&cnt=2 HTTP/1.1");
    client.println("Host: api.openweathermap.org");
    client.println("User-Agent: ESP8266");
    client.println("Connection: close");
    client.println();

    while (client.connected() || client.available()) {
      char c = client.read();
      result = result + c;
    }

    Serial.println(result);
    temperature = getValue("\"temp\":");
    humidity = getValue("\"humidity\":");
    icon = getValue("\"icon\"");
    result = "";

    Serial.print("Icon = ");
    Serial.println(icon);
    Serial.print("Temperature = ");
    Serial.println(temperature);
    Serial.print("Humidity = ");
    Serial.println(humidity);

    client.stop();

    display.clearDisplay();
    display.setCursor(0, 20);
    display.print(temperature);
    display.setTextSize(1);
    display.print(" o");
    display.setTextSize(2);
    display.println("C");
    display.print(humidity);
    display.println(" %");
    display.setCursor(49, 54);
    display.setTextSize(1);
    display.print(city);
    display.display();
    display.setTextSize(2);
    
    if (icon == "01d\"}]")
      display.drawBitmap(75, 20, draw01d, 32, 32, WHITE);

    if (icon == "01n\"}]")
      display.drawBitmap(75, 20, draw01n, 32, 32, WHITE);

    if (icon == "02d\"}]")
      display.drawBitmap(75, 20, draw02d, 32, 32, WHITE);

    if (icon == "02n\"}]")
      display.drawBitmap(75, 20, draw02n, 32, 32, WHITE);

    if (icon == "03d\"}]")
      display.drawBitmap(75, 20, draw03d, 32, 32, WHITE);

    if (icon == "03n\"}]")
      display.drawBitmap(75, 20, draw03n, 32, 32, WHITE);

    if (icon == "04d\"}]")
      display.drawBitmap(75, 20, draw04d, 32, 32, WHITE);

    if (icon == "04n\"}]")
      display.drawBitmap(75, 20, draw04n, 32, 32, WHITE);

    if (icon == "09d\"}]")
      display.drawBitmap(75, 20, draw09d, 32, 32, WHITE);

    if (icon == "09n\"}]")
      display.drawBitmap(75, 20, draw09n, 32, 32, WHITE);

    if (icon == "10d\"}]")
      display.drawBitmap(75, 20, draw10d, 32, 32, WHITE);

    if (icon == "10n\"}]")
      display.drawBitmap(75, 20, draw10n, 32, 32, WHITE);

    if (icon == "11d\"}]")
      display.drawBitmap(75, 20, draw11d, 32, 32, WHITE);

    if (icon == "11n\"}]")
      display.drawBitmap(75, 20, draw11n, 32, 32, WHITE);

    if (icon == "50d\"}]")
      display.drawBitmap(75, 20, draw50d, 32, 32, WHITE);

    if (icon == "50n\"}]")
      display.drawBitmap(75, 20, draw50n, 32, 32, WHITE);

    display.display();
  }
}

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + 19800; //Indian Time zone is +5:30 or 19800 epoch. Adjust according to your timezone
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
