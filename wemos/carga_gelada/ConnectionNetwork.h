#ifndef CONNECTIONNETWORK_H_
#define CONNECTIONNETWORK_H_
 
 
#include <ESP8266WiFi.h>
#include <Thread.h>

class ConnectionNetwork{
public:
	ConnectionNetwork();

	
	
	void networkConnect(const char* ssid, const char* password);
	bool establishConnection(const char* host, const int port);
	void onDataCallback(void (*onData)(String data));
	bool sendData(char *data);
	void closeConnection();
	
	static String readingData();

	virtual ~ConnectionNetwork();

private:

	void (*onData)(String data);
	
	void readingDataReceived(void (*onData)(String data));
  int string2ByteArray(char* input, unsigned char* output);
};

#endif 
