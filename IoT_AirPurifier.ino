/*
  Rui Santos
  Complete project details at our blog.
    - ESP32: https://RandomNerdTutorials.com/esp32-firebase-realtime-database/
    - ESP8266: https://RandomNerdTutorials.com/esp8266-nodemcu-firebase-realtime-database/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  Based in the RTDB Basic Example by Firebase-ESP-Client library by mobizt
  https://github.com/mobizt/Firebase-ESP-Client/blob/main/examples/RTDB/Basic/Basic.ino
*/

#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include "MQ135.h"

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "Ilvana"
#define WIFI_PASSWORD "cedevita"

// Insert Firebase project API Key
#define API_KEY "AIzaSyA9kP9RDEGUXywpLa2ysU-maFec0w5G5yg"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://iot-air-purifier-default-rtdb.europe-west1.firebasedatabase.app/" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;
double air_quality;
bool automatski = true;
bool manuelno = false;
double percentage;
String quality = " ";
bool postepenoUpali=false;

void setup() {
  // put your setup code here, to run once:
 
    pinMode(D1, OUTPUT);    

    Serial.begin(115200);
    Serial.println("Connecting to ");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

   while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print(".");
      delay(550);
    }

  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")) {
    Serial.println("ok");
    signupOK = true;
  }
  else {
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

    
}

void loop() {
  // put your main code here, to run repeatedly:
  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
     air_quality = analogRead(A0);
     percentage=((air_quality)/1024.0)*100.0;
  delay(500);

  Serial.print(F("Air Quality Index: "));
  Serial.println(air_quality);
  Serial.print(F("Quality in percentage: "));
  Serial.println(percentage);
  Serial.println("%");
  Serial.print(F("Quality: "));
  Serial.println(quality);

   if (Firebase.RTDB.getBool(&fbdo, "/airpurifier/automatski")) {   
      if (fbdo.dataType() == "boolean") {
        automatski = fbdo.boolData();
        Serial.println("automatski:");
        Serial.println(automatski);
      }
    }
    else {
      Serial.println(fbdo.errorReason());
    }
    
    if (automatski == true) {
      if (Firebase.RTDB.setFloat(&fbdo, "airpurifier/indeks", air_quality)){
        Serial.println("PASSED - indeks");
         if(air_quality <= 100.0){
            quality="Normal";
            analogWrite(D1, 0);
        }     
        else if(air_quality > 100.0 && air_quality <= 200.0){
          quality="Medium";
          analogWrite(D1, 70);
        }
        else{
          quality="High!";
          analogWrite(D1, 255);
        }
       if (isnan(air_quality)) {
        Serial.println(F("Failed to read from MQ135 sensor!"));
        return;
        }
      }
      else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }
      if (Firebase.RTDB.setFloat(&fbdo, "airpurifier/postotak", percentage)){
        Serial.println("PASSED - postotak");
      }
      else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }      
      if (Firebase.RTDB.setString(&fbdo, "airpurifier/kvaliteta", quality)){
        Serial.println("PASSED - kvaliteta");
      }
      else {
        Serial.println("FAILED");
        Serial.println("REASON: " + fbdo.errorReason());
      }
    }
    else if (Firebase.RTDB.getBool(&fbdo, "/airpurifier/manuelno")) {   
      if (fbdo.dataType() == "boolean") {
        manuelno = fbdo.boolData();
        if(manuelno == true){
          analogWrite(D1, 255);          
        }
        else{
          analogWrite(D1, 0);
        }
      }      
    }

    else {
      Serial.println(fbdo.errorReason());
    }
  }
}

