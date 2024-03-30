#include <HTTPClient.h>
#include <ArduinoJson.h>

// OpenWeather definitions
String openWeatherApiKey = OPEN_WEATHER_API_KEY;
String url = "http://api.openweathermap.org/data/2.5/weather?q=Leipzig&units=metric&appid=" + String(openWeatherApiKey);
String weatherMsg;

// Bitcoin definitions
float BTCtoEUR;


// Bitcoin API Setup On Startup
void setupWeather_API() {
  weatherApiCall(true);
}

// Make HTTP request to OpenWeatherMap API
String weatherApiCall(bool requestAPI){


  if(requestAPI) {
    HTTPClient http;
    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {

      String payload = http.getString();  // Parse JSON response
      DynamicJsonDocument doc(1024);
      deserializeJson(doc, payload); 
      float temperature = doc["main"]["temp"];
      weatherMsg = "Temperature in Leipzig is: " + String(temperature) + "°";
        
    } 
    else {

      weatherMsg = "Error fetching weather data";

    }
    http.end(); // Free
  }

  return weatherMsg;
}


// Bitcoin API Setup On Startup
void setupBTC_API() {
  callBTC_API(true);
}

// Bitcoin API Timed Call
float callBTC_API(bool requestAPI) {

  if (requestAPI) {
    HTTPClient http;
    http.begin("https://api.coingecko.com/api/v3/simple/price?ids=bitcoin&vs_currencies=eur"); // CoinGecko API URL
    int httpCode = http.GET();

    if (httpCode > 0) { //Check for the returning code
        const size_t capacity = JSON_OBJECT_SIZE(3) + 60;
        DynamicJsonDocument doc(capacity);

        deserializeJson(doc, http.getString());

        BTCtoEUR = doc["bitcoin"]["eur"].as<float>();
    }

    http.end(); //Free the resources
  }
  return BTCtoEUR;
}

