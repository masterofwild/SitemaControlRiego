//Área de include
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <iostream>
#include <string>

//Área de definición de dht
#define DHTPIN 4 
#define DHTTYPE DHT11

//Área de constantes
const char* ssid = "Totalplay-11A5";
const char* pass = "11A58B7DQT7zMuTr";
//const char* ssid = "INFINITUMA7A1_2.4";
//const char* pass = "5n2ZZusgAU";
const char* brokerUser = "cesarz";
const char* brokerPass = "123";
const char* broker = "192.168.100.31";
const char* outTopic ="cesarz/planta"; 
const char* inTopic  ="cesarz/esp32";
const int   relevador =  32;
const char* cadena = "";


//Área de inicialización
WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);
char messages[50];
String humedad = "";
String mensaje = "";

void setupWifi(){
  delay(100);
  Serial.print("\nConnecting to...");
  Serial.println(ssid);

  WiFi.begin(ssid, pass);

  dht.begin();

  while(WiFi.status() != WL_CONNECTED){
    delay(100);
    Serial.print("-"); 
    
  }

  Serial.print("\nConnected to...");
  Serial.println(ssid);
}

  void reconnect(){
    while(!client.connected()){
      Serial.print("\nConnecting to...");
      Serial.println(broker);
      if(client.connect("cesarz", brokerUser, brokerPass)){
        Serial.print("\nConnected to...");
        Serial.println(broker);
        client.subscribe(inTopic);
      } else {
        Serial.println("\nTriyin connect again");
        delay(5000);
      }
    }
  }


void regarPlanta(String msg){

  // Lee la humedad en %
  float h = dht.readHumidity();

  // Lee la temperatura en Celcius
  float t = dht.readTemperature();
  
  // Lee la temperatura en Farenheit
  float f = dht.readTemperature(true);

  // Revisa si no fallan las lecturas, si falla volverá a leerlas.
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  humedad = (String) h; //Convierte la humedad en string

  Serial.println(h);
  
  //El siguiente método manda los mensajes a mosquitto de que realiza la esp32 en el momento
  if (msg != ""){
    digitalWrite(relevador,HIGH);
    while(humedad<msg){
    snprintf(messages, 75, "%ld", humedad);
    client.publish(outTopic, "La humedad es... ");
    client.publish(outTopic, messages);
    client.publish(outTopic, "Regando...");
    digitalWrite(relevador,HIGH);
    //delay(30000);
    delay(10000);
    client.publish(outTopic,"Terminó el riego, tiempo de descanso 10 minutos...");
    digitalWrite(relevador,LOW);
    //delay(600000); Ideal de 10 minutos
    //delay(60000); //Prueba con un minuto de espera  
    delay(10000);
    }

  }
  
}


void callback(char* topic, byte* payload, unsigned int length){

  //Se guarda todo el valor del mensaje de payload recibido  
  for(int i=0; i<length; i++){

    mensaje+=(char) payload[i];
  }
  Serial.print(mensaje);
  Serial.println();
  
  regarPlanta(mensaje);

  mensaje = "";  
}

void setup() {
  // put your setup code here, to run once:
  pinMode(relevador,OUTPUT);
  Serial.begin(115200);
  setupWifi();
  client.setServer(broker, 1883);
  client.setCallback(callback);
}

void loop() {
  // put your main code here, to run repeatedly:
  if (!client.connected()){
    reconnect();
  }
  client.loop();
  delay(1000);

}