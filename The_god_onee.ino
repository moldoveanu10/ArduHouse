#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <WiFiClient.h>
#include <Arduino.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#define API_KEY "AIzaSyCgSFG1xX6jqR5IlwQjNxMOKzLNBXuONbI"
#define DATABASE_URL "arduhouse-us-default-rtdb.firebaseio.com"


FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long dataMillis = 0;
bool signupOK = false;

const char* ssid = "Wi-fi moca";
const char* password = "12345678";

#define TCS 5
OneWire tcss(TCS);
DallasTemperature rcs(&tcss);

int caldura1 = 14; // Incalzire camera 1
int caldura2 = 12; // Incalzire camera 2
int caldura3 = 13; // Incalzire camera 3
int lumina1 = 15; // Lumina camera 1
int lumina2 = 3; // Lumina camera 2
int lumina3 = 1; // Lumina camera 3
int LS1 = 4; // Senzor lumina

int heat1_v = HIGH;
int heat2_v = HIGH;
int heat3_v = HIGH;
int light1_v = LOW;
int light2_v = LOW;
int light3_v = LOW;
int ls1value = LOW;
float tempc1 = 0;
float tempc2 = 0;
float tempc3 = 0;
bool connected = false;
bool lsr1 = false;
bool lsr2 = false;
bool lsr3 = false;
bool tl1 = false;
bool tl2 = false;
bool tl3 = false;
bool tt1 = false;
bool tt2 = false;
bool tt3 = false;
bool keep_tt1 = false;
bool keep_tt2 = false;
bool keep_tt3 = false;
float set_heat1 = 23;
float set_heat2 = 23;
float set_heat3 = 23;
String timer1l;
String timer2l;
String timer3l;
String timer1t;
String timer2t;
String timer3t;



WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 10800);  // setare offset pentru EET (UTC+2)


WiFiServer server(80);
WiFiClient client;


void setup() {
  Serial.begin(9600);
  delay(10);

  // Conectarea la WI-FI

  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Pornim serverul

  server.begin();
  Serial.println("Server started");

  // Afisarea IP-ului in Serial Monitor

  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());

  Serial.println("/");

  //Setam tipul pinilor (intrare/iesire)

  pinMode(caldura1, OUTPUT);
  pinMode(caldura2, OUTPUT);
  pinMode(caldura3, OUTPUT);
  pinMode(lumina1, OUTPUT);
  pinMode(lumina2, OUTPUT);
  pinMode(lumina3, OUTPUT);
  pinMode(LS1, INPUT);

  // Programul incepe cu tot oprit

  digitalWrite(caldura1, HIGH);
  digitalWrite(caldura2, HIGH);
  digitalWrite(caldura3, HIGH);
  digitalWrite(lumina1, LOW);
  digitalWrite(lumina2, LOW);
  digitalWrite(lumina3, LOW);

  rcs.begin();
  timeClient.begin();


  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;
  Firebase.reconnectWiFi(true);

  Serial.print("Sign up new user... ");


  if (Firebase.signUp(&config, &auth, "", ""))
  {
    Serial.println("ok");
    signupOK = true;

  }
  else
    Serial.printf("%s\n", config.signer.signupError.message.c_str());


  config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&config, &auth);
}




void loop() {

  // Prima data solicit valorile masurate de senzori

  // Temperatura

  rcs.requestTemperatures();
  tempc3 = rcs.getTempCByIndex(0);
  tempc2 = rcs.getTempCByIndex(1);
  tempc1 = rcs.getTempCByIndex(2);

  // Lumina

  ls1value = digitalRead(LS1);

  // Citim ora

  timeClient.update(); // Obținerea timpului actual de la serverul NTP

  // Formatarea orei într-un șir de caractere


  char realTime[5];
  sprintf(realTime, "%02d%02d", timeClient.getHours(), timeClient.getMinutes());


  // Postam informatiile citite de senzori

  Serial.printf("Set int... %s\n", Firebase.RTDB.setString(&fbdo, "realtime/timp", realTime) ? "ok" : fbdo.errorReason().c_str());
  Serial.printf("Set int... %s\n", Firebase.RTDB.setString(&fbdo, "realtime/ip", WiFi.localIP().toString()) ? "ok" : fbdo.errorReason().c_str());
  Serial.printf("Set int... %s\n", Firebase.RTDB.setFloat(&fbdo, "realtime/set_heat1", set_heat1) ? "ok" : fbdo.errorReason().c_str());
  Serial.printf("Set int... %s\n", Firebase.RTDB.setFloat(&fbdo, "realtime/set_heat2", set_heat2) ? "ok" : fbdo.errorReason().c_str());
  Serial.printf("Set int... %s\n", Firebase.RTDB.setFloat(&fbdo, "realtime/set_heat3", set_heat3) ? "ok" : fbdo.errorReason().c_str());

  Serial.printf("Set int... %s\n", Firebase.RTDB.setBool(&fbdo, "realtime/lsr1", lsr1) ? "ok" : fbdo.errorReason().c_str());
  Serial.printf("Set int... %s\n", Firebase.RTDB.setBool(&fbdo, "realtime/lsr2", lsr2) ? "ok" : fbdo.errorReason().c_str());
  Serial.printf("Set int... %s\n", Firebase.RTDB.setBool(&fbdo, "realtime/lsr3", lsr3) ? "ok" : fbdo.errorReason().c_str());

  Serial.printf("Set int... %s\n", Firebase.RTDB.setInt(&fbdo, "realtime/heat1_v", heat1_v) ? "ok" : fbdo.errorReason().c_str());
  Serial.printf("Set int... %s\n", Firebase.RTDB.setInt(&fbdo, "realtime/heat2_v", heat2_v) ? "ok" : fbdo.errorReason().c_str());
  Serial.printf("Set int... %s\n", Firebase.RTDB.setInt(&fbdo, "realtime/heat3_v", heat3_v) ? "ok" : fbdo.errorReason().c_str());
  Serial.printf("Set int... %s\n", Firebase.RTDB.setInt(&fbdo, "realtime/light1_v", light1_v) ? "ok" : fbdo.errorReason().c_str());
  Serial.printf("Set int... %s\n", Firebase.RTDB.setInt(&fbdo, "realtime/light2_v", light2_v) ? "ok" : fbdo.errorReason().c_str());
  Serial.printf("Set int... %s\n", Firebase.RTDB.setInt(&fbdo, "realtime/light3_v", light3_v) ? "ok" : fbdo.errorReason().c_str());

  if (millis() - dataMillis > 2000 && signupOK && Firebase.ready())
  {
    dataMillis = millis();
    //String path = auth.token.uid.c_str(); //<- user uid

    Serial.printf("Set int... %s\n", Firebase.RTDB.setFloat(&fbdo, "realtime/tempc1", tempc1) ? "ok" : fbdo.errorReason().c_str());
    Serial.printf("Set int... %s\n", Firebase.RTDB.setFloat(&fbdo, "realtime/tempc2", tempc2) ? "ok" : fbdo.errorReason().c_str());
    Serial.printf("Set int... %s\n", Firebase.RTDB.setFloat(&fbdo, "realtime/tempc3", tempc3) ? "ok" : fbdo.errorReason().c_str());


  }



  // Verificarea conexiunii

  if (!connected) {
    connectClient();
  } else {
    readData();
  }





  if (ls1value == HIGH && lsr1 == true) {
    digitalWrite(lumina1, HIGH);
    light1_v = HIGH;
    lsr1 = false;
  }


  if (ls1value == HIGH && lsr2 == true) {
    digitalWrite(lumina2, HIGH);
    light2_v = HIGH;
    lsr2 = false;
  }


  if (ls1value == HIGH && lsr3 == true) {
    digitalWrite(lumina3, HIGH);
    light3_v = HIGH;
    lsr3 = false;
  }


  if (tl1 == true && timer1l == realTime) {
    digitalWrite(lumina1, HIGH);
    light1_v = HIGH;
    tl1 = false;
  }


  if (tl2 == true && timer2l == realTime) {
    digitalWrite(lumina2, HIGH);
    light2_v = HIGH;
    tl2 = false;
  }


  if (tl3 == true && timer3l == realTime) {
    digitalWrite(lumina3, HIGH);
    light3_v = HIGH;
    tl3 = false;
  }


  if (tt1 == true && timer1t == realTime) {
    keep_tt1 = true;
    heat1_v = LOW;
    tt1 = false;
  }


  if (tt2 == true && timer2t == realTime) {
    keep_tt2 = true;
    heat2_v = LOW;
    tt2 = false;
  }


  if (tt3 == true && timer3t == realTime) {
    keep_tt3 = true;;
    heat3_v = LOW;
    tt3 = false;
  }

  if (keep_tt1 == true) {

    if (set_heat1 - tempc1 > 0) {
      digitalWrite(caldura1, LOW);


    } else if (set_heat1 - tempc1 < 0) {
      digitalWrite(caldura1, HIGH);
    }
  }




  if (keep_tt2 == true) {
    if (set_heat2 - tempc2 > 0) {
      digitalWrite(caldura2, LOW);
    } else if (set_heat2 - tempc2 < 0) {
      digitalWrite(caldura2, HIGH);
    }
  }

  if (keep_tt3 == true) {
    if (set_heat3 - tempc3 > 0) {
      digitalWrite(caldura3, LOW);
    } else if (set_heat3 - tempc3 < 0) {
      digitalWrite(caldura3, HIGH);
    }
  }

}

void connectClient() {

  // Asteptarea unei conexiuni

  client = server.available();
  if (client) {
    Serial.println("Conexiune realizata");
    connected = true;
  }
}


void readData() {

  // Citirea datelor transmise de aplicatia Android

  while (client.available()) {
    String data = client.readStringUntil('\r');
    Serial.println(data);
    processRequest(data);
    client.stop();
  }

  // Inchiderea conexiunii daca clientul s-a deconectat

  if (!client.connected()) {
    Serial.println("Client deconectat");
    client.stop();
    connected = false;
  }
}





void processRequest(String data) {

  data = data.substring(5);
  data = data.substring(0, data.length() - 9);

  // Procesarea datelor primite si prelucrarea acestora
  if (data.length() == 8) {

    // Chestii de procesarea timpului
    String label = data.substring(0, 4);
    String time = data.substring(4);



    // Vedem ce fel de label s-a trimis sa categorisim cererea


    if (label == "LR1T") {

      tl1 = true;
      timer1l = time;
    }


    if (label == "LR2T") {
      tl2 = true;
      timer2l = time;
    }


    if (label == "LR3T") {
      tl3 = true;
      timer3l = time;
    }


    if (label == "TR1T") {
      tt1 = true;
      timer1t = time;
    }


    if (label == "TR2T") {
      tt2 = true;
      timer2t = time;
    }


    if (label == "TR3T") {
      tt3 = true;
      timer3t = time;
    }
  }

  if (data.length() == 12) {

    String label = data.substring(0, 10);
    String temp = data.substring(10);
    float set_heat = temp.toFloat();

    if (label == "SET1TEMPER") {
      set_heat1 = set_heat;
    }
    if (label == "SET2TEMPER") {
      set_heat2 = set_heat;
    }
    if (label == "SET3TEMPER") {
      set_heat3 = set_heat;
    }

  }

  // Aici se executa restul care nu necesita procesat request-ul


  if (data == "LIGHT1=ON") {
    digitalWrite(lumina1, HIGH);
    light1_v = HIGH;

  } else if (data == "LIGHT1=OFF") {
    digitalWrite(lumina1, LOW);
    light1_v = LOW;
  }


  if (data == "LIGHT2=ON") {
    digitalWrite(lumina2, HIGH);
    light2_v = HIGH;
  } else if (data == "LIGHT2=OFF") {
    digitalWrite(lumina2, LOW);
    light2_v = LOW;
  }


  if (data == "LIGHT3=ON") {
    digitalWrite(lumina3, HIGH);
    light3_v = HIGH;
  } else if (data == "LIGHT3=OFF") {
    digitalWrite(lumina3, LOW);
    light3_v = LOW;
  }


  if (data == "LSLIGHT1ON") {
    lsr1 = true;
  } else if (data == "LSLIGHT1OF") {
    lsr1 = false;
  }


  if (data == "LSLIGHT2ON") {
    lsr2 = true;
  } else if (data == "LSLIGHT2OF") {
    lsr2 = false;
  }


  if (data == "LSLIGHT3ON") {
    lsr3 = true;
  } else if (data == "LSLIGHT3OF") {
    lsr3 = false;
  }



  if (data == "HE1ON") {
    keep_tt1 = true;
    heat1_v = LOW;
  } else if (data  == "HE1OFF") {
    digitalWrite(caldura1, HIGH);
    keep_tt1 = false;
    heat1_v = HIGH;
  }


  if (data  == "HE2ON") {
    keep_tt2 = true;
    heat2_v = LOW;
  } else if (data == "HE2OFF") {
    digitalWrite(caldura2, HIGH);
    keep_tt2 = false;
    heat2_v = HIGH;
  }


  if (data == "HE3ON") {
    keep_tt3 = true;
    heat3_v = LOW;
  } else if (data  == "HE3OFF") {
    digitalWrite(caldura3, HIGH);
    keep_tt3 = false;
    heat3_v = HIGH;
  }

}










