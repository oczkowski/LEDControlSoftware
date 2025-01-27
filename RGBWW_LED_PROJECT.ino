#include <Redis.h>

#include "FastLED.h"
#include "FastLED_RGBW.h"

#include <NTPClient.h>
#include <WiFiUdp.h>

#include <ArduinoJson.h>
#include <ESP8266WiFi.h>

// Importing config
#include "config.h"

// Cycle count for restart purposes
int cycles = 0;

// JSON Object for LED's data
DynamicJsonDocument ledsData(20480); // This can handle up to 200 LEDs
//NTP timestamp string
String formattedDate;

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
  timeClient.setTimeOffset(0); // Time zone GMT 0
  
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
    
    doc["MAC_ADDRESS"] = WiFi.macAddress();
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
//  FastLED.show(); Show only when updating led's to prevent LEDs from flashing on restart
  
}

void loop(){
  // Update time every "updateTimeCycle" number of cycles
  if((cycles % updateTimeCycle) == 0){
    // Update NTP time on each loop
    timeClient.update();
    formattedDate = timeClient.getEpochTime();
  
    char timeStamp[12];
    formattedDate.toCharArray(timeStamp, 12);
    
    // Updating "last_connected";
    gRedis->set(KEY_NAME_LC, timeStamp);

    Serial.println("Updating time: "+formattedDate);
  }
  
  // Check if there is DATA for LED's
  if(!gRedis->exists(KEY_NAME_DATA)){
    // No data for LED's waiting for registration
    Serial.println("No data for leds...");
    colorFillHalf(CRGB::Red);
  } else {
    Serial.println("["+String(cycles)+"] Found data for LED's.");
    
    deserializeJson(ledsData, gRedis->get(KEY_NAME_DATA));
    
    int mode = ledsData["mode"].as<int>();
    switch(mode){
      case 0:
        FillLEDs(ledsData);
        break;
      case 1:
        rainbow();
        break;
      case 2:
        rainbowLoop(ledsData);
        break;
      default:
        Serial.println("Configuration exist but doesn't select any of the available modes.");
        break;
    }
  }
  // Restart cycle
  cycles++;
  if(cycles > maxCycles){
    // When restarting, set the last connected to 0
    char * zero = "0";
    gRedis->set(KEY_NAME_LC, zero);
    // Restart
    Serial.println("Max Cycle count reached. Restarting in 1 second...");
    delay(1000);
    ESP.restart();
  }
}

void FillLEDs(DynamicJsonDocument config){
  // Processing data
  for(int i = 0; i < NUM_LEDS; i++){
    if(config["data"][i]){ // Does the config entry exist?
      leds[i] = CRGBW(config["data"][i]["r"], config["data"][i]["g"], config["data"][i]["b"], config["data"][i]["w"]);
    } 
  }
  // Once the configuration has been done, show LED's from the new array.
  FastLED.show();
}

void wipeLEDs(){
  for(int i = 0; i < NUM_LEDS; i++){
    leds[i] = CRGBW(0, 0, 0, 0);
    FastLED.show();
  }
}

void rainbow(){
  static uint8_t hue;

  for(int i = 0; i < NUM_LEDS; i++){
    leds[i] = CHSV((i * 256 / NUM_LEDS) + hue, 255, 255);
  }
  FastLED.show();
  hue++;
}

void rainbowLoop(DynamicJsonDocument config){
  long millisIn = millis();
  long loopTime = config["modeConfig"]["looptime"]; // 5 seconds

  while(millis() < millisIn + loopTime){
    rainbow();
    delay(5);
  }
}

void colorFillHalf(CRGB c){
  for(int i = 0; i < NUM_LEDS; i += 2){
    leds[i] = c;
    FastLED.show();
  }
}
