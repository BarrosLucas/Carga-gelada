#ifndef CONNECTIONSIM_H_
#define CONNECTIONSIM_H_

#include <Thread.h>
#include <Sim800L.h>
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
  Sim800L *sim800;
private:
	int isLeap(int y);
	
	void readingDataReceived(void (*onData)(String data));
};

#endif 
