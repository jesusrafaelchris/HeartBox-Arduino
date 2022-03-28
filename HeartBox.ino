#include <EEPROM.h>
#include <Arduino.h>

#include <Servo.h>

#if !( defined(ESP8266) )
  #error This code is intended to run on ESP8266 platform! Please check your Tools->Board setting.
#endif
#define _WIFIMGR_LOGLEVEL_    4  
#include <ESP_WiFiManager.h>              
#include <ESP_WiFiManager-Impl.h>    
#if defined(ESP32)
#include <WiFi.h>
#include <FirebaseESP32.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#endif

#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>
#define API_KEY "AIzaSyBbp3BryR_QX4f_8glb3guvEI02F6rp2EQ"
#define DATABASE_URL "heartboxarduino-default-rtdb.europe-west1.firebasedatabase.app" 
#define USER_EMAIL "test@gmail.com"
#define USER_PASSWORD "password"

#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>

Adafruit_ILI9341 tft = Adafruit_ILI9341 (D2, D4, D3);


//Define Firebase Data object
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
unsigned long sendDataPrevMillis = 0;


int servo_pin = D7;
Servo myservo;
int angle = 0;  
char message[6];
long randNumber;
byte Target;
String code = "";

void setup() {

  printToTFT("Starting up",3,160,120 + 5);
  delay(1000);
  tft.fillScreen(ILI9341_BLACK);
  yield();

  
  EEPROM.begin(10);
  myservo.attach(servo_pin);
  Serial.begin(115200); while (!Serial); delay(200);
  Serial.print(F("\nStarting AutoConnect_ESP8266_minimal on ")); Serial.println(ARDUINO_BOARD); 
  Serial.println(ESP_WIFIMANAGER_VERSION);
  ESP_WiFiManager ESP_wifiManager("LoveBox");
  ESP_wifiManager.autoConnect("LoveBox");
  if (WiFi.status() == WL_CONNECTED) { 
    Serial.print(F("Connected. Local IP: ")); Serial.println(WiFi.localIP()); 
      printToTFT("Connected to Wifi",2,160,120+8);
      //printToTFT(String(WiFi.localIP()),2,160,120);
    }
  else { 
    Serial.println(ESP_wifiManager.getStatus(WiFi.status())); 
    //printToTFT(ESP_wifiManager.getStatus(WiFi.status()),4,0,0);
    }

  Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);

  //Comment or pass false value when WiFi reconnection will control by your code or third party library
  Firebase.reconnectWiFi(true);

  Firebase.setDoubleDigits(5);

//  Serial.println( "Read Target from EEPROM");
//  EEPROM.get(0, Target);
//
//  Serial.print(F("Read value of Target is "));
//  Serial.println (Target);
  
//  if (Target == 99) {
//    Serial.println("EEPROM empty");
//    Serial.println("Updating EEPROM");
//    randomSeed(analogRead(0));
//    random_6_letter_code();
//    writeWord(String(message));
//    Target += 1;
//  }

//   Serial.println("New Value in EEPROM is");
//   EEPROM.get(0, Target);
//   code = readWord();
//   Serial.println(code);
   
   //display code to board
   tft.begin();
   tft.setRotation(3);  
   tft.fillScreen(ILI9341_BLACK);
   yield();
   tft.setCursor(0, 0);
   tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(3);
   tft.println("Your Box Code:");
   //tft.println(code);
   tft.println("itinnb");

  if (!Firebase.beginStream(fbdo, "/test/itinnb")) {
    Serial.println(fbdo.errorReason());
  }

}


void loop() {

  if (Firebase.ready() && (millis() - sendDataPrevMillis > 15000 || sendDataPrevMillis == 0)) {
    sendDataPrevMillis = millis();
    
  if (!Firebase.readStream(fbdo)) {
    Serial.println(fbdo.errorReason());
  }
  
  if (fbdo.streamTimeout()) {
    Serial.println("Stream timeout, resume streaming...");
    Serial.println();
  }
  
  if (fbdo.streamAvailable()) {

    //check if lid is open
    
    //while lid closed:
      //flash leds
      //spin motor around every minute
      
    //else:
      //show text on screen
      
    
    
    String data = fbdo.stringData();
    String datastr = String(data);
    //myString.remove(index,count)
    datastr.remove(0, 1);
    datastr.remove(datastr.length()-1, 1);
    Serial.println(datastr.c_str());
    //remove quotation marks

    move_Servo();
    printToTFT(datastr,2,160 - ((datastr.length() / 2) * 10 - 20),120);
    
//    for (int i;i<=26;i++) {
//      int spacing;
//        if (i == 0) {
//          spacing = 0;
//          printToTFT(datastr,2,spacing,120);
//        }
//      else if (datastr.length() == i) {
//           spacing = 160 - (abs(26 - i) + 10);
//           printToTFT(datastr,2,spacing,120);
//          }
//        }
     }
  }
  
}

void printToTFT(String text, int sizeOfFont, int x, int y) {
     
   tft.begin();
   tft.setRotation(3);  
   tft.fillScreen(ILI9341_BLACK);
   yield();
   tft.setCursor(x, y);
   tft.setTextColor(ILI9341_WHITE);  tft.setTextSize(sizeOfFont);
   tft.println(text);

}

void writeWord(String word) {
  delay(10);

  for (int i = 0; i < word.length(); ++i) {
    Serial.println(word[i]);
    EEPROM.write(i, word[i]);
  }

  EEPROM.write(word.length(), '\0');
  EEPROM.commit();
}

String readWord() {
  String word;
  char readChar;
  int i = 0;

  while (readChar != '\0') {
    readChar = char(EEPROM.read(i));
    delay(10);
    i++;

    if (readChar != '\0') {
      word += readChar;
    }
  }

  return word;
}

void random_6_letter_code() {
  int generated=0;
  
  while(generated<6){
     randNumber = random(26);
     char letter = randNumber + 'a';
     message[generated] = letter;
     generated ++;
  }

  Serial.println(message);
  
}

void move_Servo() {
  
    for(angle = 0; angle < 180; angle += 5) {                          
    myservo.write(angle);
    delay(15);                       
  } 

  delay(1000);
  
  // move from 180 to 0 degrees with a negative angle of 5
  for(angle = 180; angle>=1; angle-=5) {                                
    myservo.write(angle);
    delay(5);                       
  } 
}
