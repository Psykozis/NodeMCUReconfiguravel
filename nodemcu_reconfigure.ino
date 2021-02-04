/* Create a WiFi access point and provide a web server on it. */

#include <stdio.h>
#include <string.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include <PubSubClient.h> // integração futura
#include <user_interface.h> //lib timer 

ESP8266WebServer server(80);

os_timer_t tmr0; //create timer named tmr0
int addr = 0; //endereço que escreveremos na eprom
int flagglobal=0; // flag para configuração

//  HHHHHHHHHHHHH PONTO DE ACESSO PARA CONFIgURAÇÃO HHHHHHHHHHHHHHHHHHHHHHHHHHHH
void paginaconfig(){ //configura ponto de acesso, temporização, mantem conexão e cancela ponto de acesso quando acaba o tempo, salva dados na memória não volátil
          Serial.print("Configurando ponto de acesso inicial...");
          
          WiFi.softAP("Configura_Node"); //nome da rede de configuração
          IPAddress myIP = WiFi.softAPIP();
          server.on("/", handleRoot);
          server.on("/action_page.php", handleForm);
          server.on("/cancel.php", handleesc);
          server.onNotFound(handleNotFound); 
          server.begin();
          
          Serial.println("servidor HTTP iniciado \n rede: Configura_Node \n endereço de conexão à porta: ");
          Serial.println(myIP);
  
            os_timer_setfn(&tmr0,func_timer, NULL);  //callback for timer
            os_timer_arm(&tmr0, 210000, false);   //(timer, time for timer ms, repeat timer? (loop = true) //inicia timer
            
            while(flagglobal!=1){
                                    server.handleClient();
              }
             server.stop(); //fecha o servidor
             server.close(); //fecha o servidor 
             Serial.println("servidor fechado");
             
             WiFi.softAPdisconnect(true);//mata a rede do ponto de acesso
             
          
            server.close(); //fecha o servidor
  
  }

void handleNotFound(){ //caso tente acessar qualquer página que não a enviada para configuração
  String message = "File Not Found\n\n try to connect click <a href='192.168.4.1'>here</a>";
  Serial.print("página não encontrada");
  server.send(404, "text/plain", message);
}

void handleRoot() {//envia página de configuração
   //site salvo em https://www.w3schools.com/code/tryit.asp?filename=GNC3Z9EQ4CBX
String html ="<!DOCTYPE html><html><head><meta name='viewport' content='width=device-width, initial-scale=1.0'><title>config mqtt</title><style>*{box-sizing: border-box;}  body {background-color: #706665;}  #regForm {background-color: #302A26;margin: 100px auto;font-family: Raleway;padding: 40px;min-width: 300px;} h3 {text-align: center; color: #ffffff;}  input {padding: 10px; background-color: #706665; color: #ffffff; width: 100%;font-size: 17px;font-family: Raleway;border: 1px solid #aaaaaa;}  #button {background-color: #F7411E;color: #ffffff;border: none;padding: 10px 20px;font-size: 17px;font-family: Raleway;cursor: pointer;}#cancel {background-color: #F7411e;color: #ffffff;border: none;padding: 10px 20px;font-size: 17px;font-family: Raleway;cursor: pointer;} </style></head><body><form id='regForm' action='/action_page.php' method='post'><div class='tab'><h3>Rede a se conectar:</h3><br><p><input  type='text' id='ssid' name='ssid' value='rede'></p> <br><p><input  type='text' id='password' name='password' value='senha'></p> <br> <br><h3>MQTT broker:</h3><p><input type='text' id='IPbrok' name='IPbrok' value='endereco'></p> <br><p><input type='text' id='portbrok' name='portbrok' value='porta'></p> <br><br><h3>MQTT client config:</h3> <br><p><input type='text' id='ID' name='ID' value='Identificador'></p> <br><p><input type='text' id='route' name='route' value='rota/topic'></p> <br><br></div><input id='button' type='submit' value='Submit'><br><br><input id='cancel' type='submit' value='cancel' formaction='/cancel.php'></form></body></html>";
  server.send(200, "text/html", html);
 
}

void handleForm() { //quando acontece um submit e a chamada do /action_page acontece, essa função captura os valores das caixas de entrada
 String msg;
 String msg1; 
 String xssid = server.arg("ssid");         //server arg pega o que tem no campo do input com o name especificado
 String xpassword = server.arg("password"); 
 String xIPbrok = server.arg("IPbrok"); 
 String xportbrok = server.arg("portbrok"); 
 String xID = server.arg("ID"); 
 String xroute = server.arg("route"); 
 

 Serial.println("ssid: "+xssid);
 Serial.println("senha: "+xpassword);
 Serial.println("broker: "+xIPbrok);
 Serial.println("porta: "+xportbrok);
 Serial.println("cliente: "+xID);
 Serial.println("rota do topico: "+xroute);

 //tamanho de cada info
 int nssid = xssid.length();
 msg = String(nssid,HEX);
 Serial.println("tamanho da rede "+nssid);
  
 int npassword = xpassword.length();
 msg += String(npassword,HEX);
 Serial.println("tamanho da senha "+npassword);
 
 int nipbrok = xIPbrok.length();
 msg += String(nipbrok,HEX);
 Serial.println("tamanho do broker "+nipbrok);
 
 int nportbrok = xportbrok.length();
 msg += String(nportbrok,HEX);
 Serial.println("tamanho da porta "+nportbrok);
 
 int nid = xID.length();
 msg += String(nid,HEX);
 Serial.println("tamanho do id "+nid);
    
 int nroute = xroute.length();
 msg += String(nroute,HEX);
 Serial.println("tamanho da rota "+nroute); 
 
 Serial.println("tamanhos salvos sequencialmente em hexadecimal "+msg); 
 Serial.println("mensagem recebida da configuração");
 
  msg +=xssid;        
  msg +=xpassword;   
  msg +=xIPbrok;      
  msg +=xportbrok;   
  msg +=xID;          
  msg +=xroute;       
  
  Serial.println("tamhhos em sequencia e informacoes em sequencia "+msg);
  Serial.println("salvando na eeprom");
 
 escreveString(0,msg); //escreve string na memoria não volátil ////////////////////////////AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAPAGAR COMENTARIO

  flagglobal=1;//encerra servidor e rede
}

void handleesc(){// fecha server e rede
      flagglobal=1;
  }

void func_timer(void*z){//quando ocorre estouro de timer a flag é setada para cancelar o ponto de acesso
   handleesc(); //quando o timer contar 2minutos a flag ativa para não abrir a pagina de configuração sem um reset;
   }

//  HHHHHHHHHHHHH PONTO DE ACESSO PARA CONFIURAÇÃO HHHHHHHHHHHHHHHHHHHHHHHHHHHH  

//  HHHHHHHHHHHHH memória não volátil HHHHHHHHHHHHHHHHHHHHHHHHHHHH  
void escreveString(int enderecoBase, String mensagem){ // Salva a string nos endereços de forma sequencial
  Serial.println ("escevendo EEPROM"); // Caso não caiba mensagem de erro é mostrada
  if (mensagem.length()>EEPROM.length() || (enderecoBase+mensagem.length()) >EEPROM.length() ){ // verificamos se a string cabe na memória a partir do endereço desejado
    Serial.println ("A sua String não cabe na EEPROM"); // Caso não caiba mensagem de erro é mostrada
  }
  else{ // Caso seja possível armazenar 
    for (int i = 0; i<mensagem.length(); i++){ 
       if(mensagem[i]!=EEPROM.read(i+enderecoBase)){ //escreve apenas se for diferente
            EEPROM.write(enderecoBase,mensagem[i]); // Escrevemos cada byte da string de forma sequencial na memória
            EEPROM.commit();//Salva o dado na EEPROM.
            delay(100);
       }     
       enderecoBase++; // Deslocamos endereço base em uma posição a cada byte salvo
    }
     if(EEPROM.read(enderecoBase)!='\0'){
          EEPROM.write(enderecoBase,'\0'); // Salvamos marcador de fim da string 
          EEPROM.commit();//Salva o dado na EEPROM.
     }
     Serial.print ("salvo na eprom: ");
     Serial.println (mensagem); 
  }
}

String leString(int enderecoBase){
  String mensagem="";
   Serial.println ("lendo EEPROM"); // Caso não caiba mensagem de erro é mostrada
  if (enderecoBase>EEPROM.length()){ // Se o endereço base for maior que o espaço de endereçamento da EEPROM retornamos uma string vazia
    return mensagem;
  }
  else { // Caso contrário, lemos byte a byte de cada endereço e montamos uma nova String
    char pos;
    do{
      pos = EEPROM.read(enderecoBase); // Leitura do byte com base na posição atual
      enderecoBase++; // A cada leitura incrementamos a posição a ser lida
      mensagem = mensagem + pos; // Montamos string de saídaa
    }
    while (pos != '\0'); // Fazemos isso até encontrar o marcador de fim de string
  }
  return mensagem; // Retorno da mensagem
}
//  HHHHHHHHHHHHH memória não volátil HHHHHHHHHHHHHHHHHHHHHHHHHHHH  

void setup() {
  flagglobal=0;
  

  Serial.begin(115200);
  Serial.println();
  EEPROM.begin(128);
  Serial.println("lendo eprom");
  Serial.println(leString(0)); //mostra o que existe na eprom;
  
  paginaconfig(); //abre página de configuração //configurando o cliente inicialmente como um ponto de acesso //depois de um tempo mata o ponto
  delay(1000);  

  Serial.println("lendo eprom");
  Serial.println(leString(0)); //mostra o que existe na eprom;

  delay(500);
  
}

void loop() {

}




 
