#include <Arduino.h>
#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>
#include <LiquidCrystal.h>
//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "M2-TIC-2023"
#define WIFI_PASSWORD "M2-TiC@imsp2023"

// Insert Firebase project API Key
#define API_KEY "AIzaSyC-m7n5DNeaSMfrFu-7R4Snmj0axRFODPw"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "https://soilmoisture-65bd1-default-rtdb.europe-west1.firebasedatabase.app/" 


#define AOUT_PIN 35
#define BuzzerLed 13
#define yellowLed 14
#define greenLed 27

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;


void setup(){
  Serial.begin(115200);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
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
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  pinMode(BuzzerLed, OUTPUT);
}

void loop(){
  //Firebase.begin(&config, &auth);
  //Firebase.reconnectWiFi(true);
  int readValue = analogRead(AOUT_PIN);
  int value = (int)readValue;

  if (value >= 0  && value <= 300){
    digitalWrite(BuzzerLed, HIGH);
    delay(200);
    digitalWrite(BuzzerLed, LOW);
    delay(2000);
  }
  else{
    digitalWrite(BuzzerLed, LOW);
  }
  
  if (value > 300  && value <= 600){
    digitalWrite(yellowLed, HIGH);
  }
  else{
    digitalWrite(yellowLed, LOW);
  }
  if (value > 600){
    digitalWrite(greenLed, HIGH);
  }
  else{
    digitalWrite(greenLed, LOW);
  }

  if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    // Write an Int number on the database path test/int
    if (Firebase.RTDB.setInt(&fbdo, "Moisture/value", value)){
      Serial.println("PASSED " + String(value));
      Serial.println("PATH: " + fbdo.dataPath());
      Serial.println("TYPE: " + fbdo.dataType());
    }
    else {
      Serial.println("FAILED");
      Serial.println("REASON: " + fbdo.errorReason());
    }
  }
  delay(100);
}