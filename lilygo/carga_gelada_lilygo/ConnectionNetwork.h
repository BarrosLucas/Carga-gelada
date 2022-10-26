#ifndef CONNECTIONNETWORK_H_
#define CONNECTIONNETWORK_H_
 
 
#include <WiFi.h>
#include <HTTPClient.h>
#include <Thread.h>

class ConnectionNetwork{
public:
	ConnectionNetwork();

	
	
	void networkConnect(const char* ssid, const char* password);
	void onDataCallback(void (*onData)(String data));
	int sendData(char *data, const char* host, const char *ret);
	void closeConnection();
	
	static String readingData();

	virtual ~ConnectionNetwork();

private:

	void (*onData)(String data);
	
  bool establishConnection(const char* host, const int port);
	void readingDataReceived(void (*onData)(String data));
  int string2ByteArray(char* input, unsigned char* output);
};

#endif 
