#include <Redis.h>

#include "FastLED.h"
#include "FastLED_RGBW.h"

#include <NTPClient.h>
#include <WiFiUdp.h>

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

// Importing config
#include "config.h"

void setup() {
  // Setting up
  Serial.begin(115200);
  Serial.println();
  Serial.println("Starting up...");
  
  // Setting up WiFi Connection
  Serial.print("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  
  while(WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
  }
  
  Serial.println();
  Serial.print("Assigned IP Address: ");
  Serial.println(WiFi.localIP());
  delay(500);

  // Setup NTP Client
  timeClient.begin();
  timeClient.update();
  
  // Setting up Redis
  if (redisConnection.connect(REDIS_ADDR, REDIS_PORT))
  {
      Serial.println("Connected to the Redis server!");
  } else {
      Serial.println("Failed to connect to the Redis server! Restarting in 5 seconds...");
      delay(5000);
      ESP.restart();
  }

  // Redis Authentication
  gRedis = new Redis (redisConnection);
  auto connRet = gRedis->authenticate(REDIS_PASS);
  if (connRet == RedisSuccess)
  {
      Serial.println("Successfull Redis Authentication!");
  } 
  else 
  {
      Serial.printf("Failed to authenticate to the Redis server! Errno: %d\n", (int)connRet, ". Restarting in 5 seconds...");
      delay(5000);
      ESP.restart();
  }
  Serial.println("====================> Configuration <====================");
  
  // Registering device in redis
  if(!gRedis->exists(KEY_NAME_CONFIG)){
    DynamicJsonDocument doc(1024);
    
    doc["MAC_ADDRESS"] = WiFi.macAddress(mac);
    doc["LED_TYPE"] = LED_TYPE;
    doc["LOCATION"] = LOCATION;
    doc["NUM_LEDS"] = NUM_LEDS;
    doc["LOCAL_IP"] = WiFi.localIP().toString();
    doc["NEW"] = true;
    
    char config_string[256];

    serializeJson(doc, config_string);
    Serial.println("Registered as a new device, please complete setup in the dashboard.");
    gRedis->set(KEY_NAME_CONFIG, config_string);
  } else {
    Serial.println("Device already registered!");
    Serial.println("Current Configuration: " + gRedis->get(KEY_NAME_CONFIG));
  }
  
  // FastLED setup
  FastLED.addLeds<WS2812B, DATA_PIN, RGB>(ledsRGB, getRGBWsize(NUM_LEDS));
  FastLED.setBrightness(brightness);
  FastLED.show();
  
}

void loop(){
  // Update NTP time on each loop
  timeClient.update();
  char time_and_date[16];
  String timeadndate = timeClient.getFormattedTime();
  timeadndate.toCharArray(time_and_date, 16);
  
  // Updating "last_connected";
  gRedis->set(KEY_NAME_LC, time_and_date);
  
  // Check if there is DATA for LED's
  if(!gRedis->exists(KEY_NAME_DATA)){
    // No data for LED's waiting for registration
    Serial.println("No data for leds... Waiting 3 second...");
    colorFillHalf(CRGB::Red);
    delay(3000);
  } else {
    Serial.println("Found data for LED's... Checking again in 5 seconds....");

    DynamicJsonDocument lightsConfig(1024);
    
    delay(5000);
  }

  
//  colorFill(CRGB::Red);
//  colorFill(CRGB::Green);
//  colorFill(CRGB::Blue);
//  fillWhite();
//  rainbowLoop();
}




void colorFill(CRGB c){
  for(int i = 0; i < NUM_LEDS; i++){
    leds[i] = c;
    FastLED.show();
  }
  delay(1000);
}

void colorFillHalf(CRGB c){
  for(int i = 0; i < NUM_LEDS; i += 2){
    leds[i] = c;
    FastLED.show();
  }
}

void applyLEDdata(CRGB c){
  for(int i = 0; i < NUM_LEDS; i += 2){
    leds[i] = c;
    FastLED.show();
  }
}

void fillWhite(){
  for(int i = 0; i < NUM_LEDS; i++){
    leds[i] = CRGBW(0, 0, 0, 255);
    FastLED.show();
    delay(50);
  }
  delay(500);
}

void rainbow(){
  static uint8_t hue;

  for(int i = 0; i < NUM_LEDS; i++){
    leds[i] = CHSV((i * 256 / NUM_LEDS) + hue, 255, 255);
  }
  FastLED.show();
  hue++;
}

void rainbowLoop(){
  long millisIn = millis();
  long loopTime = 5000; // 5 seconds

  while(millis() < millisIn + loopTime){
    rainbow();
    delay(5);
  }
}