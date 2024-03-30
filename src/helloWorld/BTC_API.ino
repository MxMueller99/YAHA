#include <HTTPClient.h>
#include <ArduinoJson.h>

float BTCtoEUR;

void setupBTC_API() {
  callBTC_API(true);
}

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