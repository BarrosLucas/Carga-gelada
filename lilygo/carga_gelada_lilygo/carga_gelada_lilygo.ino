#include <OneWire.h>  
#include <DallasTemperature.h>
#include "ConnectionNetwork.h"
#include "connectionSIM.h"
#include "SDCard.h"


//const int chipSelect = D8;

#define PIN_TX 26
#define PIN_RX 27

#define data 4

char phone[16] = "043999214040";
//char nameFile[9] = "LOG.TXT";

char* ssid = "brisa-941446";
char* pass = "vgudfwye";

char* host = "http://carga-gelada-test.herokuapp.com/musics/";
char* h = "http://carga-gelada-test.herokuapp.com";
const int p = 443;

unsigned long timer;

OneWire oneWire(data);  
DallasTemperature sensors(&oneWire);

void sendData(float tempC);

ConnectionNetwork connectionNetwork;
SDCard sdCard;
ConnectionSIM *connectionSIM;

void setup(){   
  Serial.begin(9600);

  Serial.println("*****SDCard init*****");
  //sdCard = SDCard();
  //sdCard.initCard();
  
  Serial.println("\n\n*****Setting WiFi*****");
  connectionNetwork = ConnectionNetwork();
  connectionNetwork.networkConnect(ssid, pass);

  Serial.println("\n\n*****Setting SIM Card******");
  connectionSIM = new ConnectionSIM(PIN_TX, PIN_RX);

  Serial.println("\n\n*****Setting Sensor*******");
  //sensors.begin(); 
} 
void loop(){ 
//  sensors.requestTemperatures();
  float tempC = 35.3;

  Serial.print("A temperatura é: ");
  Serial.print(tempC);
  Serial.println("°C");
  sendData(tempC);
  
  delay(1000);
}


void sendData(float tempC){
  char msg[100]; 
  String json = generateJSON(tempC);
  json.toCharArray(msg, 100);

  const char* response;
  //int ret = connectionNetwork.sendData(msg, host, response);
  int ret = 0;
  Serial.print("Return: ");
  Serial.println(ret);
  Serial.print("Payload: ");
  Serial.println(response);
  
  if(ret==201){
    Serial.println("Send data by WiFi connection!");
  }else{
    Serial.println("Not was possible send data by WiFi connection...\nTrying send data by GPRS connection");
    connectionSIM->sendData(h, p, msg);
  }
  //saveData(json);
}

void saveData(String msg){
/*  Serial.println("Saving warning on SDCard");
  while(!sdCard.writeFile(msg, nameFile)){};
  Serial.println("Warning saved!");*/
}

String generateJSON(float tempC){
  float lat;
  float lon;
  float wspeed;

  char lt[12];
  char lg[12];
  char w[12];

  int h;
  int m;
  int s;
  int d;
  int mo;
  int y;

  String json = "{";
  
  if(connectionSIM->initSIM()){
    connectionSIM->getGPS(&lat, &lon, &wspeed);
    dtostrf(lat, 4, 6, lt);
    dtostrf(lon, 4, 6, lg);
    dtostrf(wspeed, 6, 2, w);
    
    connectionSIM->getTimestamp(&d, &mo, &y, &h, &m, &s);

    //timestamp: "28/9/2022 12:15:30,"
    json.concat("\"timestamp\": ");
    json.concat("\"");
    json.concat(d);
    json.concat("/");
    json.concat(mo);
    json.concat("/");
    json.concat(y);
    json.concat(" ");
    json.concat(h);
    json.concat(":");
    json.concat(m);
    json.concat(":");
    json.concat(s);
    json.concat("\",");

    //lat: 14.55555,
    json.concat("\"lat\": \"");
    json.concat(lt);
    json.concat("\",");

    //lng: 30.4444,
    json.concat("\"lng\": \"");
    json.concat(lg);
    json.concat("\",");
  }else{
    //timestamp: "28/9/2022 12:15:30,"
    json.concat("\"timestamp\": \"01/01/1971\",");

    //lat: 14.55555,
    json.concat("\"lat\": \"0.00000\",");

    //lng: 30.4444,
    json.concat("\"lng\": \"0.00000\",");
  }

  //temperature: 14.5
  json.concat("\"temperature\":");
  json.concat(tempC);
  
  json.concat("}");

  return json;
}
