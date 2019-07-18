/***************************************************
  Adafruit MQTT Library CC3000 Example

  Designed specifically to work with the Adafruit WiFi products:
  ----> https://www.adafruit.com/products/1469

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Limor Fried/Ladyada for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/


#include <Adafruit_SleepyDog.h>
#include <Adafruit_CC3000.h>
#include <SPI.h>
#include "utility/debug.h"
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_CC3000.h"
/*-----------------------------------------*/

#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define ONE_WIRE_BUS_1 A0
#define ONE_WIRE_BUS_2 A1

#define DHTPIN            A3  
#define DHTTYPE           DHT11  
DHT_Unified dht(DHTPIN, DHTTYPE);


/*************************** CC3000 Pins ***********************************/

#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
#define ADAFRUIT_CC3000_VBAT  5  // VBAT & CS can be any digital pins.
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11

/************************* WiFi Access Point *********************************/

///Configuracion Datos de Red
//#define WiFi_Home
#define WiFi_Lab

#ifdef WiF_Home
#define WLAN_SSID       "SSID"  
#define WLAN_PASS       "PASSWORD"
#define WLAN_SECURITY   WLAN_SEC_WPA2  // Can be: WLAN_SEC_UNSEC, WLAN_SEC_WEP,
#endif                                 //         WLAN_SEC_WPA or WLAN_SEC_WPA2

#ifdef WiFi_Lab
#define WLAN_SSID       "SSID"  
#define WLAN_PASS       "PASSWORD"
#define WLAN_SECURITY   WLAN_SEC_WPA2  
#endif                                 


/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "User"        //Change
#define AIO_KEY         "Key"         //Change

/************ Global State (you don't need to change this!) ******************/

// Setup the main CC3000 class, just like a normal CC3000 sketch.
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT);

// Store the MQTT server, username, and password in flash memory.
// This is required for using the Adafruit MQTT library.
const char MQTT_SERVER[] PROGMEM    = AIO_SERVER;
const char MQTT_USERNAME[] PROGMEM  = AIO_USERNAME;
const char MQTT_PASSWORD[] PROGMEM  = AIO_KEY;

// Setup the CC3000 MQTT class by passing in the CC3000 class and MQTT server and login details.
Adafruit_MQTT_CC3000 mqtt(&cc3000, MQTT_SERVER, AIO_SERVERPORT, MQTT_USERNAME, MQTT_PASSWORD);

// You don't need to change anything below this line!
#define halt(s) { Serial.println(F( s )); while(1);  }

// CC3000connect is a helper function that sets up the CC3000 and connects to
// the WiFi network. See the cc3000helper.cpp tab above for the source!
boolean CC3000connect(const char* wlan_ssid, const char* wlan_pass, uint8_t wlan_security);

/****************************** Feeds ***************************************/

// Setup a feed called 'photocell' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
const char TEMP_IN_FEED[] PROGMEM = AIO_USERNAME "/feeds/temp_in";
Adafruit_MQTT_Publish temp_in = Adafruit_MQTT_Publish(&mqtt, TEMP_IN_FEED);

const char TEMP_OUT_FEED[] PROGMEM = AIO_USERNAME "/feeds/temp_out";
Adafruit_MQTT_Publish temp_out = Adafruit_MQTT_Publish(&mqtt, TEMP_OUT_FEED);

const char TEMP_ENV_FEED[] PROGMEM = AIO_USERNAME "/feeds/temp_env";
Adafruit_MQTT_Publish temp_env = Adafruit_MQTT_Publish(&mqtt, TEMP_ENV_FEED);

const char HUM_FEED[] PROGMEM = AIO_USERNAME "/feeds/hum";
Adafruit_MQTT_Publish hum = Adafruit_MQTT_Publish(&mqtt, HUM_FEED);


/*************************** Sketch Code ************************************/
OneWire oneWire_in(ONE_WIRE_BUS_1);
OneWire oneWire_out(ONE_WIRE_BUS_2);

DallasTemperature sensor_in(&oneWire_in);
DallasTemperature sensor_out(&oneWire_out);

//uint32_t delayMS;

void setup() {
  delay(3000);
  Serial.begin(115200);
  sensor_in.begin();
  sensor_out.begin();
  dht.begin();

   sensor_t sensor;
  dht.temperature().getSensor(&sensor);

  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);

  Serial.println(F("Adafruit MQTT demo"));

  Serial.print(F("Free RAM: ")); Serial.println(getFreeRam(), DEC);

  // Initialise the CC3000 module
  Serial.print(F("\nInit the CC3000..."));
  if (!cc3000.begin())
      halt("Failed");

 
  while (! CC3000connect(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    Serial.println(F("Retrying WiFi"));
    delay(1000);
  
  }
    
   Watchdog.enable(8000);
}


void loop() {
  // Make sure to reset watchdog every loop iteration!
  Watchdog.reset();
 

  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // Now we can publish stuff!
sensors_event_t event;  
  
 dht.humidity().getEvent(&event);
 if (! hum.publish(event.relative_humidity)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
 dht.temperature().getEvent(&event);
  if (! temp_env.publish(event.temperature)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  
  sensor_in.requestTemperatures();

  if (! temp_in.publish(sensor_in.getTempCByIndex(0))) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
 sensor_out.requestTemperatures();
  if (! temp_out.publish(sensor_out.getTempCByIndex(0))) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }

  // ping the server to keep the mqtt connection alive
  if(! mqtt.ping()) {
    Serial.println(F("MQTT Ping failed."));
  }
delay(5000);
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       if (ret < 0)
            CC3000connect(WLAN_SSID, WLAN_PASS, WLAN_SECURITY);  // y0w, lets connect to wifi again
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
  }
  Serial.println("MQTT Connected!");
}
