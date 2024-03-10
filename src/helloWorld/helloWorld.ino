#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

#define TFT_CS     5
#define TFT_RST    4
#define TFT_DC     16
#define TFT_CLK    18
#define TFT_MOSI   23
#define TFT_BL     15

#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 280

// Create an instance of the Adafruit ST7789 library
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST);


void setup() {
  // Initialize serial communication at a baud rate of 9600
  Serial.begin(115200);
  Serial.println("WiFi Status is:" + String(init_WiFi()));
  display_setup();
  
}

void loop() {
  // Print "Hello, world!" to the serial monitor
  //Serial.println("Time: ");
  //Serial.println(get_WiFi_status());
  setCursorTime();
  tft.println(get_time());
  setCursorWeather();
  tft.println(weather_API_call());  
  
  // Delay for 1 second
  delay(10000);
  tft.fillScreen(ST77XX_BLACK);
  
}

void display_setup(){
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
