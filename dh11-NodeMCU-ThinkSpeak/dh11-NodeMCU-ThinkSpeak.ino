#include <DHT.h>
#include <ESP8266WiFi.h>

//Stuff for DH Module
#define DHTPIN 14     // what digital pin we're connected to  D5 = 14;
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);

//stuff for Thingspeak
const char* myWriteAPIKey = "YOUR-THINKSPEAK-API-KEY";
const char* thinkSpeakHost = "api.thingspeak.com";

//Wifistuff
const char* ssid     = "YOUR-WIFI-SSID";
const char* password = "YOUR-WIFI-PASSWORD";



void setup() {
  Serial.begin(115200);
  //Connecting to wifi
  WiFi.begin(ssid, password);

  //Loop till the wifi connection is established
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Wifi Connecting...");
  }

  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("DHT data is being fetched now...");
  //DH module about to begin
  dht.begin();
  // Wait a few seconds between measurements.
  delay(2000);
}

void loop() {
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(t);
  Serial.print(" *C ");
  Serial.print(f);
  Serial.print(" *F\t");
  Serial.print("Heat index: ");
  Serial.print(hic);
  Serial.print(" *C ");
  Serial.print(hif);
  Serial.println(" *F");

  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(thinkSpeakHost, httpPort)) {
    Serial.println("connection failed");
    return;
  }

  // We now create a URI for the http GET request
  String url = "/update?";
    url+="api_key=";
    url+=myWriteAPIKey;
    url+="&field1=";
    url+=t;
    url+="&field2=";
    url+=h;

  Serial.print("Requesting URL: ");
  Serial.println(url);

  // This will send the GET request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + thinkSpeakHost + "\r\n" + 
               "Connection: close\r\n\r\n");
  unsigned long timeout = millis();
  while (client.available() == 0) {
    if (millis() - timeout > 5000) {
      Serial.println(">>> Client Timeout !");
      client.stop();
      return;
    }
  }
  
  // Read all the lines of the reply from server and print them to Serial
  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();
  Serial.println("closing connection");

  // ThingSpeak will only accept updates every 15 seconds. 
  // We are gonna set 20 seconds timeout between each request
  delay(20000);

}
