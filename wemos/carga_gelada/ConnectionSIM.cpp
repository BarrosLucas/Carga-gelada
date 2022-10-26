#include "ConnectionSIM.h"

#define JANUARY 	1
#define FEBRUARY 	2
#define MARCH		3
#define APRIL		4 
#define MAY			5
#define JUNE		6 
#define JULY		7
#define AUGUST		8
#define SEPTEMBER	9
#define OCTOBER		10
#define NOVEMBER	11
#define DECEMBER	12

#define APN "timbrasil.br"
#define BN "tim"
#define PW "tim"

enum _parseState {
  PS_DETECT_MSG_TYPE,

  PS_IGNORING_COMMAND_ECHO,

  PS_HTTPACTION_TYPE,
  PS_HTTPACTION_RESULT,
  PS_HTTPACTION_LENGTH,

  PS_HTTPREAD_LENGTH,
  PS_HTTPREAD_CONTENT
};
byte parseState = PS_DETECT_MSG_TYPE;
char buffer[80];
byte pos = 0;
unsigned long lastActionTime = 0;

int contentLength = 0;

ConnectionSIM::ConnectionSIM(const int PIN_TX, const int PIN_RX){
	mySerial 	= new SoftwareSerial(PIN_TX,PIN_RX);
	sim808		= new DFRobot_SIM808(mySerial);
	mySerial->begin(9600);
}

bool ConnectionSIM::initSIM(){
	Serial.println("Waiting SIM Signal...");
	long time_init = millis();
	int init;
	while((init = sim808->init())!=1 && ((millis() - time_init) <  60000)){}
  if(init){
		 Serial.println("Signal OK!");
     return true;
  }
	Serial.println("Failed Signal");
  return false;
}

void ConnectionSIM::getGPS(float* lt, float* lg, float* ws){
	Serial.println("Waiting GPS...");
	while(!sim808->attachGPS()){
		delay(1000);
	}
	Serial.println("GPS connected!");
	Serial.println("Waiting coordinates coordinates");
	while(!sim808->getGPS()){}
  Serial.println("Coordinates obtain success!");
	*lt = sim808->GPSdata.lat * -1;
	*lg = sim808->GPSdata.lon * -1;
	*ws = sim808->GPSdata.speed_kph;
	
	return;
}

void ConnectionSIM::getTimestamp(int* day, int* month, int* year, int* hour, int* minute, int* second){
	  *day = sim808->GPSdata.day;
    *month = sim808->GPSdata.month;
    *year = sim808->GPSdata.year;
    *hour = sim808->GPSdata.hour;
    *minute = sim808->GPSdata.minute;
    *second = sim808->GPSdata.second;

    setHour(hour, minute, second, year, month, day);
}

bool ConnectionSIM::sendSMS(char* message, char* phone){
	long time_init = millis();
	int sent;
	while((sent = sim808->sendSMS(phone,message))!=1 && ((millis() - time_init) < 10000)){}
	if(sent){
		return true;
	}
	return false;
}

void ConnectionSIM::setHour(int* h, int* m, int *s, int* y, int* mo, int* d){   
    if((*h) > 3){
        *h -= 3;
    }else{
        *h = 24 - (3 - *h);
        if((*d) == 1){
            -- *mo;
            switch((*mo)){
                case JANUARY:
                case MARCH:
                case MAY:
                case JULY:
                case AUGUST:
                case OCTOBER:
                    *d = 31;
                    break;
                case APRIL:
                case JUNE:
                case SEPTEMBER:
                case NOVEMBER:
                    *d = 30;
                    break;
                case FEBRUARY:
                    if(isLeap(*y)){
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

bool ConnectionSIM::sendData(const char* host, const int port, char* payload){
  char http_cmd[] = "POST /musics/ HTTP/1.1\r\nContent-Type: application/json\r\nContent-Length: 85\r\nHost: carga-gelada-test.herokuapp.com\r\n{\"timestamp\":\"29/09/2022 01:42:41\",\"lat\":\"11.111\",\"lng\":\"22.222\",\"temperature\":11.2}";
  char buffer[512];
  
  Serial.println("Join on DHCP network");
  while(!sim808->join(F(APN), F(BN), F(PW))) {}
  Serial.println("Succesful DHCP");

  //************ Successful DHCP ****************
  Serial.print("IP Address is ");
  Serial.println(sim808->getIPAddress());
  long start = millis();
  Serial.println("Trying send...");

  bool c = sim808->connect(TCP,host, port);
  //*********** Establish a TCP connection ************
  while(!c && ((millis() -  start) < 5000)) {
      c = sim808->connect(TCP,host, port);
  }
  
  if(!c){
     Serial.println("Connect to server fail");
     return c;
  }

  Serial.println("Connect to server success");

  bool r = false;

  //*********** Send a POST request *****************  
  Serial.println("waiting to fetch...");
  Serial.println(sim808->send(http_cmd, sizeof(http_cmd)-1));
  while (true) {
      int ret = sim808->recv(buffer, sizeof(buffer)-1);
      if (ret <= 0){
          Serial.println("fetch over...");
          r = false;
          break; 
      }
      buffer[ret] = '\0';
      Serial.print("Recv: ");
      Serial.print(ret);
      Serial.print(" bytes: ");
      Serial.println(buffer);
      r = true;
      break;
  }
  //************* Close TCP or UDP connections **********
  sim808->close();

  //*** Disconnect wireless connection, Close Moving Scene *******
  sim808->disconnect();

  return r;
}


int ConnectionSIM::isLeap(int y){
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

ConnectionSIM::~ConnectionSIM() {}
