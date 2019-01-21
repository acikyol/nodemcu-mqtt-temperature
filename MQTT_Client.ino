#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "WIFI_NAME"
#define WLAN_PASS       "WIFI_PASS"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER       "io.adafruit.com"  // MQTT broker address
#define AIO_SERVERPORT   1883               // MQTT port - use 8883 for SSL
#define AIO_USERNAME     "username"         // MQTT username
#define AIO_END_USERNAME "username"         // adafruit username - if using adafruit io services
#define AIO_KEY          "password"         // MQTT password

/************ Global State (you don't need to change this!) ******************/

WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

Adafruit_MQTT_Publish sicaklikpub = Adafruit_MQTT_Publish(&mqtt, AIO_END_USERNAME "/feeds/sicaklik");

Adafruit_MQTT_Subscribe sicaklik = Adafruit_MQTT_Subscribe(&mqtt, AIO_END_USERNAME "/feeds/sicaklik");

/*************************** Sketch Code ************************************/

void MQTT_connect();

void setup() {
  Serial.begin(115200);
  delay(10);
  pinMode(A0,INPUT);

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); Serial.println(WiFi.localIP());

  // Setup MQTT subscription for onoff feed.
  mqtt.subscribe(&sicaklik);
}


void loop() {
  MQTT_connect();

//  Adafruit_MQTT_Subscribe *subscription;
//  while ((subscription = mqtt.readSubscription(5000))) {
//    if (subscription == &sicaklik) {
//      Serial.print(F("Sıcaklık Geldi : "));
//      Serial.println((char *)sicaklik.lastread);
//    }
//  }

// Publish temprature data to mqtt broker every 2 second

  // Calculating to celsius value using LM35 Temprature sensore voltage value
  int analogValue = analogRead(A0);
  float millivolts = (analogValue/1024.0) * 3300; //3300 is the voltage provided by NodeMCU
  float celsius = millivolts/10;
  char chrCelsius[8];
  sprintf(chrCelsius, "%2.2f", celsius);
  
  Serial.print(F("\nSending temprature val "));
  Serial.print("...");
  if (! sicaklikpub.publish(chrCelsius)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  delay(2000);

  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  */
}

void MQTT_connect() {
  int8_t ret;

  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
