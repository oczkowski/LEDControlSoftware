# LED Control Software

## Part (1/3)

## Intro

This in a repo for a project split into 3 repositories.
Inside of this repository you will find the ESP controller software. Please bare with me, my C++ knowledge is minimal.
This project was built on react for training purposes.

## Requirements

-   An ESP8266 Controller board
-   Some soldering knowledge to solder up the LED's
-   A micro usb cable to upload the software to the ESP
-   Some Arduino development enviroment knowledge
    -   NTPClient by Fabrice Weinberg [LINK](https://github.com/arduino-libraries/NTPClient)
    -   WiFi (Should come with the development software) [LINK](https://www.arduino.cc/en/Reference/WiFi)
    -   ArduinoJson by Benoit Blanchon [LINK](https://arduinojson.org/?utm_source=meta&utm_medium=library.properties)
    -   FastLED by Daniel Garcia [LINK](https://github.com/FastLED/FastLED)
    -   Redis for Arduino by Ryan Joseph [LINK](https://github.com/electric-sheep-co/arduino-redis)

## Configuration

Before continuing make sure your arduino development enviroment is ready to work with the ESP8266 aka NodeMcu.
Withing the Arduino software navigate to File -> Preferences -> Additional Boards Manager URLs
and add a new link: `http://arduino.esp8266.com/stable/package_esp8266com_index.json`

Once this is done, please setup your development enviroment.

1. Open this repository inside of the Arduino software.
2. Install all Libraries outlined above in the requirements
3. Click on Tools and apply the following settings

```
Board: NodeMCU 1.0 (ESP-12E Module)
Upload Speed 115200
CPU Frequency: 80MHz
Port: PLEASE CHOOSE CORRECT COM PORT FOR THE ESP
```

Once the setup is done, let's prepare out software. You'd usually reconfigure this each time you flash a new ESP

1. Copy `config.h.example` and save it as `config.h`.
2. In the first section I would only recomment changing KEY_NAME, LED_TYPE and LOCATION, if you're not sure what you are doing.
3. In the NTP section the only thing you might want to adjust is the NTP_OFFSET as this adjust the time to the time zone you're in. You might also want to change NTP server if desired, that would depend on your geo location.
4. In the FastLED section you must make sure these settings are correct, NUM_LEDS to the correct number of LEDs on the strip, DATA_PIN 3 means D3 on the ESP depending on the board. BRIGHTNESS which is a value between 0 and 255.
5. Change the WIFI_SSID and WIFI_PASS to the WiFi name and password
6. For redis configuration it could be an internal network or an external server depending on your setup. Password is required.
7. The next FastLED section is pretty custom and it will depend on the type of LEDs you're using. The current setup is built for the RGBW Led's so it's set to CRGB. Please adjust it based on the FastLED configuration you want ot use.
8. maxCycles is really important on the ESP chip. During testing I found that the ESP will often freeze and won't reboot. To prevent that from happening I've introduced a system that will restart the ESP evey SOMETHING cycles. The default is 1000 meaning that the ESP will run the code 1000 times before restarting. This is a safe number and will vary on different boards.
9. updateTimeCycle is the number of cycles between each NTP server call. 18 is just about less than a second which helps to keep the time up to date. ^-^

## Flashing

Once you're happy with the setup Click on Sketch -> Verify/Compile and wait for the program to be uploaded to the ESP. This step should take about a minute.
Give the ESP a minute to load up and if everythings fine you should see every second LED on the strip light up RED.
This means that the ESP connected to Redis but doesn't have any data for the LED strip and needs to be registered in the dashboard.
For debugging please check the Serial Monitor: Tools -> Serial Monitor.
