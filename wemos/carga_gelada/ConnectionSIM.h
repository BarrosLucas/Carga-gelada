#ifndef CONNECTIONSIM_H_
#define CONNECTIONSIM_H_

#include <ESP8266HTTPClient.h>
#include <Thread.h>
#include <DFRobot_SIM808.h>
#include <SoftwareSerial.h>

class ConnectionSIM{
public:
	ConnectionSIM(const int PIN_TX, const int PIN_RX);

	void getGPS(float* lt, float* lg, float* ws);
	void getTimestamp(int* day, int* month, int* year, int* hour, int* minute, int* second);
	bool sendSMS(char* message, char* phone);
	void setHour(int* h, int* m, int *s, int* y, int* mo, int* d);
	bool sendData(const char* host, const int port, char* payload);
  bool initSIM();
  virtual ~ConnectionSIM();

private: 
  SoftwareSerial *mySerial;
  DFRobot_SIM808 *sim808;
private:
	int isLeap(int y);
	
	void readingDataReceived(void (*onData)(String data));
};

#endif 
