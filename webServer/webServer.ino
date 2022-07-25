// Programa: Web Server com modulo ESP8266
// Alteracoes e adaptacoes: FILIPEFLOP

#include <SoftwareSerial.h>

//RX pino 2, TX pino 3
SoftwareSerial esp8266(2, 3);

#define DEBUG true

int unsigned long tempoInicial = 0;

void setup()
{
  Serial.begin(115200);
  esp8266.begin(115200);
  pinMode(13, OUTPUT);


  sendData("AT+RST\r\n", 2000, DEBUG); // rst
  // Conecta a rede wireless
  sendData("AT+CWJAP=\"Pede A Senha\",\"naotemsenha\"\r\n", 15000, DEBUG);
  delay(3000);
  sendData("AT+CWMODE=1\r\n", 1000, DEBUG);
  // Mostra o endereco IP
  sendData("AT+CIFSR\r\n", 1000, DEBUG);
  // Configura para multiplas conexoes
  sendData("AT+CIPMUX=1\r\n", 1000, DEBUG);
  // Inicia o web server na porta 80
  sendData("AT+CIPSERVER=1,80\r\n", 1000, DEBUG);
  tempoInicial = millis();
}

String payload;
int unsigned long tempo = 0;
int timeout = 5000;
int alarme = LOW;
int prevState = 0;

void loop()
{
  // Verifica se o ESP8266 esta enviando dados
  delay(1000);
  int connectionId = esp8266.read() - 48;

  int state = digitalRead(8);
  if (state){
    tempo = 0;  //zera o temporizador quando a porta é fechada
    tempoInicial = millis();
    alarme = 0;
  }else{
    tempo = millis() - tempoInicial; //Inicia o contador  
    if(tempo > timeout){ // Disparar alarme pois excedeu o tempo
        alarme = 1; 
    }
  }

  Serial.print("\n alarme: ");
  Serial.print(alarme);
  Serial.print("\n state : ");
  Serial.print(state);
  Serial.print("\n prevState : ");
  Serial.print(prevState);
  Serial.print("\n tempo : ");
  Serial.print(tempo);
  Serial.print("\n tempo inicial : ");
  Serial.print(tempoInicial);
  
  digitalWrite(13,alarme);
  prevState = state;
  
  if (esp8266.available())
  {
    if (esp8266.find("+IPD,"))
    {      
      
      payload = "HTTP/1.1 200 OK\nAccess-Control-Allow-Origin: *\nConnection: Close\nContent-Type: application/json; charset=UTF-8\n\n{\"response\":{\"estado\":";
      
      payload += state;
      payload += ",\"alarme\":";
      payload += alarme;
      payload += ",\"tempo\":";
      payload += tempo;
      payload += "}}";
      

      String cipSend = "AT+CIPSEND=";
      cipSend += connectionId;
      cipSend += ",";
      cipSend += payload.length();
      cipSend += "\r\n";

      sendData(cipSend, 1000, DEBUG);
      sendData(payload, 1000, DEBUG);

      String closeCommand = "AT+CIPCLOSE=";
      closeCommand += connectionId; // append connection id
      closeCommand += "\r\n";

      sendData(closeCommand, 3000, DEBUG);
    }
  }
}

String sendData(String command, const int timeout, boolean debug)
{
  // Envio dos comandos AT para o modulo
  String response = "";
  Serial.print(command);
  esp8266.print(command);
  long int time = millis();
  while ( (time + timeout) > millis())
  {
    while (esp8266.available())
    {
      // The esp has data so display its output to the serial window
      char c = esp8266.read(); // read the next character.
      response += c;
    }
  }
  if (debug)
  {
    Serial.print(response);
  }
  return response;
}
