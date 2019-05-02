
#include <Wire.h>
#include <ESP8266WiFi.h>



#define distance_per_revolution 0.4
#define debug

const char* ssid     = "dsfafsda"; // Your SSID (Name of your WiFi)
const char* password = "asfdsdf"; //Your Wifi password

const char* host = "api.thingspeak.com";
String api_key = "asfdsaddsdaf"; // Your API Key provied by thingspeak

float uno_speed, uno_distance; //Variables for the BME280 Sensor
unsigned long lap_count=0;
unsigned long lap_time=0;
unsigned long lap_start=0;


bool int_fired=false;


#define interruptPin 5

void handleInterrupt()
{
  
  unsigned long this_lap_time=millis()-lap_start;
  if (this_lap_time<10)
    return;
  lap_time=this_lap_time;
  lap_count++;
  lap_start=millis();
  int_fired=true;
}

void setup() {


#ifdef debug
  Serial.begin(115200);
  delay(10);
  Serial.println("init");
#endif
  
  Connect_to_Wifi();

 pinMode(interruptPin, INPUT);
 attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, FALLING);



}

void loop() {
  delay(5000);

  // Uno has inactive for 6 hours and has finished his exercise routine  
  if ((millis()-lap_start)>1000*60*60*6)
      lap_count=0;
      uno_speed=0;
      uno_distance=0;
      
  if (int_fired)
  {     
    uno_speed=distance_per_revolution*1000 / (float)lap_time;
    uno_distance=(float)lap_count * distance_per_revolution;
    int_fired=false;
  
  }
  else 
  {  
    uno_speed=0;
  }
  Serial.println("Uno speed: " + String (uno_speed) + ", Uno distance: " + String(uno_distance)+ ", pa count: " + String (lap_count) + ", lap_time: " + String(lap_time));
  send_data();
    
}



void Connect_to_Wifi()
{

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  /* Explicitly set the ESP8266 to be a WiFi-client, otherwise, it by default,
     would try to act as both a client and an access-point and could cause
     network-issues with your other WiFi-devices on your WiFi-network. */
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void send_data()
{

  // Use WiFiClient class to create TCP connections
  WiFiClient client;

  const int httpPort = 80;

  if (!client.connect(host, httpPort)) {
#ifdef debug
    Serial.println("connection failed");
#endif
    return;
  }
  else
  {
    String data_to_send = api_key;
    data_to_send += "&field1=";
    data_to_send += String(uno_speed);
    data_to_send += "&field2=";
    data_to_send += String(uno_distance);
    data_to_send += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + api_key + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(data_to_send.length());
    client.print("\n\n");
    client.print(data_to_send);

    delay(1000);
  }

  client.stop();
}
