#include <OneWire.h>  
#include <DallasTemperature.h>
#include <DFRobot_SIM808.h>
#include <SoftwareSerial.h>

#define PIN_TX 10
#define PIN_RX 11
#define dados A0

#define JANEIRO 1
#define FEVEREIRO 2
#define MARCO 3
#define ABRIL 4 
#define MAIO 5
#define JUNHO 6 
#define JULHO 7 
#define AGOSTO 8
#define SETEMBRO 9
#define OUTUBRO 10
#define NOVEMBRO 11
#define DEZEMBRO 12

char lat[12];
char lon[12];
char wspeed[12];

char phone[16] = "083987915646";
char datetime[24];

unsigned long timer;

SoftwareSerial mySerial(PIN_TX,PIN_RX);
DFRobot_SIM808 sim808(&mySerial);//Connect RX,TX,PWR,

void sendSMS(float tempC);
OneWire oneWire(dados);  
DallasTemperature sensors(&oneWire);

boolean enviado = false;

void setup(){ 
  mySerial.begin(9600);
  Serial.begin(9600);
  
  while(!sim808.init()){
      Serial.print("Aguardando sinal\r\n");
      delay(1000);
  }
  delay(3000);

  Serial.println("SIM Init success");
  Serial.begin(9600);
  sensors.begin(); 
} 
void loop(){ 
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);
  
  if(tempC > 22){
    Serial.println("Temperatura elevada");
    sendSMS(tempC);
  }else{
    enviado = false;
  }
  
  Serial.print("A temperatura é: ");
  Serial.print(tempC);
  Serial.print("\n");
  delay(1000);
}


void sendSMS(float tempC){
  int day, month, year, hour, minute, second;
  if(enviado == false){
    getGPS();
    
    day = sim808.GPSdata.day;
    month = sim808.GPSdata.month;
    year = sim808.GPSdata.year;
    hour = sim808.GPSdata.hour;
    minute = sim808.GPSdata.minute;
    second = sim808.GPSdata.second;

    setHour(&hour, &minute, &second, &year, &month, &day);
    
    String msg = "";
    msg.concat(day);
    msg.concat("/");
    msg.concat(month);
    msg.concat("/");
    msg.concat(year);
    msg.concat(" ");
    msg.concat(hour);
    msg.concat(":");
    msg.concat(minute);
    msg.concat(":");
    msg.concat(second);
    msg.concat("\n");
    msg.concat("A carga esta a uma temperatura elevada: ");
    msg.concat(tempC);
    msg.concat("\nCoordenadas:\n");
    msg.concat("Latitude: ");
    msg.concat(lat);
    msg.concat("\nLongitude: ");
    msg.concat(lon);
  
    const char* ssid;
    int ssid_len = msg.length() + 1;
    char ssid_array[ssid_len];
    msg.toCharArray(ssid_array, ssid_len);
  
    Serial.println("\nEnviando mensagem...");
    sim808.sendSMS(phone,ssid_array);
    Serial.println("Mensagem enviada: ");
    Serial.println(ssid_array);
    timer = millis();
    enviado = true;
  }else{
    if(millis() > (timer + 5000)){
      enviado = false;
    }
  }
}

void getGPS(){ 
  int cont = 0;
  while(!sim808.attachGPS()){
    Serial.println("Aguardando GPS...");
    delay(1000);
  }
  delay(3000);

  Serial.println("Antena GPS conectada!");

  Serial.print("Aguardando sinal de coordenadas");
  while(!sim808.getGPS()){
    Serial.print(".");
    cont ++;
    if(cont == 100){
      Serial.print("\n");
      cont = 0;
    }
  }
  Serial.print("\n");

  float la = sim808.GPSdata.lat * -1; //Movendo ao quadrante geográfico da América Latina
  float lo = sim808.GPSdata.lon * -1; //Movendo ao quadrante geográfico da América Latina
  float ws = sim808.GPSdata.speed_kph;
  
  dtostrf(la, 4, 6, lat);
  dtostrf(lo, 4, 6, lon);
  dtostrf(ws, 6, 2, wspeed);
}


void setHour(int* h, int* m, int *s, int* y, int* mo, int* d){
    
    if((*h) > 3){
        *h -= 3;
    }else{
        *h = 24 - (3 - *h);
        if((*d) == 1){
            -- *mo;
            switch((*mo)){
                case JANEIRO:
                case MARCO:
                case MAIO:
                case JULHO:
                case AGOSTO:
                case OUTUBRO:
                    *d = 31;
                    break;
                case ABRIL:
                case JUNHO:
                case SETEMBRO:
                case NOVEMBRO:
                    *d = 30;
                    break;
                case FEVEREIRO:
                    if(ehBissexto(*y)){
                        *d = 29;
                    }else{
                        *d = 28;
                    }
                    break;
                default:
                    *d = 31;
                    *mo = 12;
                    -- *y;
                    break;
            }
            
        }else{
            *d --;
        }
    }
}
int ehBissexto(int y){
    if(y % 4 == 0){
        if(y % 100 == 0){
            if(y % 400 == 0){
                return 1;
            }
            else {
                return 0;
            }
        }else{
            return 1;
        }
    }else{
        return 0;
    }
}
