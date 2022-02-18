#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "pitches.h"

String user = "user_mqtt";
String passwd = "password_mqtt";

const char* ssid = "ssid_home";
const char* password = "password_home";
const char* mqtt_server = "broker_IP";

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];

int value = 0; //sinal de heartbeat

bool prevBtn1 = 0; //estado anterior do botão
bool btn1 = 0; //estado atual do botão

bool prevBtn2 = 0; //estado anterior do botão
bool btn2 = 0; //estado atual do botão

bool prevBtn3 = 0; //estado anterior do botão
bool btn3 = 0; //estado atual do botão

bool prevBtn4 = 0; //estado anterior do botão
bool btn4 = 0; //estado atual do botão

bool led = 0; //estado do led

uint32_t prev_millis;
uint32_t ms_cnt = 0;

const char* zero_cstr = "0";
const char* one_cstr = "1";

int valor;

void setup_wifi() {

  delay(10);

  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Led buit-in mapeado no topico "user/ledhome"
  if (strcmp(topic,(user+"/ledhome").c_str())==0) {
    if ((char)payload[0] == '1') {
      digitalWrite(D4, LOW);
      led = 1;
    } else {
      digitalWrite(D4, HIGH);
      led = 0;
    }
  }

  if (strcmp(topic,(user+"/buzzer").c_str())==0) {
    if ((char)payload[0] == '1') {
      tone(D5, NOTE_FS6, 250); //125 ou 250
    }
    else if ((char)payload[0] == '2') {
      tone(D5, NOTE_C4, 250); //125 ou 250
    }
    else if ((char)payload[0] == '3') {
      tone(D5, NOTE_DS5, 250); //125 ou 250
    }
    else if ((char)payload[0] == '4') {
      tone(D5, NOTE_A7, 250); //125 ou 250
    }
    else {
      noTone(D5);
    }
  }
  
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    
    Serial.print("Attempting MQTT connection...");
    
    // Create a random client ID
    String clientId = user;
    clientId += String(random(0xffff), HEX);
    
    // Attempt to connect
    if (client.connect(clientId.c_str(), user.c_str(), passwd.c_str())) {
      
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish((user+"/homehello").c_str(), "hello world");

      client.subscribe((user+"/ledhome").c_str());

      client.subscribe((user+"/buzzer").c_str());
      
    } else {
      
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      
      // Wait 5 seconds before retrying
      delay(5000);
      
    }
  }
}

void setup() {
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(D5, OUTPUT);

  pinMode(D1, INPUT); //botão como input
  pinMode(D2, INPUT); //botão como input
  pinMode(D6, INPUT); //botão como input
  pinMode(D7, INPUT); //botão como input
  
  Serial.begin(115200);
  
  setup_wifi();
  
  client.setServer(mqtt_server, 80);
  client.setCallback(callback);
}

void loop() {

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  if(prev_millis!=millis()){
    prev_millis=millis();
    if(ms_cnt%100==0){
      
      btn1 = digitalRead(D1); //leitura do estado do botão
      if (!prevBtn1 and btn1){
        Serial.println("botão 1 pressionado (borda de subida)");
        client.publish((user+"/home1").c_str(), one_cstr); // botão virtual
      }
      prevBtn1 = btn1;

      btn2 = digitalRead(D2); //leitura do estado do botão
      if (!prevBtn2 and btn2){
        Serial.println("botão 2 pressionado (borda de subida)");
        client.publish((user+"/home2").c_str(), one_cstr); // botão virtual
      }
      prevBtn2 = btn2;

      btn3 = digitalRead(D6); //leitura do estado do botão
      if (!prevBtn3 and btn3){
        Serial.println("botão 3 pressionado (borda de subida)");
        client.publish((user+"/home3").c_str(), one_cstr); // botão virtual
      }
      prevBtn3 = btn3;

      btn4 = digitalRead(D7); //leitura do estado do botão
      if (!prevBtn4 and btn4){
        Serial.println("botão 4 pressionado (borda de subida)");
        client.publish((user+"/home4").c_str(), one_cstr); // botão virtual
      }
      prevBtn4 = btn4;
      
    }
    ms_cnt++;
  }

  unsigned long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    snprintf (msg, MSG_BUFFER_SIZE, "#%ld", value);
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish((user+"/homehello").c_str(), msg);

    client.publish((user+"/home1").c_str(), zero_cstr); // botão virtual
    client.publish((user+"/home2").c_str(), zero_cstr); // botão virtual
    client.publish((user+"/home3").c_str(), zero_cstr); // botão virtual
    client.publish((user+"/home4").c_str(), zero_cstr); // botão virtual
  }
}
