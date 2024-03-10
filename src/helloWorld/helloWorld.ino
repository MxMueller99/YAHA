#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#define BUTTON_PIN 33

#define TFT_CS     5
#define TFT_RST    4
#define TFT_DC     16
#define TFT_CLK    18
#define TFT_MOSI   23
#define TFT_BL     15

#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 280

volatile int currentScreen = 1; // Use volatile for variables accessed within ISR

// Create an instance of the Adafruit ST7789 library
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST);
bool screen1Active = true;


void setup() {
  // Initialize serial communication at a baud rate of 9600
  Serial.begin(115200);
<<<<<<< Updated upstream
  Serial.println("WiFi Status is:" + String(init_WiFi()));
  initWiFi();
  initNTP();
  displaySetup(); 
  initISR();
}

// ISR for button press
void IRAM_ATTR handleButtonPress() {
  currentScreen = !currentScreen; // Toggle screen
}

void initISR() {
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Initialize the button pin as input with pull-up resistor
  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), handleButtonPress, FALLING); // Attach the ISR

}

void loop() {
  if (currentScreen==1){
    displayScreen1();
  }
  else{
    displayScreen2();
  }
}

void displaySetup(){
  tft.init(SCREEN_WIDTH, SCREEN_HEIGHT);
  tft.fillScreen(ST77XX_BLACK); // Clear the screen with black color
  tft.setRotation(0);        // Set rotation
  
}

void setCursorWeather(){
  tft.setTextColor(ST77XX_YELLOW); // Set text color
  tft.setTextSize(2);          // Set text size
  // Set the cursor position where the text will start
  tft.setCursor(15, 70);
  //tft.print("Hello, World!"); // Print text to screen
}

void setCursorTime(){
   tft.setTextColor(ST77XX_WHITE); // Set text color
  tft.setTextSize(2);          // Set text size
  // Set the cursor position where the text will start
  tft.setCursor(15, 15);
  //tft.print("Hello, World!"); // Print text to screen
}

void displayScreen1() {

  tft.fillScreen(ST77XX_BLACK);
  setCursorTime();
  tft.println(getTime());
  setCursorWeather();
  tft.println(weatherApiCall());  
  
}

void displayScreen2() {
  
}