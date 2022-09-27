#ifndef CONNECTIONSIM_H_
#define CONNECTIONSIM_H_
 
 
#include <ESP8266WiFi.h>
#include <Thread.h>
#include <DFRobot_SIM808.h>
#include <SoftwareSerial.h>

class ConnectionSIM{
public:
	ConnectionSIM(const int PIN_TX, const int PIN_RX, const int dados);

	void getGPS(const char* ssid, const char* password);
	bool getTimestamp(int* day, int* month, int* year, int* hour, int* minute, int* second);
	void sendSMS(const char* message, const char* phone);
	bool setHour(int* h, int* m, int *s, int* y, int* mo, int* d);
	void postOnEndpoint(JsonObject object, String host);

	virtual ~ConnectionSIM();

private:

	void isLeap(int y);
	void initSIM();
	void sendsim808(const char* msg, int waitMs = 500);
	void parseATText(byte b);
	void resetBuffer();
	
	void readingDataReceived(void (*onData)(String data));
};

#endif 
