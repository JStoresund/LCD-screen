//SCL kobles til D22
//SDA kobles til D21
#include <LiquidCrystal_I2C.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include "OneButton.h"

//OneButton
OneButton button(25, true);

// set the LCD number of columns and rows
int lcdColumns = 16;
int lcdRows = 2;
// set LCD address, number of columns and rows
LiquidCrystal_I2C lcd(0x27, lcdColumns, lcdRows);
int Auto = 33;

bool timer = false;
int Traffic_Light = 1; //dette var en bool, men jeg trengte flere enn 2 mulige verdier
int minutes = 0;
int sec = 15;         //husk å endre disse nederst også!!!
float prosent = 0;
int Ant_publikum = 0;
int Publikum_total = 14;
//SKRIV INN ANTALL FORVENTEDE PUBLIKUM HER

byte hjerte[8] = {
  0b00000,
  0b01010,
  0b11111,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
  0b00000
};
byte O[8] = {
  0b00001,
  0b01110,
  0b10011,
  0b10101,
  0b10101,
  0b11001,
  0b01110,
  0b10000
};

//Wifi
const char* ssid = "Johans WiFi";
const char* password = "123456789";
AsyncWebServer server(80);
String Publikum_func() {
  return String(prosent/10);
}

//OneButton

void setup() { 
  // initialize LCD
  lcd.init();
  // turn on LCD backlight
  lcd.backlight();
  pinMode(27, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(Auto, INPUT_PULLUP);

  lcd.createChar(0, hjerte);
  lcd.createChar(1, O);

  //Wifi ting:
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial.println();
  // Setting the ESP as an access point
  Serial.print("Setting AP (Access Point)...");
  // Remove the password parameter, if you want the AP (AccessPoint) to be open
  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print(WiFi.softAPIP());
  Serial.print("AP IP address: ");
  Serial.println(IP);
  Serial.print(WiFi.softAPIP());

  server.begin();
  server.on("/", [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", Publikum_func());
  });

  //OneButton
  button.attachDoubleClick(doubleclick);
  button.attachClick(singleclick);
  button.attachLongPressStop(longclick);
}

void loop() {
  //OneButton
  button.tick();
  delay(10);
  if (timer == false and Traffic_Light == 1) {
      if (prosent < 1000) {
      //Formatisering
      prosent = Ant_publikum * 1000 / Publikum_total;
      lcd.setCursor(7, 0);
      if (Ant_publikum < 10) {
        lcd.print("0");
      }
      lcd.print(Ant_publikum);
      lcd.setCursor(6, 1);
      if (prosent < 100) {
        lcd.print("0");
      }
      lcd.print(prosent / 10);
      if (prosent < 1000) {
        lcd.setCursor(10, 1);
        lcd.print(" %");
      }
      else {
        lcd.setCursor(9, 1);
        lcd.print("%  ");
      }
    }
  Traffic_Light = 0;
  }
  //Timer
  else if (timer == true) {
    if (minutes >= 0 and Traffic_Light == 0) {
      lcd.setCursor(6, 0);
      lcd.print(minutes);
      lcd.print(":");
      if (sec < 10) {
        lcd.print("0");
      }
      lcd.print(sec);
      lcd.setCursor(6, 0);
      if (sec == 0) {
        minutes--;
        sec = 60;
      } 
      sec--;
      delay(1000);
      if (minutes == 0 and sec == 0) {
        Traffic_Light = 2;       
      }
    }
    else if (Traffic_Light == 2) {
      lcd.clear();
      lcd.setCursor(6, 0);
      lcd.print("F");
      lcd.write((byte)1);
      lcd.print("LG");
      lcd.setCursor(5, 1);
      lcd.print("LYSENE");
      prosent = 1020;
      server.on("/", [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", Publikum_func());
        delay(1000);
      });
      Traffic_Light = 4;
      //hindrer at esp kontinuerlig oppdaterer webserver
    }
    else if (Traffic_Light == 5) {
      lcd.clear();
      lcd.setCursor(4, 0);
      lcd.print("Velkommen");
      lcd.setCursor(4, 1);
      lcd.print("Tilbake");
      lcd.setCursor(12, 1);
      lcd.write((byte)0);
      Traffic_Light = 3;
    }
  }
}

//Alt utover er OneButton
void doubleclick() {
  if (timer == false and Ant_publikum != 0) {                          
    digitalWrite(27,HIGH);                               
    delay(500);   
    Ant_publikum--; 
    prosent--;
    server.on("/", [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", Publikum_func());
    });        
    Traffic_Light = 1;                             
    digitalWrite(27,LOW);
  } 
}
 
void singleclick(){     
  if (timer == false and prosent != 1000) {                     
    digitalWrite(14, HIGH);               
    delay(350);          
    Ant_publikum++; 
    server.on("/", [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", Publikum_func());
    });                
    Traffic_Light = 1;           
    digitalWrite(14, LOW);
  }                           
}
 
void longclick(){  
  if (timer == false and prosent != 1000) {
    if (digitalRead(Auto) == LOW) {
      digitalWrite(12,HIGH);  
      delay(1000);   
      Ant_publikum = Publikum_total; 
      server.on("/", [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", Publikum_func());
      });                   
      Traffic_Light = 1;                 
      digitalWrite(12,LOW);
    }           
  }
  else if (timer == false and prosent == 1000) {
    if (digitalRead(Auto) == LOW) {
      timer = true;
    lcd.clear();
    }
  }     
  else if (Traffic_Light == 3) {
    digitalWrite(12,HIGH);  
    delay(1000); 
    Ant_publikum = 0; 
    prosent = 0;
    Traffic_Light = 1;
    timer = false;
    minutes = 0;
    sec = 15;
    lcd.clear();
    server.on("/", [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", Publikum_func());
    });  
    digitalWrite(12,LOW);
  }
  else if (Traffic_Light == 4 and digitalRead(Auto) == LOW) {
    prosent = 1080;
    server.on("/", [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", Publikum_func());
    });  
    Traffic_Light = 5;
  }
}
