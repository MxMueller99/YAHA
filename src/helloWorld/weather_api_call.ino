#include <HTTPClient.h>
#include <ArduinoJson.h>

String weatherApiCall(){

  // Make HTTP request to OpenWeatherMap API
  String apiKey = "cae3530cfe3f8667a7e17679eace2fce";
  HTTPClient http;
  String url = "http://api.openweathermap.org/data/2.5/weather?q=Leipzig&units=metric&appid=" + String(apiKey);
  http.begin(url);

  delay(1000);

  int httpCode = http.GET();
  String msg;
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


