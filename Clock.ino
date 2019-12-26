//#define USE1306

#include <WiFi.h>
#include <BLEDevice.h>
#include <BLE2902.h>
#include <NeoPixelBus.h>
#include <EEPROM.h>

#ifdef USE1306
#include "SSD1306.h" 
#include "OLEDDisplayUi.h"
#endif

std::string name = "Clock";

struct tm timeinfo;

int state = 0;
unsigned long tryWifi = 0;
unsigned long tryBLE = 0;
unsigned long ipTime = 0;
String ssid = String("");
String password = String("");
const char* ntpServer = "pool.ntp.org";
String gmtOffset = "";
char _timeString[50];

#ifdef USE1306
SSD1306  display(0x3c, 5, 4);
OLEDDisplayUi ui ( &display );
int screenW = 128;
int screenH = 64;
int overlayHeight = 20;
int clockCenterX = screenW / 2;
int clockHeight = screenH - overlayHeight;
int clockRadius = clockHeight / 2;
int clockCenterY = overlayHeight + clockRadius;
#endif

struct RGB {
  byte r;
  byte g;
  byte b;
};

int ledIndex = 0;
unsigned long ledTime = 0;
int ledOffset = 27;

#define NEOPIN 16
#define NUMPIXELS 60
NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(NUMPIXELS, NEOPIN);
RGB rgb[NUMPIXELS];

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
BLECharacteristic *pCharacteristic;
bool BLEConnected = false;

#define EEPROM_SIZE 100

int oldSecond = -1;
int millisec = 0;

RGB getHourColor(int hour)
{
  byte r = 0;
  byte g = 0;
  byte b = 0;
  if      (hour % 6 == 0)   r = 127;
  else if (hour % 6 == 1) { r = 127; g = 127; }
  else if (hour % 6 == 2)   g = 127;
  else if (hour % 6 == 3) { g = 127; b = 127; }
  else if (hour % 6 == 4)   b = 127;
  else if (hour % 6 == 5) { r = 127; b = 127; }
  return { r, g, b };
}

void updateClock() 
{    
    int second = timeinfo.tm_sec;
    int minute = timeinfo.tm_min;
    int hour = timeinfo.tm_hour;
    
    // jiffy = 1/60th of a second
    int ms = millis();
    if (oldSecond != second)
    {
      oldSecond = second;
      millisec = ms;
    }
    int millisecond = min(999, ms - millisec);
    int jiffy = (int)(3 * millisecond / 50);

    if (minute == 0 && second == 0)
    {
      if((hour % 12) == 0) 
      {
        theaterChaseRainbow(50);
      }
      else
      {
        RGB c = getHourColor(hour);
        theaterChase(RgbColor(c.r, c.g, c.b), 50);
      }
    }   
    else 
    {   
      clock(5 * (hour % 12) + minute / 12, minute, second, jiffy);
    }
}

void clock(int hour, int minute, int second, int jiffy)
{  
    bool show = false;
    for (int ii = 0; ii < 60; ii++)
    {    
      RGB c = { 0, 0, 0 };
      
      if (ii == second && second != hour && second != minute) 
      {
        c.b = 10;
      }
             
      if (ii == minute && hour != minute) 
      {
        c.g = 20;
      }
      
      if (ii == hour) 
      {
        c.r = 40;
      }
                 
      if (second == 59 && ii == jiffy)
      {
        c.b += 1;
      }   
      else if (c.r == 0 && c.g == 0 && c.b == 0 && (ii % 5) == 0)
      {
        c.r += 1;
        c.g += 1;
        c.b += 1;
      }

      if (c.r != rgb[ii].r || c.g != rgb[ii].g || c.b != rgb[ii].b)
      {
        show = true;
      }
      
      int index = (ii + ledOffset) % 60;
      strip.SetPixelColor(index, RgbColor(c.r, c.g, c.b));
      rgb[ii] = c;      
    }

    if (show)
    {
      strip.Show();
    }
}

void setOneColor(int index, RGB color)
{
    RGB off = { 0, 0, 0 };
    for (int ii = 0; ii < 60; ii++)
    { 
      RGB c = (ii == index) ? color : off;
      strip.SetPixelColor(ii, RgbColor(c.r, c.g, c.b));
    }
    strip.Show();
}

void theaterChase(RgbColor c, uint8_t wait) 
{
  for (int j = 0; j < 10; j++) 
  { 
    for (int q = 0; q < 3; q++) 
    {
      for (uint16_t i = 0; i < NUMPIXELS; i = i + 3) 
      {
        strip.SetPixelColor(i+q, c);
      }
      strip.Show();

      delay(wait);

      for (uint16_t i = 0; i < NUMPIXELS; i = i + 3) 
      {
        strip.SetPixelColor(i+q, 0);
      }
    }
  }
}

void theaterChaseRainbow(uint8_t wait) 
{
  for (int j = 0; j < 256; j++) 
  {
    for (int q = 0; q < 3; q++) 
    {
      for (uint16_t i = 0; i < NUMPIXELS; i = i + 3) 
      {
         strip.SetPixelColor(i + q, Wheel((i + j) % 255));
      }
      strip.Show();

      delay(wait);

      for (uint16_t i = 0; i < NUMPIXELS; i = i + 3) 
      {
        strip.SetPixelColor(i + q, 0);
      }
    }
  }
}

RgbColor Wheel(byte WheelPos) 
{
  WheelPos = 255 - WheelPos;
  
  if(WheelPos < 85) 
  {
    return RgbColor(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  
  if (WheelPos < 170) 
  {
    WheelPos -= 85;
    return RgbColor(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  
  WheelPos -= 170;
  return RgbColor(WheelPos * 3, 255 - WheelPos * 3, 0);
}

#ifdef USE1306

String twoDigits(int digits)
{
  return (digits < 10) ? '0' + String(digits) : String(digits);
}

void clockOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) 
{
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_16);
  display->drawString(clockCenterX, 0, (String)name.c_str());
}

void analogClockFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) 
{  
  int second = timeinfo.tm_sec;
  int minute = timeinfo.tm_min;
  int hour = timeinfo.tm_hour;

  display->drawCircle(clockCenterX + x, clockCenterY + y, 2);

  for (int z = 0; z < 360; z += 30)
  {
    float angle = z ;
    angle = ( angle / 57.29577951 );
    int x2 = ( clockCenterX + ( sin(angle) * clockRadius ) );
    int y2 = ( clockCenterY - ( cos(angle) * clockRadius ) );
    int x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 8 ) ) ) );
    int y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 8 ) ) ) );
    display->drawLine( x2 + x , y2 + y , x3 + x , y3 + y);
  }

  float angle = second * 6 ;
  angle = ( angle / 57.29577951 );
  int x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 5 ) ) ) );
  int y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 5 ) ) ) );
  display->drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);

  angle = minute * 6 ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 4 ) ) ) );
  y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 4 ) ) ) );
  display->drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
 
  angle = hour * 30 + int( ( minute / 12 ) * 6 )   ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 2 ) ) ) );
  y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 2 ) ) ) );
  display->drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
}

void wifiFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) 
{
  String message = ssid;
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_16);
  display->drawString(clockCenterX + x, clockCenterY + y, (String)message);
}

void digitalClockFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) 
{
  int second = timeinfo.tm_sec;
  int minute = timeinfo.tm_min;
  int hour = timeinfo.tm_hour;
  
  String timenow = String(hour)+":"+twoDigits(minute)+":"+twoDigits(second);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_24);
  display->drawString(clockCenterX + x , clockCenterY + y, timenow );
}

int frameCount = 3;
int overlaysCount = 1;
FrameCallback frames[] = { analogClockFrame, digitalClockFrame, wifiFrame };
OverlayCallback overlays[] = { clockOverlay };

#endif

void connectWifi()
{
  // WiFi status codes
  // WL_IDLE_STATUS      = 0
  // WL_NO_SSID_AVAIL    = 1
  // WL_SCAN_COMPLETED   = 2
  // WL_CONNECTED        = 3
  // WL_CONNECT_FAILED   = 4
  // WL_CONNECTION_LOST  = 5
  // WL_DISCONNECTED     = 6
  // WL_NO_SHIELD        = 255
  
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.printf("Connecting to %s ", ssid.c_str());
    WiFi.begin(ssid.c_str(), password.c_str());
    for (int ii = 0; ii < 100 && WiFi.status() != WL_CONNECTED; ii++) 
    {
      delay(100);
      Serial.print(".");
    }
    Serial.println("");
    
    Serial.print(ssid.c_str());
    Serial.print(" status = ");
    Serial.println(WiFi.status());
    
    Serial.print(ssid);
    bool connected = (WiFi.status() == WL_CONNECTED);
    Serial.println(connected ?  " connected." : " failed to connect.");
    configTime(gmtOffset.toInt(), 0, ntpServer);

    if (!connected) {
      if (state == 3) {
        BLEPrint("Failed to connect to " + ssid + "\n");
        state = 0;
        tryBLE = millis() + 5 * 1000;
      }
    }
    else {
      BLEPrint("Successfully connected to " + ssid + "\n");
      if (state == 3) {
        ipTime = 0;
      }
      state = 4;
    }
  }
}

void updateTime() 
{
  if(!getLocalTime(&timeinfo))
  {
     Serial.println("Failed to obtain time");
  }
  char timeString[50];
  strftime(timeString, sizeof(timeString), "%A, %B %d %Y %H:%M", &timeinfo);
  if (strcmp(timeString, _timeString) != 0)
  {
    Serial.println(timeString);
    strcpy(_timeString, timeString);
  }
}

void disconnectWifi()
{
  if (WiFi.status() == WL_CONNECTED)
  {
    WiFi.disconnect();
    for (int ii = 0; ii < 100 && WiFi.status() == WL_CONNECTED; ii++) {
      delay(100);
      Serial.print(".");
    }
    Serial.print(ssid);
    Serial.println(" disconnected.");
  }
}

int wifiNetworkCount = 0;

int scanWifi()
{
    wifiNetworkCount = WiFi.scanNetworks();
    
    if (wifiNetworkCount == 0) {
        BLEPrint("No Wifi networks found:\n");
    } 
    else {
      String s1 = String(wifiNetworkCount) + " Wifi networks found:\n";
      BLEPrint(s1);
        for (int i = 0; i < wifiNetworkCount; ++i) {
            String s2 = WiFi.SSID(i) + /* + " (" + WiFi.RSSI(i) + ") " + ((WiFi.encryptionType(i) == WIFI_AUTH_OPEN)?" ":"*") */ "\n";
            BLEPrint(s2);
            delay(10);
        }
    }
    return wifiNetworkCount;
}

class MyServerCallbacks: public BLEServerCallbacks 
{
    void onConnect(BLEServer* pServer) 
    {
      BLEConnected = true;
      tryBLE = millis() + 10 * 1000;
      Serial.println("BLE Connected");
   };

    void onDisconnect(BLEServer* pServer) 
    {
      BLEConnected = false;
      Serial.println("BLE Disconnected");
      if (state != 4) {
        state = 0;
        tryBLE = 0;
      }
   }
};

class MyCallbacks: public BLECharacteristicCallbacks 
{
    void onWrite(BLECharacteristic *pCharacteristic) 
    {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        
//        Serial.println("*********");
//        Serial.print("Received Value: ");
//        for (int i = 0; i < rxValue.length(); i++){
//          Serial.print(rxValue[i]);
//        }
//        Serial.println();

        String input = rxValue.c_str();
        input.replace("\n", "");
        
        if (state == 1) {
          bool found = false;
          for (int i = 0; i < wifiNetworkCount; i++) {
            String s1 = WiFi.SSID(i);
            Serial.println("|" + s1 + "|" + " " + "|" + input + "|");
            if (s1.equals(input)) {
              found = true;
              ssid = String(input.c_str());
              if (WiFi.encryptionType(i) != WIFI_AUTH_OPEN) {
                BLEPrint("Type in wifi network password:\n");
                state = 2;
                break;
              }
              else {
                password = String("");
                state = 3;
                setOneColor(0, {0x00, 0x00, 0x00});
                tryWifi = 0;
                writeInfo(ssid, password, gmtOffset);
              }
            }
          }
          if (!found) {
            BLEPrint(input + " not found.\n");
            state = 0;
            tryBLE = 0;
          }
        }
        else if (state == 2) {
          password = String(input.c_str());
          state = 3;
          setOneColor(0, {0x00, 0x00, 0x00});
          tryWifi = 0;
          writeInfo(ssid, password, gmtOffset);
        }
        else if (state == 4) {
          if (input.equals("Reset")) {
            writeInfo("", "", gmtOffset);
            state = 0;
            tryBLE = 0;
          }
        }
      }
    }
};

void BLEPrint(String input)
{
  if (BLEConnected) 
  {
    int length = input.length();
    if (length  > 0)
    {
      String message = "";
      int index = 0;
      while (length > 0) {
        int size = min(6, length);
        message = input.substring(index, index + size);
        pCharacteristic->setValue(message.c_str());
        pCharacteristic->notify();
        length -= size;
        index += size;
      }
    }
  }

  Serial.print(input.c_str());
}

void writeInfo(String name, String password, String gmtOffset)
{
  String input = name + "\t" + password + "\t" + gmtOffset;
  int ii = 0;
  for (ii = 0; ii < input.length(); ii++) {
    byte b = input[ii];
    //Serial.println(b);
    EEPROM.write(ii, b);
  }
  EEPROM.write(ii, 0);
  EEPROM.commit();
}

String readInfo()
{
  String output = "";
  for (int ii = 0; ii < EEPROM_SIZE; ii++) {
    byte b = EEPROM.read(ii);
    output += char(b);
    if (b == 0) break;
  } 
  return output;
}

String getTmz() {
    
    String tmz = "";

    // ip-api.com 
    // Free for non-commercial use. 
    // No API key required.
    // Limited to 45 requests per minute.
    WiFiClient client1;
    if (!client1.connect("ip-api.com", 80)) Serial.println("connection1 failed");    
    client1.print((String)"GET /json/ HTTP/1.1\r\n" + "Host: " + String("ip-api.com") + "\r\n" + "Connection: close\r\n\r\n");                
    delay(1000); 
    while (client1.available()) { 
      String line = client1.readStringUntil('\r'); 
      //Serial.print(line);
      int idx1 = line.indexOf("timezone");
      if (idx1 != -1) {
        int idx2 = line.indexOf(":", idx1);
        int idx3 = line.indexOf("\"", idx2) + 1;
        int idx4 = line.indexOf("\"", idx3);
        tmz = line.substring(idx3, idx4);
      }
    }
    client1.stop();

    if (tmz.length() == 0) {

      // ipinfo.io
      // Key required
      // Limited to 50,000 requests per month.
      WiFiClient client2;
      if (!client2.connect("ipinfo.io", 80)) Serial.println("connection2 failed");    
      client2.print((String)"GET / HTTP/1.1\r\n" + "Authorization: Bearer b0dd1aac416970" + "\r\n" + "Host: " + String("ipinfo.io") + "\r\n" + "Connection: close\r\n\r\n");                
      delay(1000); 
      while (client2.available()) { 
        String line = client2.readStringUntil('\r'); 
        //Serial.print(line);
        int idx1 = line.indexOf("timezone");
        if (idx1 != -1) {
          int idx2 = line.indexOf(":", idx1);
          int idx3 = line.indexOf("\"", idx2) + 1;
          int idx4 = line.indexOf("\"", idx3);
          tmz = line.substring(idx3, idx4);
        }
      }
      client2.stop();
    }

    return tmz;
}

String getGmtOffset(String tmz) {

    String gmtOffset = "";

    // timezondb.com
    // Free for non-commercial use. 
    // Key required.
    // Limited to one query per second.
    WiFiClient client1;
    String command = (String)"GET /v2.1/get-time-zone?key=HTYHALVJ6E5P&format=json&by=zone&zone=" + tmz + " HTTP/1.1\r\n" + "Host: " + String("api.timezonedb.com") + "\r\n" + "Connection: close\r\n\r\n";
    if (!client1.connect("api.timezonedb.com", 80)) Serial.println("connection1 failed");    
    client1.print(command);                
    delay(1000);   
    while (client1.available()) { 
      String line = client1.readStringUntil('\r'); 
      //Serial.print(line); 
      int idx1 = line.indexOf("gmtOffset");
      if (idx1 != -1) {
        int idx2 = line.indexOf(":", idx1) + 1;
        int idx3 = line.indexOf(",", idx2);
        gmtOffset = line.substring(idx2, idx3);
      }       
    }
    client1.stop();
      
    return gmtOffset;
}

void setGmtOffset() { 
   
  if (ipTime == 0 || millis() >= ipTime) {
    
    ipTime = millis() + 60 * 60 * 1000;

    connectWifi();
    delay(1000);
    
    String tmz = getTmz();
    if (tmz.length() > 0) {
      String offset = getGmtOffset(tmz);
      if (offset.length() > 0) {
        gmtOffset = offset;
        writeInfo(ssid, password, gmtOffset);
        configTime(gmtOffset.toInt(), 0, ntpServer);
        Serial.println(gmtOffset);
      }
    }

    disconnectWifi();   
  } 
}

void initDisplay()
{
  int now = millis();
  if (ledTime == 0 || now >= ledTime) { 
    
    ledTime = now + 10;
    
    RGB color = { 0x80, 0x80, 0x80 }; 
    if (state == 0) color = {0x80, 0x00, 0x00};  
    else if (state == 1) color = {0x00, 0x80, 0x00};  
    else if (state == 2) color = {0x00, 0x00, 0x80};
    
    setOneColor(ledIndex, color); 
     
    ledIndex += 1;
    if (ledIndex == 60) {
      ledIndex = 0;
    }
  }
}

void syncTime()
{
  int now = millis();
  if (tryWifi == 0 || now - tryWifi > 10 * 60 * 1000)
  {
    tryWifi = now;
    connectWifi();
    updateTime();
    disconnectWifi();
  }
}

void initClock()
{
  if (BLEConnected) {
    
   int now = millis();
   if (tryBLE == 0 || now >= tryBLE) {   
      tryBLE = now + 1 * 60 * 1000;    
      if (state == 1) {      
        state = 0;
      }     
      if (state == 0) {
        setOneColor(0, {0x00, 0x00, 0x00});
        int n = scanWifi();
        if (n > 0) {
          BLEPrint("Type in network name to select:\n"); 
          state = 1;
        }
      }     
      else if (state == 4) {
       BLEPrint("Currently connected to " + ssid + "\n"); 
       BLEPrint("Type 'Reset' to disconnect.\n"); 
      }      
    }
  }
}

void setup() 
{
  Serial.begin(115200);
  Serial.println("Hello");

  EEPROM.begin(EEPROM_SIZE);

  String info = readInfo();
  int index1 = info.indexOf('\t');
  int index2 = info.indexOf('\t', index1 + 1);
  ssid = info.substring(0, index1);
  password = info.substring(index1 + 1, index2);
  gmtOffset = info.substring(index2 + 1).toInt();

  Serial.println(info);

  connectWifi();
  updateTime();
  disconnectWifi();

#ifdef USE1306
  ui.setTargetFPS(60);
  ui.disableAllIndicators();
  ui.setIndicatorPosition(TOP);
  ui.setIndicatorDirection(LEFT_RIGHT);
  ui.setFrameAnimation(SLIDE_LEFT);
  ui.setFrames(frames, frameCount);
  ui.setOverlays(overlays, overlaysCount);
  ui.init();
#endif

  strip.Begin();
  strip.Show();

  BLEDevice::init(name);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->addDescriptor(new BLE2902());
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();
  pServer->getAdvertising()->start();  

 pinMode(LED_BUILTIN, OUTPUT);
 digitalWrite(LED_BUILTIN, LOW);
}

void loop() 
{
  int remainingTimeBudget = 1;

#ifdef USE1306
  remainingTimeBudget = ui.update();
#endif
 
  if (remainingTimeBudget > 0) 
  {
    initClock();

    setGmtOffset();

    if (state >= 3) {
      syncTime();
    }
    
    if (state <= 3) {
      initDisplay();
    }
    else {
      updateTime();
      updateClock();
    }
  
  }
}
 
