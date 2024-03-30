#include <HTTPClient.h>
#include <ArduinoJson.h>

String msg;
String openWeatherApiKey = OPEN_WEATHER_API_KEY;
String url = "http://api.openweathermap.org/data/2.5/weather?q=Leipzig&units=metric&appid=" + String(openWeatherApiKey);

String weatherApiCall(){

  // Make HTTP request to OpenWeatherMap API
  
  HTTPClient http;
  
  http.begin(url);

  int httpCode = http.GET();
  
  if (httpCode > 0) {

    String payload = http.getString();  // Parse JSON response
    DynamicJsonDocument doc(1024);
    deserializeJson(doc, payload); 
    float temperature = doc["main"]["temp"];
    msg = "Temperature in Leipzig is: "+String(temperature);
        
  } 
  else {

    msg="Error fetching weather data";

  }
  http.end();
  
  return msg;
  
}


