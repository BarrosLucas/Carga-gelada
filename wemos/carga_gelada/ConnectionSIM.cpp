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

SoftwareSerial mySerial;
DFRobot_SIM808 sim808;

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

ConnectionSIM::ConnectionSIM(const int PIN_TX, const int PIN_RX, const int dados){
	mySerial 	= mySerial(PIN_TX,PIN_RX);
	sim808		= sim808(&mySerial);
	mySerial.begin(9600);
	initSIM();
}

void ConnectionSIM::initSIM(){
	Serial.println("Waiting SIM Signal...");
	long time_init = millis();
	int init;
	while((init = sim808.init())!=1 && ((millis() - time_init) <  60000)){
  	}
  	if(init){
		Serial.println("Signal OK!");
	}else{
		Serial.println("Failed Signal");
	}
}

void ConnectionSIM::getGPS(float* lt, float* lg, float* ws){
	Serial.println("Waiting GPS...");
	while(!sim808.attachGPS()){
		delay(1000);
	}
	Serial.println("GPS connected!");
	Serial.println("Waiting coordinates coordinates");
	while(!sim808.getGPS()){
		
	}
	*lt = sim808.GPSdata.lat * -1;
	*lg = sim808.GPSdata.lon * -1;
	*ws = sim808.GPSdata.speed_kph;
	
	return;
}

void ConnectionSIM::getTimestamp(int* day, int* month, int* year, int* hour, int* minute, int* second){
	*day = sim808.GPSdata.day;
    *month = sim808.GPSdata.month;
    *year = sim808.GPSdata.year;
    *hour = sim808.GPSdata.hour;
    *minute = sim808.GPSdata.minute;
    *second = sim808.GPSdata.second;

    setHour(&hour, &minute, &second, &year, &month, &day);
}

bool ConnectionSIM::sendSMS(const char* message, const char* phone){
	long time_init = millis();
	int sent;
	while((sent = sim808.sendSMS(phone,message))!=1 && ((millis() - time_init) < 10000)){}
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

void ConnectionSIM::sendsim808(const char* msg, int waitMs = 500) {
  mySerial.println(msg);
  while(mySerial.available()) {
    parseATText(mySerial.read());
  }
  delay(waitMs);
}

void ConnectionSIM::parseATText(byte b) {

  buffer[pos++] = b;

  if ( pos >= sizeof(buffer) )
    resetBuffer(); // just to be safe

  switch (parseState) {
  case PS_DETECT_MSG_TYPE: 
    {
      if ( b == '\n' )
        resetBuffer();
      else {        
        if ( pos == 3 && strcmp(buffer, "AT+") == 0 ) {
          parseState = PS_IGNORING_COMMAND_ECHO;
        }
        else if ( b == ':' ) {

          if ( strcmp(buffer, "+HTTPACTION:") == 0 ) {
            Serial.println("Received HTTPACTION");
            parseState = PS_HTTPACTION_TYPE;
          }
          else if ( strcmp(buffer, "+HTTPREAD:") == 0 ) {
            Serial.println("Received HTTPREAD");            
            parseState = PS_HTTPREAD_LENGTH;
          }
          resetBuffer();
        }
      }
    }
    break;

  case PS_IGNORING_COMMAND_ECHO:
    {
      if ( b == '\n' ) {
        Serial.print("Ignoring echo: ");
        Serial.println(buffer);
        parseState = PS_DETECT_MSG_TYPE;
        resetBuffer();
      }
    }
    break;

  case PS_HTTPACTION_TYPE:
    {
      if ( b == ',' ) {
        Serial.print("HTTPACTION type is ");
        Serial.println(buffer);
        parseState = PS_HTTPACTION_RESULT;
        resetBuffer();
      }
    }
    break;

  case PS_HTTPACTION_RESULT:
    {
      if ( b == ',' ) {
        Serial.print("HTTPACTION result is ");
        Serial.println(buffer);
        parseState = PS_HTTPACTION_LENGTH;
        resetBuffer();
      }
    }
    break;

  case PS_HTTPACTION_LENGTH:
    {
      if ( b == '\n' ) {
        Serial.print("HTTPACTION length is ");
        Serial.println(buffer);
        
        // now request content
        mySerial.print("AT+HTTPREAD=0,");
        mySerial.println(buffer);
        
        parseState = PS_DETECT_MSG_TYPE;
        resetBuffer();
      }
    }
    break;

  case PS_HTTPREAD_LENGTH:
    {
      if ( b == '\n' ) {
        contentLength = atoi(buffer);
        Serial.print("HTTPREAD length is ");
        Serial.println(contentLength);
        
        Serial.print("HTTPREAD content: ");
        
        parseState = PS_HTTPREAD_CONTENT;
        resetBuffer();
      }
    }
    break;

  case PS_HTTPREAD_CONTENT:
    {
      // for this demo I'm just showing the content bytes in the serial monitor
      Serial.write(b);
      
      contentLength--;
      
      if ( contentLength <= 0 ) {

        // all content bytes have now been read

        parseState = PS_DETECT_MSG_TYPE;
        resetBuffer();
        
        Serial.print("\n\n\n");
        
      }
    }
    break;
  }
}

void ConnectionSIM::resetBuffer() {
  memset(buffer, 0, sizeof(buffer));
  pos = 0;
}

void ConnectionSIM::postOnEndpoint(JsonObject object, String host){
  sendsim808("AT");
  sendsim808("AT+SAPBR=3,1,\"Contype\",\"GPRS\"")
  sendsim808("AT+SAPBR=3,1,\"APN\",\"airtelgprs.com\"");  
  sendsim808("AT+SAPBR=1,1",3000);
  sendsim808("AT+SAPBR=2,1");
  sendsim808("AT+HTTPINIT");  
  sendsim808("AT+HTTPPARA=\"CID\",1");
  String url = "AT+HTTPPARA=\"URL\",\""+host+"\"";
  const char* urlChar = url.c_str();
  sendsim808(urlChar);
  //=0 get post=1
  sendsim808("AT+HTTPPARA=\"CONTENT\",\"application/json\"");
  sendsim808("AT+HTTPDATA=" + String(sendtoserver.length()) + ",100000",6000);
  sendsim808(sendtoserver);
  sendsim808("AT+HTTPACTION=1");
  sendsim808("AT+HTTPREAD");
  sendsim808("AT+HTTPTERM",3000);
  delay(5000);
  mySerial.flush();
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

ConnectionNetwork::~ConnectionNetwork() {
}
