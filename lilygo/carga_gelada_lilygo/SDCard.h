#ifndef SDCARD_H_
#define SDCARD_H_

#include <SPI.h>
#include <SD.h>

class SDCard{
public:
	SDCard();

	bool initCard();
  const char* readFile(char* nameFile);
  boolean writeFile(String content, char* nameFile);
 
  virtual ~SDCard();
 
private:
};

#endif 
