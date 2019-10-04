/* ESP8266: DHT11 + Elasticsearch
 * Author: Paula Mihalcea
 */

#include <WiFi.h>  // Library for WiFi connection
#include <HTTPClient.h>  // RESTful API
#include <NTPClient.h>  // Needed for the timestamp
#include <WiFiUdp.h>  // Same, for timestamp
#include "DHT.h"  // Library for the DHT 11 sensor

// WiFi settings
const char *ssid = "La Torre_Fastweb";  // WiFi SSID
const char *password = "6j0rdmbe";  // WiFi password

// NTP server settings
const long utcOffset = 3600;  // UTC time offset in seconds (UTC+01:00 for Italy)
WiFiUDP ntpUDP;  // Creation of a WiFiUDP object
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", utcOffset);  // NTPClient configuration

// Server settings
String serverIP = "192.168.8.100";  // Server IP address
String serverPort = "9200";  // Server port
String indexName = "ambient_variables";  // Elasticsearch index name
String docType = "_doc";  // Elasticsearch document type
String MAC = WiFi.macAddress();  // Device MAC (needed to identify entries in the database)

// DHT 11 sensor
#define DHTTYPE DHT11  // DHT 11 temperature/humidity sensor type
const int DHTPin = 4;  // DHT sensor pin
DHT dht(DHTPin, DHTTYPE);  // Initialize DHT sensor

// DHT temporary variables
static char temperature[7];
static char humidity[7];

// POST settings
int x = 30000;  // Time period for POST requests, in milliseconds; must NOT be less than 10000-15000 ms, the DHT sensor is very slow
 
void setup() {
  Serial.begin(115200);  // 115200 baud serial monitor, useful for debugging
  pinMode (LED_BUILTIN, OUTPUT);  // LED setup
  pinMode(DHTPin, INPUT);  // DHT setup
  delay(1000);

  dht.begin();

  // WiFi connection
  WiFi.begin(ssid, password);  // Connection to the WiFi router
  Serial.println("");
  Serial.print("Connecting to ");
  Serial.print(ssid);
  
  while (WiFi.status() != WL_CONNECTED) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(500);
    digitalWrite(LED_BUILTIN, HIGH);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("Connection successful. IP: ");
  Serial.println(WiFi.localIP());
  Serial.println("");

  timeClient.begin();
}
 
void loop() {
  while(!timeClient.update()) {  // Update the time from the NTP server
    timeClient.forceUpdate();
  }

  String date = timeClient.getFormattedDate();  // Get current time from NTP server
  
  // DHT sensor readings
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);
  
  float temp = dht.readTemperature();  // DHT temperature (°C)
  float hum = dht.readHumidity();  // DHT humidity (%)

  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);

  if (isnan(temp) || isnan(hum)) {  // Check if any reads failed
    Serial.println("Failed to read from DHT sensor.");
    strcpy(temperature, "000000");
    strcpy(humidity, "000000");
  }
  else {    
    Serial.print("Temperature: ");  // Serial prints for debugging
    Serial.print(temp);
    Serial.print("°C");
    Serial.print("\t Humidity: ");
    Serial.print(hum);
    Serial.println("%");
  }

  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);

  // Server connection and POST request
  HTTPClient http;  // Creation of a new HTTPClient object

  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);

  String httpAddress = "http://" + serverIP + ":" + serverPort + "/" + indexName + "/" + docType;  // HTTP address to be used in the POST request
  http.begin(httpAddress);  // Server connection
  http.addHeader("Content-Type", "application/json");  // Specify content-type header; application/json for Elasticsearch

  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);

  // Data to be sent to the server
  String postData = "{";
  postData.concat("\"date\"");
  postData.concat(":");
  postData.concat("\"");
  postData.concat(date);
  postData.concat("\"");
  postData.concat(",");
  postData.concat("\"temp\"");
  postData.concat(":");
  postData.concat("\"");
  postData.concat(temp);
  postData.concat("\"");
  postData.concat(",");
  postData.concat("\"hum\"");
  postData.concat(":");
  postData.concat("\"");
  postData.concat(hum);
  postData.concat("\"");
  postData.concat(",");
  postData.concat("\"");
  postData.concat("MAC");
  postData.concat("\"");
  postData.concat(":");
  postData.concat("\"");
  postData.concat(MAC);
  postData.concat("\"");
  postData.concat("}");

  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);
  
  int postReturn = http.POST(postData);  // Send the POST request
  String postPayload = http.getString();  // Get the response payload

  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);
  
  if (postReturn == 201)    // More serial prints for debugging
    Serial.println("POST request to server successful, data has been saved to database.");
  else {
    Serial.print("POST request to ");
    Serial.print(serverIP);
    Serial.print(" unsuccessful (error ");
    Serial.print(postReturn);
    Serial.println(").");
  }

  Serial.println("");

  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);
 
  http.end();  // Close connection

  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);

  int i = 0;
  while(i < x) {  // Wait for the next cycle while blinking LED
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    i += 2000;
  }
}
