#ifndef CONNECTIONSIM_H_
#define CONNECTIONSIM_H_

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
	void postOnEndpoint(String object, String host);
  bool initSIM();
  virtual ~ConnectionSIM();

private: 
  SoftwareSerial *mySerial;
  DFRobot_SIM808 *sim808;
private:
	int isLeap(int y);
	void sendsim808(const char* msg, int waitMs);
	void parseATText(byte b);
	void resetBuffer();
	
	void readingDataReceived(void (*onData)(String data));
};

#endif 
