#include <TFT_eSPI.h> 
#include "BTC_coin.h"
#include "config.h"

#define BUTTON_PIN 33
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 240

// interrupt stuff
volatile int currentDisplay = 0; // track display number
volatile bool switchedDisplay = false; // track if screen was switched with button

// API timed stuff
unsigned long currentMillis;
unsigned long previousWthCall = 0; // will store last time the weather was updated
unsigned long previousBtcCall = 0; // stores previous time BTC API was called
const long intervalWthCall = 1000*60*30;  // interval at which to switch screens (ms)
const long intervalBtcCall = 1000*60*60;  // interval at which to call BTC API again (ms)

// display object according to user_setup.h of eSPI library
TFT_eSPI tft = TFT_eSPI();  

void setup() {
  // Initialize serial communication at a baud rate of 9600
  Serial.begin(115200);
  Serial.println("Setup start");
  initWiFi();
  initNTP();
  displaySetup(); 
  initISR();
  setupWeather_API();
  setupBTC_API();
  Serial.println("Setup complete");
}

// ISR for button press and display switch
void IRAM_ATTR handleButtonPress() {
  // Toggle screen
  if(currentDisplay == 1) {
    currentDisplay = 0;
  } else {
    currentDisplay += 1; 
  }
  
  switchedDisplay = true;
}

void initISR() {
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Initialize the button pin as input with pull-up resistor
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonPress, FALLING); // Attach the ISR
}

void loop() {
  Serial.println("Loop start");
  Serial.println("Screen is active: " + String(currentDisplay+1));
    // Call the function to display the current screen
    if (currentDisplay == 0) {
    displayScreen1();
  } else if (currentDisplay == 1) {
    displayScreen2();
  } else {
    displayScreen3();
  }

  delay(10);
  }


void displaySetup(){
  Serial.println("displaySetup start");
  tft.init();
  tft.fillScreen(TFT_BLACK); // Clear the screen with black color
  tft.setRotation(3);        // Set rotation
}

void setCursorWeather(){
  tft.setTextColor(TFT_YELLOW); // Set text color
  tft.setTextSize(1);          // Set text size
  // Set the cursor position where the text will start
  tft.setCursor(15, 120);
}

void setCursorBitcoin(){
  tft.setTextColor(TFT_BLUE); // Set text color
  tft.setTextSize(1);          // Set text size
  // Set the cursor position where the text will start
  tft.setCursor(110, 230);
}

void setCursorTime(){
   tft.setTextColor(TFT_WHITE); // Set text color
  tft.setTextSize(1);          // Set text size
  // Set the cursor position where the text will start
  tft.setCursor(15, 100);
}

// 1st display (Weater, Temperature, Time and Date)
void displayScreen1() {
  Serial.println("Displaying Screen 1");
  if(switchedDisplay){
    tft.fillScreen(TFT_BLACK);
    switchedDisplay = false;
  }
  currentMillis = millis();
  if (currentMillis - previousWthCall >= intervalWthCall) {
  previousWthCall = currentMillis;
  tft.fillScreen(TFT_BLACK);
  setCursorTime();
  tft.println(getTime(true));
  setCursorWeather();
  tft.println(weatherApiCall(true));  
  } else {
    setCursorTime();
    tft.println(getTime(false));
    setCursorWeather();
    tft.println(weatherApiCall(false));
  }
}

// 2nd display (Bitcoin Tracker)
void displayScreen2() {
  if(switchedDisplay){
    tft.fillScreen(TFT_BLACK);
    tft.pushImage(20, 20, 200, 200, BTC_scaled);
    switchedDisplay = false;
  }
  currentMillis = millis();
  Serial.println("Displaying Screen 2");
  if (currentMillis - previousBtcCall >= intervalBtcCall) {
  previousBtcCall = currentMillis;
  setCursorBitcoin();
  tft.println(callBTC_API(true));
  } else {
    setCursorBitcoin();
    tft.println(callBTC_API(false));
  }
  
}

// Template for 3rd display
void displayScreen3() {

}