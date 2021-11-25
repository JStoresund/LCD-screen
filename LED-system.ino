#include <FastLED.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>

#define LED_PIN 33
#define NUM_LEDS 20

float prosent;

//WIFI
const char* ssid = "Johans WiFi";
const char* password = "123456789";
const char* serverNameTemp = "http://192.168.4.1/";
unsigned long previousMillis = 0;
const long interval = 5000;


//LEDS
CRGB whiteLeds[NUM_LEDS];
CRGB gradientLeds[NUM_LEDS];
CRGB output[NUM_LEDS];

uint8_t colorIndex[NUM_LEDS];

DEFINE_GRADIENT_PALETTE (whiteLight_gp){
  0, 255, 249, 216
};

DEFINE_GRADIENT_PALETTE (npnLight_gp){
  0, 255, 69, 0,
  102, 255, 50, 0,
  153, 255, 0, 0,
  255, 255, 69, 0
};

CRGBPalette16 npnLight = npnLight_gp;
CRGBPalette16 whiteLight = whiteLight_gp;

//Blending Patterns
uint8_t patternCounter = 0;
uint8_t source1Pattern = 0;
uint8_t source2Pattern = 0;

void setup() {
  Serial.begin(9600);

  //Wifi
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  
  FastLED.addLeds<WS2812B, LED_PIN, GRB>(output, NUM_LEDS);

  //SuperWhite Setting
  FastLED.setCorrection(TypicalPixelString); //UncorrectedColor, TypicalLEDStrip, TypicalPixelString
  FastLED.setTemperature(Tungsten40W);
  
  for (int i = 0; i<NUM_LEDS; i++){
    colorIndex[i] = random8();
  }
}

//WiFi
String httpGETRequest(const char* serverName) {
  HTTPClient http;
  http.begin(serverName);
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  String payload = "--";
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
  return payload;
}

String info;
void getInfo(){
  unsigned long currentMillis = millis();
  if(currentMillis - previousMillis >= interval) {
    // Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED ){
      info = httpGETRequest(serverNameTemp);
      Serial.println("Info: " + info);
    
      // save the last HTTP GET Request
      previousMillis = currentMillis;
    }
    else {
      Serial.println("WiFi Disconnected");
    }
  }
}

int activeScene = 1;
int avalibleScenes = 3;
void changeScene(){
  activeScene++;
  if (activeScene > avalibleScenes){
    activeScene = 1;
  }
}

//How fast color change
void lightSpeed(int millisec){
  EVERY_N_MILLIS(millisec){
    for (int i=0; i<NUM_LEDS; i++){
      colorIndex[i]++;
    }
  } 
}

//Blend between to palettes
void gradientBlendLightMode() {
  for (int i=0; i<NUM_LEDS; i++){
      whiteLeds[i] = ColorFromPalette(whiteLight, colorIndex[i]);
    }

  for (int i=0; i<NUM_LEDS; i++){
      gradientLeds[i] = ColorFromPalette(npnLight, colorIndex[i]);
    }
  
  EVERY_N_MILLIS(50){
    blend(whiteLeds, gradientLeds, output, NUM_LEDS, prosent);
  }
}

//Calming lights from one palette
void gradientLightMode(CRGBPalette16 palette){
  for(int i=0; i<NUM_LEDS; i++){
    output[i] = ColorFromPalette(palette, colorIndex[i]);
  }
}

//Guide light from start to end
int num = 0;
void guideLightMode(){
  EVERY_N_MILLIS(150){
      fadeToBlackBy(output, NUM_LEDS, 70);
      output[num] = ColorFromPalette(npnLight, colorIndex[num]);
      
      num++;
      if (num == NUM_LEDS){
        num = 0;
      }
  }
}

//Guide light from end to start
int rNum = NUM_LEDS-1;
void guideLightModeReverse(){
  EVERY_N_MILLIS(150){
      fadeToBlackBy(output, NUM_LEDS, 70);
      output[rNum] = ColorFromPalette(npnLight, colorIndex[rNum]);
      
      rNum--;
      if (rNum < 0){
        rNum = NUM_LEDS-1;
      }
  }
}

void loop(){
  getInfo();
  prosent = 2.5*info.toFloat();
    
  if (prosent < 250){
    gradientBlendLightMode();
  }
  else if (prosent == 250){
    gradientLightMode(npnLight);
  }
  else if (prosent == 252){
    guideLightMode();
  }
  else if (prosent == 255){
    guideLightModeReverse();
  }
  
  lightSpeed(10);
  FastLED.setBrightness(200);
  FastLED.show();
}
