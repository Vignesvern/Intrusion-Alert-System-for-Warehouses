#include <WiFi.h>
#include <PubSubClient.h>

//// WiFi
//const char *ssid = "SSN"; // Enter your Wi-Fi name
//const char *password = "Ssn1!Som2@Sase3#";  // Enter Wi-Fi password
//
const char *ssid = "Anonymous"; // Enter your Wi-Fi name
const char *password = "abcd4321";  // Enter Wi-Fi password

//
//const char *ssid = "Error"; // Enter your Wi-Fi name
//const char *password = "hello1234";  // Enter Wi-Fi password

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic = "emqx/esp32";
const char *mqtt_username = "emqx";
const char *mqtt_password = "public";
const int mqtt_port = 1883;

const int PIR_SENSOR_OUTPUT_PIN = 25; 
const int LED_RED = 12; 
const int LED_BLUE = 13;
const int LED_GREEN = 14; 

const int BUZZER_OUTPUT_PIN = 26; 

int playing =0;
int warm_up;
int check_status=1;

WiFiClient espClient;
PubSubClient client(espClient);

void tone(byte pin, int freq){
  ledcSetup(0, 2000, 8); // setup beeper
  ledcAttachPin(pin, 0); // attach beeper
  ledcWriteTone(0, freq); // play tone
  playing = pin; // store pin
}

void setup() {
  Serial.begin(115200); 
  WiFi.begin(ssid, password);
  
  pinMode(PIR_SENSOR_OUTPUT_PIN, INPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(BUZZER_OUTPUT_PIN, OUTPUT);
  
  Serial.println("Waiting For Power On Warm Up");
  delay(2000); /* Power On Warm Up Delay */
  Serial.println("Ready!");

  tone(playing, 0);

  while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Connecting to WiFi..");
    }
    Serial.println("Connected to the Wi-Fi network");
    //connecting to a mqtt broker
    client.setServer(mqtt_broker, mqtt_port);
    client.setCallback(callback);
    while (!client.connected()) { 
        String client_id = "esp32-client-";
        client_id += String(WiFi.macAddress());
        Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
        if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
            Serial.println("Public EMQX MQTT broker connected");
        } else {
            Serial.print("failed with state ");
            Serial.print(client.state());
            delay(2000);
        }
    }
}

void callback(char *topic, byte *payload, unsigned int length) {
    Serial.print("Message arrived in topic: ");
    Serial.println(topic);
    Serial.print("Message:");
    for (int i = 0; i < length; i++) {
        Serial.print((char) payload[i]);
    }
     
    if(payload[0] == 'S'){
      digitalWrite(LED_BLUE,LOW); 
      digitalWrite(LED_RED,LOW);
      tone(BUZZER_OUTPUT_PIN, 0);
      digitalWrite(LED_GREEN,HIGH);
      check_status=0;
      //delay(4000);
    }
    
    if(payload[0] == 'C'){
      digitalWrite(LED_BLUE,HIGH);  
      //tone(BUZZER_OUTPUT_PIN, 0);
      digitalWrite(LED_GREEN,LOW);  
      check_status=1;
      //delay(4000);
    }

    Serial.println();
    Serial.println("-----------------------");
}

void loop() {
  int sensor_output;
  sensor_output = digitalRead(PIR_SENSOR_OUTPUT_PIN);
  if( sensor_output == LOW )
  {
    if(check_status==1){
      digitalWrite(LED_BLUE,HIGH);
    }
    if( warm_up == 1 )
     {
      Serial.print("Warming Up\n\n");
      warm_up = 0;
      delay(2000);
    }
    if(check_status==0){
      Serial.print("No object in sight\n\n");
      digitalWrite(LED_RED,LOW);
      tone(BUZZER_OUTPUT_PIN, 0);
      delay(1000);
    }
  }
  else
  {
    if(check_status==1){
     Serial.print("Object detected\n\n");   
     warm_up = 1;
     client.publish(topic, "Object Detected");
     tone(BUZZER_OUTPUT_PIN, 1000); 
     digitalWrite(LED_RED,HIGH);
     client.subscribe(topic);
     delay(1000); 
    }
  } 
  client.loop();
}
