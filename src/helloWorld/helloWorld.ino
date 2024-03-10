

void setup() {
  // Initialize serial communication at a baud rate of 9600
  Serial.begin(9600);
  Serial.println("WiFi Status is:" + String(init_WiFi()));
}

void loop() {
  // Print "Hello, world!" to the serial monitor
  //Serial.println("Time: ");
  //Serial.println(get_WiFi_status());
  Serial.println(weather_API_call());
  
  // Delay for 1 second
  delay(1000);
}
