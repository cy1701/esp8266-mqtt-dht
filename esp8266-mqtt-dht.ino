#include <DHT.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <Base64.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include<stdlib.h>
#define wifi_ssid "YOUR WIFI SSID"
#define wifi_password "WIFI PASSWORD"

#define mqtt_server "YOUR_MQTT_SERVER_HOST"
#define mqtt_user "your_username"
#define mqtt_password "your_password"

#define humidity_topic "sensor/humidity"
#define temperature_topic "sensor/temperature"
//#define DHTTYPE DHT11   // DHT 11
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)
#define DHTPIN  2
WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE, 11); // 11 works fine for ESP8266
  
void setup() {
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);


  // Start sensor
  dht.begin(); 

}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    // If you do not want to use a username and password, change next line to
    // if (client.connect("ESP8266Client")) {
    if (client.connect("ESP8266Client", mqtt_user, mqtt_password)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

bool checkBound(float newValue, float prevValue, float maxDiff) {
  return newValue < prevValue - maxDiff || newValue > prevValue + maxDiff;
}

long lastMsg = 0;
float temp = 0.0;
float hum = 0.0;
float diff = 1.0;
    char humchar[10];
    char tempchar[10];

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;

    float  newTemp = dht.readTemperature(true);
    float newHum = dht.readHumidity();

    
    if (checkBound(newTemp, temp, diff)) {
      temp = newTemp;
      Serial.print("New temperature:");
      Serial.println(String(temp).c_str());
      dtostrf(temp,3, 1, tempchar);
      client.publish(temperature_topic, tempchar);
    }

    if (checkBound(newHum, hum, diff)) {
      hum = newHum;

      Serial.print("New humidity:");
      Serial.println(String(hum).c_str());
      dtostrf(hum,3, 1, humchar);
      client.publish(humidity_topic, humchar);
    }
  }
}
