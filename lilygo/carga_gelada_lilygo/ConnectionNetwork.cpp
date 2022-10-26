#include "ConnectionNetwork.h"


WiFiClient client;
Thread *thread;

//Utilizada no callback, para simular uma thread
long lastTimeOnData = -1; 


ConnectionNetwork::ConnectionNetwork(){
	
}


/*
PUBLIC 
CONECTA O ARDUINO � REDE WIFI DISPONIVEL
PARAMETROS:
const char* ssid       -> nome da rede
const char* password   -> senha da rede
*/
void ConnectionNetwork::networkConnect(const char* ssid, const char* password){
  WiFi.mode(WIFI_STA); 							//TIPO DE CONEX�O
  WiFi.begin(ssid, password); 					//INICIA A CONEX�O POR WIFI
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {		//EXECUTA AT� QUE A CONEXAO SEJA ESTABELECIDA
    delay(500);
  }
  Serial.println("WiFi connected");
}


/*
PUBLIC
CONECTA O ARDUINO A UM HOST
PARAMETROS:
const char* host    -> IP onde se conecta
const int port      -> Porta de conex�o
void (*onConnect)   -> Callback que � executado ao ser estabelecida a conex�o
*/
bool ConnectionNetwork::establishConnection(const char* host, const int port){
  return client.connect(host, port);
}


/*
PUBLIC
CHAMA UM M�TODO QUANDO RECEBER UM DADO
PARAMETRO:
void (*onData)    -> Fun��oo a ser executada
*/
void ConnectionNetwork::onDataCallback(void (*onData)(String data)){
  this->onData = onData;
  thread = new Thread();				//CRIA UMA THREAD
  thread->setInterval(1000);				//DEFINE QUE SER� EXECUTADA A CADA 100 MS
  thread->onRun(readingData); 			//QUANDO EXECUTAR, CHAMAR A FUN��O...
  readingDataReceived(this->onData);	//INICIA UMA THREAD
}

/*
PUBLIC
FECHA A CONEX�O 
*/
void ConnectionNetwork::closeConnection(){
	client.stop();
}

/*
PUBLIC
ENVIA OS DADOS PARA O HOST NO QUAL EST� CONECTADO
PARAMETRO:
const void *data    -> Recebe o array de bytes
int size			-> Recebe o tamanho
*/
int ConnectionNetwork::sendData(char *data, const char* host, const char* ret){
  if(WiFi.status() == WL_CONNECTED){
    HTTPClient http;
    http.begin(client, host);
    http.addHeader("Content-Type", "application/json");
    Serial.println("Sending: ");
    Serial.println(data);
    int err = http.POST(data);
    if(err > 0){
      ret = http.getString().c_str();
    }
    http.end();
    return err;
  }
  return 0;
  /*unsigned char* output = (unsigned char*)(&data);
  const char* p = (const char*) output;
  int size = sizeof(data)/sizeof(char);

  Serial.println("Connecting to Server");
  long init = millis();
  while(!client.connected() && ((millis() - init) < 60000)){
    establishConnection(host, port);
  }
  if(client.connected()){
    Serial.println("Server connected");
    Serial.println("Send data...");
		while (size--) {
			client.write(p);
			*p++;
		}
		return true;
	}
 Serial.println("Server cannot connect");
	return false;*/
}


/*
PRIVATE
� CHAMADO PELO OnDataCallback PARA EXECUTAR A THREAD DE LEITURA
DA CONEX�O, VERIFICANDO SE POSSUI ALGUM DADO A SER LIDO...
�, BASICAMETNE, UMA THREAD QUE EXECUTA OUTRA THREAD
PARAMETRO:
void (*onData)()    -> Fun��o que � executada quando recebe algum dado
*/
void ConnectionNetwork::readingDataReceived(void (*onData)(String data)){
	String data = "";
	while(true){
		if(lastTimeOnData == -1){
			if (thread->shouldRun()){
				data = thread->run();
				if(data != ""){
					onData(data);	
				}
			}
			lastTimeOnData = millis();
			
		}else{
			if((millis() - lastTimeOnData)>=1000){
				if (thread->shouldRun()){
					data = thread->run();
					if(data != ""){
						onData(data);
					}
				}
				lastTimeOnData = millis();
			}
		}
		data = "";
	}
}

/*
PRIVATE
FAZ A LEITURA
N�O NECESSITA DE PARAMETRO PARA SER LIDO
*/
String ConnectionNetwork::readingData(){
  String data = "";
  while(client.available()){
	  char ch = static_cast<char>(client.read());
      data += ch;
  }
  return data;
}

/*
PRIVATE
FAZ A CONVERSÃO DE UMA STRING PARA
UM ARRAY DE BYTE

PARAMETROS:
char* input -> array de char
byte* output -> array de byte
*/
int ConnectionNetwork::string2ByteArray(char* input, unsigned char* output){
    int loop;
    int i;
    
    loop = 0;
    i = 0;
    
    while(input[loop] != '\0')
    {
        output[i++] = input[loop++];
    }
    return i;
}


ConnectionNetwork::~ConnectionNetwork() {
	// TODO Auto-generated destructor stub
	delete thread;
}
