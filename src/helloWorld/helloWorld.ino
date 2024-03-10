void setup() {
  // Initialize serial communication at a baud rate of 9600
  Serial.begin(9600);
}

void loop() {
  // Print "Hello, world!" to the serial monitor
  Serial.println("Hello, world!");
  
  // Delay for 1 second
  delay(10000);
}
