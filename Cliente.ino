#include <SPI.h>

#include <Ethernet.h>



byte mac[] = { 0x99, 0xA2, 0xDA, 0x0E, 0xBF, 0x72 };

IPAddress ip(192,168,1,177);





struct Control{

  String identifier;

  int value;

};



char hostname[] = "stuffcontrol.ddns.net";

int port = 8080;

char SocketServer[] = "stuffcontrol.ddns.net";

int SocketPort = 3005;



EthernetClient client;

EthernetClient socketClient;



String Key = "KBQepUQYDT0lZvr ";



  char Buffer[20];

  int contBuffer = 0;

  String identifier;

  float value;

  bool ReadyRead = false;



int PIN_FAIL = 7;

int PIN_OK = 3;

int PIN_COMP = 2;



unsigned long time;

unsigned long initial_time=millis();

int wait = 5000;



unsigned long time_reconnect;

unsigned long initial_time_reconnect=millis();

int wait_reconnect = 13000;



unsigned long time_send;

unsigned long initial_time_send=millis();

int wait_send = 3000;



unsigned long time_heartbeat;

unsigned long initial_time_heartbeat=millis();

int wait_heartbeat = 4000;



String PostData="";

int cont_send = 0;



void setup() {

     if (Ethernet.begin(mac) == 0) {

    Serial.println("Failed to configure Ethernet using DHCP");

    // no point in carrying on, so do nothing forevermore:

    // try to congifure using IP address instead of DHCP:

    Ethernet.begin(mac, ip);

  }

    pinMode(PIN_FAIL, OUTPUT);

    pinMode(PIN_OK, OUTPUT);

    digitalWrite(PIN_FAIL,LOW);

    digitalWrite(PIN_OK,LOW);

       

    

}



void post(String key, String identifier, float value)

{

    if (cont_send!=0){

      PostData=PostData+";";

    }

    PostData=PostData+"key"+String(cont_send)+"="+key+";identifier"+String(cont_send)+"="+identifier+";value"+String(cont_send)+"="+String(value);

    cont_send++;

  

}



void send(){

  Serial.begin(9600);

    time_send = millis();

    if(time_send - initial_time_send > wait_send){

      initial_time_send = time_send;



      if (client.connect(hostname,port)) {

        client.println("POST /sensor_value/ HTTP/1.1");

        client.print("Host: ");

        client.print(hostname);

        client.print(":");

        client.println(port);

        client.println("User-Agent: Arduino/1.0");

        client.println("Connection: close");

        client.println("Content-Type: application/x-www-form-urlencoded;");

        client.print("Content-Length: ");

        client.println(PostData.length());

        client.println();

        client.println(PostData);



        client.stop();

      }





    PostData = "";

    cont_send=0;

  }



}



void controlLED(int i){

  if(i==1){

  digitalWrite(PIN_FAIL,HIGH);

  digitalWrite(PIN_OK,LOW);

  }

  else if(i==0){

  digitalWrite(PIN_OK,HIGH);

  digitalWrite(PIN_FAIL,LOW);

    }

  }



struct Control ReadOrder(){

   //digitalWrite(PIN_COMP,HIGH);

  struct Control controller;

  controller.identifier="";

  controller.value=-1;



  if(socketClient.connected()){

    if (socketClient.available()){

      char c = socketClient.read();

      if(c=='<'){

          ReadyRead=true;

      }

      

      else{

          if(ReadyRead){

              if(c==':'){      

                  char aux[contBuffer];

                  for(int i =0;i<contBuffer;i++){

                      aux[i] = Buffer[i];

                  }

                  identifier="";

                  for(int i =0;i<contBuffer;i++){

                      identifier = identifier + String(aux[i]);

                  }

                  contBuffer = 0;      

              }

              else if(c=='>'){

                  String aux="";

                  for(int i =0;i<contBuffer;i++){

                      aux = aux + String(Buffer[i]);

                  }

                  value = aux.toFloat() ;

                  contBuffer = 0;  

                  controller.value = value;

                  controller.identifier = identifier;

                  ReadyRead=false;

              }

              else{

                  Buffer[contBuffer]=c;

                  contBuffer=contBuffer+1;

              }

          }

      }



}

}

else{

    ReadyRead=false;

}



return controller;

}





void Send_PotenciometerSensor(){



  float value = (float)analogRead(A1)/100;



  post(Key,"Sensor",value);

  

}



void reconnect(){



  time_reconnect = millis();  

  

  if(!socketClient.connected()){

    delay(600);

    if(socketClient.connect(SocketServer,SocketPort)){

     socketClient.print("key:"+Key);

    }

  }

  

  if(time_reconnect - initial_time_reconnect > wait_reconnect){

    initial_time_reconnect = time_reconnect;

    if(socketClient.connected()){

    socketClient.stop();

    delay(500);

    }

    if(socketClient.connect(SocketServer,SocketPort)){

      socketClient.print("key:"+Key);

      

      delay(500);

      socketClient.print("heartbeat");

        

    }  

  }



}



void heartbeat(){



  time_heartbeat= millis();

  if(time_heartbeat - initial_time_heartbeat > wait_heartbeat){

    initial_time_heartbeat = time_heartbeat;

    if(socketClient.connected()){

      Serial.print("Latido enviado");

        socketClient.print("heartbeat");

    } 

    

    delay(500);

    

  }

  

}



void SendSensorValues(){

  time = millis();

  

  if(time - initial_time > wait){

    initial_time = time;

    Send_PotenciometerSensor();



  }

  send();



}

void loop() {

  struct Control controller = ReadOrder();

  if(controller.identifier != "" && controller.value != -1){

    if(controller.identifier=="Controlador")controlLED(value);

  }

  SendSensorValues();

  heartbeat(); 

  reconnect();

}