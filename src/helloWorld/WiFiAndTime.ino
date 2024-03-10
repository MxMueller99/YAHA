#include <WiFi.h>
#include "time.h"

// WiFi Credentials
// ToDo Add Environment Variable as preprocessor
const char* ssid = "ssid";
const char* password = "pass";

// NTP Server
const char* ntp_server = "pool.ntp.org";

// Time zone offset
const long gmt_offset_sec = 1 * 3600;
const int   daylight_offfset_sec = 3600;


// Initialize WiFi connection
int init_WiFi() {
  WiFi.begin(ssid, password);
  int counter=0;
  while (WiFi.status() != WL_CONNECTED) {
    if(counter>=10){
      return -1;
    }
    delay(1000);
    counter ++;
  }
  return 0;
}

int get_WiFi_status() {
  if(WiFi.status() == WL_CONNECTED) {
    return 0;
  }
  else {
    return -1;
  }
}


int init_NTP() {
  configTime(gmt_offset_sec, daylight_offfset_sec, ntp_server);
}

String get_time(){
  struct tm timeinfo;

  // Check if time is available
  if (!getLocalTime(&timeinfo, 5000)) { 
    return "Error fetching time data";
  }
  
  // Buffer to hold the formatted time
  char buffer[64];
  strftime(buffer, sizeof(buffer), "%A, %B %d %Y %H:%M:%S", &timeinfo);

  return String(buffer);
}




