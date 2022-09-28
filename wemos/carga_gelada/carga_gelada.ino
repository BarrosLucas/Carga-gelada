#include <OneWire.h>  
#include <DallasTemperature.h>
#include <SPI.h>
#include "ConnectionNetwork.h"
#include "connectionSIM.h"
#include <SD.h>

const int chipSelect = D8;

#define PIN_TX D4
#define PIN_RX D3

#define data D2

char phone[16] = "043999214040";
char nameFile[9] = "LOG.TXT";

char* ssid = "brisa-941446";
char* pass = "vgudfwye";

char* host = "http://www.google.com.br";
int port = 80;

char* hostWithPort = "http://www.google.com.br/80";

unsigned long timer;

OneWire oneWire(data);  
DallasTemperature sensors(&oneWire);

ConnectionNetwork connectionNetwork;
ConnectionSIM *connectionSIM;

void setup(){   
  Serial.begin(9600);
  
  Serial.println("*****Setting SDCard******");
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
 
  Serial.print("\nInitializing SD card...");
 
  // we'll use the initialization code from the utility libraries
  // since we're just testing if the card is working!
  if (!SD.begin(SS)) {
    Serial.println("initialization failed. Things to check:");
    Serial.println("* is a card inserted?");
    Serial.println("* is your wiring correct?");
    Serial.println("* did you change the chipSelect pin to match your shield or module?");
    while (1);
  } else {
    Serial.println("Wiring is correct and a card is present.");
  }

  pinMode(SS, OUTPUT); //DEFINE O PINO COMO SAÍDA

  Serial.println("SDCard File Content - LOG.TXT");
  
  File dataFile = SD.open(nameFile); //dataFile RECEBE O CONTEÚDO DO ARQUIVO DE TEXTO (ABRIR UM ARQUIVO POR VEZ)
  readFile(dataFile);

  Serial.println("\n\n*****Setting WiFi*****");
  connectionNetwork = ConnectionNetwork();
  connectionNetwork.networkConnect(ssid, pass);

  Serial.println("\n\n*****Setting SIM Card******");
  connectionSIM = new ConnectionSIM(PIN_TX, PIN_RX);
  
  sensors.begin(); 
} 
void loop(){ 
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  Serial.print("A temperatura é: ");
  Serial.print(tempC);
  Serial.println("°C");
  sendData(tempC);
  
  delay(1000);
}


void sendData(float tempC){
  char msg[500]; 
  String json = generateJSON(tempC);
  json.toCharArray(msg, 500);

  boolean dataSent = false;
  if(connectionNetwork.establishConnection(host, port)){
    if(connectionNetwork.sendData(msg)){
      dataSent = true;
      Serial.println("Send data by WiFi connection!");
    }
  }
  if(!dataSent){
    Serial.println("Not was possible send data by WiFi connection...\nTrying send data by GPRS connection");
    if(connectionSIM->initSIM()){
      connectionSIM->postOnEndpoint(json,hostWithPort);
    }
  }

  saveData(json);
  //Tenta enviar via WiFi
  //Se não conseguir, tenta via SMS

  //Salva no SDCard
}

void saveData(String msg){

  Serial.println("Saving warning on SDCard");
  while(!writeFile(msg)){}
  Serial.println("Warning saved!");
  Serial.println(msg);
  
}

void readFile(File dataFile){ 
  if(dataFile){ //SE EXISTIREM DADOS A SEREM LIDOS, FAZ
    while(dataFile.available()){ //ENQUANTO HOUVER CONTEÚDO A SER LIDO, FAZ
      Serial.write(dataFile.read()); //ESCREVE NA SERIAL AS INFORMAÇÕES DO ARQUIVO DE TEXTO
    }
    dataFile.close(); //ENCERRA A LEITURA (SEMPRE FECHAR O ARQUIVO ATUAL PARA ABRIR UM OUTRO ARQUIVO)
  }
  else{ //SENÃO, FAZ
    Serial.println("Erro ao abrir o arquivo!"); //IMPRIME O TEXTO NO MONITOR SERIAL
  }
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
    
    connectionSIM->setHour(&h, &m, &s, &y, &mo, &d);

    //timestamp: "28/9/2022 12:15:30,"
    json.concat("timestamp: ");
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
    json.concat(",\"");

    //lat: 14.55555,
    json.concat("lat: ");
    json.concat(lt);
    json.concat(",");

    //lng: 30.4444,
    json.concat("lng: ");
    json.concat(lg);
    json.concat(",");
  }else{
    //timestamp: "28/9/2022 12:15:30,"
    json.concat("timestamp: \"01/01/1971\"");

    //lat: 14.55555,
    json.concat("lat: 0.00000,");

    //lng: 30.4444,
    json.concat("lng: 0.00000,");
  }

  //temperature: 14.5
  json.concat("temperature: ");
  json.concat(tempC);
  
  json.concat("}");

  return json;
}

boolean writeFile(String content){
  //Abre o arquivo datalog.txt
  File dataFile = SD.open(nameFile, FILE_WRITE);
  //Grava as informacoes no arquivo
  if (dataFile){
    dataFile.println(content);
    dataFile.close();
    return true;
  }
  return false;
}
