#include "SDCard.h"


SDCard::SDCard(){
	
}

bool SDCard::initCard(){
	Serial.println("*****Setting SDCard******");
	while (!Serial) {}
	
	Serial.print("\nInitializing SD card...");
	
	if (!SD.begin(SS)) {
    	Serial.println("initialization failed. Things to check:");
    	Serial.println("* is a card inserted?");
    	Serial.println("* is your wiring correct?");
    	Serial.println("* did you change the chipSelect pin to match your shield or module?");
      return false;
  	} else {
    	Serial.println("Wiring is correct and a card is present.");
 	}
 	pinMode(SS, OUTPUT); 
  return true;
}

const char* SDCard::readFile(char* nameFile){
	File dataFile = SD.open(nameFile); 
  String content = "";
	if(dataFile){ 
		while(dataFile.available()){ //ENQUANTO HOUVER CONTEÚDO A SER LIDO, FAZ
			content.concat(dataFile.read()); //ESCREVE NA SERIAL AS INFORMAÇÕES DO ARQUIVO DE TEXTO
    }
    dataFile.close(); //ENCERRA A LEITURA (SEMPRE FECHAR O ARQUIVO ATUAL PARA ABRIR UM OUTRO ARQUIVO)
  }else{ //SENÃO, FAZ
    Serial.println("Erro ao abrir o arquivo!"); //IMPRIME O TEXTO NO MONITOR SERIAL
  }
  return content.c_str();
}

boolean SDCard::writeFile(String content, char* nameFile){
  File dataFile = SD.open(nameFile, FILE_WRITE);
  
  if (dataFile){
    dataFile.println(content);
    dataFile.close();
    return true;
  }
  return false;
}

SDCard::~SDCard() {}
