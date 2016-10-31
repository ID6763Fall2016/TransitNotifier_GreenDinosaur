#define ENABLE_DEBUG           1
#define ENABLE_ESP8266_DEBUG   0

#include <Adafruit_DotStar.h>
// Because conditional #includes don't work w/Arduino sketches...
#include <SPI.h>         // COMMENT OUT THIS LINE FOR GEMMA OR TRINKET
//#include <avr/power.h> // ENABLE THIS LINE FOR GEMMA OR TRINKET

#define NUMPIXELS 13 // Number of LEDs in strip

// Here's how to control the LEDs from any two pins:
//#define DATAPIN    14
//#define CLOCKPIN   12
#define DATAPIN    13
#define CLOCKPIN   14
Adafruit_DotStar strip = Adafruit_DotStar(
                           NUMPIXELS, DATAPIN, CLOCKPIN, DOTSTAR_BRG);
// The last parameter is optional -- this is the color data order of the
// DotStar strip, which has changed over time in different production runs.
// Your code just uses R,G,B colors, the library then reassigns as needed.
// Default is DOTSTAR_BRG, so change this if you have an earlier strip.

// Hardware SPI is a little faster, but must be wired to specific pins
// (Arduino Uno = pin 11 for data, 13 for clock, other boards are different).
//Adafruit_DotStar strip = Adafruit_DotStar(NUMPIXELS, DOTSTAR_BRG);

#define LED   0

#define YELLOW_COLOR  0xFFFF00
//#define YELLOW_COLOR  0xAAAA00  // lower intensity yellow

#include <Arduino.h>
#include <ArduinoJson.h>

#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>

#include <ESP8266HTTPClient.h>

#include "keys.h" // keys.h includes user definitions for WIFI_SSID and WIFI_KEY

#define SERVER_ADDRESS  "X.X.X.X:3000"    // Node.js server IP:Port


ESP8266WiFiMulti WiFiMulti;

void setup() {

///////// DotStar
  pinMode(LED, OUTPUT);

#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000L)
  clock_prescale_set(clock_div_1); // Enable 16 MHz on Trinket
#endif

  strip.begin(); // Initialize pins for output
  strip.show();  // Turn all LEDs off ASAP


///////// ESP8266

  Serial.begin(115200);

#if ENABLE_ESP8266_DEBUG
  // enable diagnostic output from WiFi libraries
  // and enable output from printf()
  Serial.setDebugOutput(true);
#endif

  Serial.println();
  Serial.println();
  Serial.println();

  for (uint8_t t = 4; t > 0; t--) {
    Serial.printf("[SETUP] WAIT %d...\n", t);
    Serial.flush();
    delay(1000);
  }

  /* Connect to SSID with WPA */
  WiFiMulti.addAP(WIFI_SSID_1, WIFI_KEY_1);
  WiFiMulti.addAP(WIFI_SSID_2, WIFI_KEY_2);

  /* Show ESP8266 Client MAC address */
  String clientMac = "";

  unsigned char mac[6];
  WiFi.macAddress(mac);
  clientMac += macToStr(mac);

  Serial.println("Client MAC: " + clientMac);

}

void loop() {

  // wait for WiFi connection
  if ((WiFiMulti.run() == WL_CONNECTED)) {

#if ENABLE_DEBUG
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
#endif

    String payload = getHTTP("http://" + SERVER_ADDRESS + "/trolley_to_HUB_stop_Klaus", "");
    Serial.print(payload + " minutes.");

    int minutes = payload.toInt();
    setPixelTimeline(minutes, YELLOW_COLOR);

  }
  else {
    Serial.printf("No WiFi");
  }

//  delay(5000);
  delay(500);
}


/* convert MAC address to string */
String macToStr(const uint8_t* mac)
{
  String result;
  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);
    if (i < 5)
      result += ':';
  }
  return result;
}



/* GET the payload of an HTTP response */
String getHTTP(String url, String SHA_fingerprint) {

  String payload = "ERROR";

  HTTPClient http;

#if ENABLE_DEBUG
  Serial.print("[HTTP] begin...\n");
#endif

  if (SHA_fingerprint.length() > 0) {
    http.begin(url, SHA_fingerprint); //HTTPS
  }
  else {
    http.begin(url); //HTTP
  }


#if ENABLE_DEBUG
  Serial.print("[HTTP] GET...\n");
#endif

  // start connection and send HTTP header
  int httpCode = http.GET();

  // httpCode will be negative on error
  if (httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
#if ENABLE_DEBUG
    Serial.printf("[HTTP] GET... code: %d\n", httpCode);
#endif

    // file found at server
    if (httpCode == HTTP_CODE_OK) {
      payload = http.getString();
#if ENABLE_DEBUG
      Serial.println(payload);
#endif
    }
  }
  else {
    Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
  }

  http.end();

  return payload;

}


void setPixelTimeline(int minutes, int color) {

  // first LED always on, it represents the "> 10 minutes, so you'd better walk" case
  strip.setPixelColor(0, color);

  if (minutes < 10) {
    for (int i = 1; i < 10 - minutes; ++i) {
      strip.setPixelColor(i, color); // 'On' pixels
    }
    for (int i = 10 - minutes; i <= 10; ++i) {
      strip.setPixelColor(i, 0x000000); // 'Off' all remaining pixels
    }

  }
  else {
    for (int i = 1; i <= 10; ++i) {
      strip.setPixelColor(i, 0x000000); // 'Off' all remaining pixels
    }
  }

  strip.show();
}
