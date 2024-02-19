#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <TridentTD_LineNotify.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define SSID        "Bin1"
#define PASSWORD    "12345678"
#define LINE_TOKEN  "HrmkzTI4mgOEpVH3eNLK41cEn1v7yZ61f5Fi4oGcqp7"
#define MQTT_SERVER "broker.hivemq.com"
const char *topic = "Test";
#define MQTT_PORT   1883

// Define the I2C address for the LCD
LiquidCrystal_I2C lcd(0x27, 16, 2);

WiFiClient espClient;
PubSubClient client(espClient);

// Define IR sensor pins for each side
const int leftIrSensorPins[] = {2, 0, 4, 16};   // Pins for the left side sensors
const int rightIrSensorPins[] = {17, 5, 18, 19}; // Pins for the right side sensors

void setup() {
  Serial.begin(9600);

  Serial.println();
  Serial.println(LINE.getVersion());

  WiFi.begin(SSID, PASSWORD);
  LINE.setToken(LINE_TOKEN);

  Serial.printf("WiFi connecting to %s\n", SSID);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(400);
  }
  client.setServer(MQTT_SERVER, MQTT_PORT);

  Serial.printf("\nWiFi connected\nIP : ");
  Serial.println(WiFi.localIP());
  // Initialize LCD
  lcd.begin();
  lcd.setCursor(0, 0);
  lcd.backlight();

  // Initialize IR sensors as inputs
  for (int i = 0; i < 4; i++) {
    pinMode(leftIrSensorPins[i], INPUT);
    pinMode(rightIrSensorPins[i], INPUT);
    digitalWrite(leftIrSensorPins[i], LOW);
    digitalWrite(rightIrSensorPins[i], LOW);
  }
}

void loop() {
  // Read values from IR sensors on the left side
  int leftSensorValues[4];
  int leftSum = 0;
  for (int i = 0; i < 4; i++) {
    leftSensorValues[i] = digitalRead(leftIrSensorPins[i]);
    leftSum += leftSensorValues[i];
  }
  int leftAverageValue = (leftSum * 100) / 4; // Calculate average as a percentage

  // Read values from IR sensors on the right side
  int rightSensorValues[4];
  int rightSum = 0;
  for (int i = 0; i < 4; i++) {
    rightSensorValues[i] = digitalRead(rightIrSensorPins[i]);
    rightSum += rightSensorValues[i];
  }
  int rightAverageValue = (rightSum * 100) / 4; // Calculate average as a percentage

  // Calculate overall average
  int overallAverage = (leftAverageValue + rightAverageValue) / 2;

   // Invert the overallAverage value to represent it as 0% when full and 100% when not full
  overallAverage = 100 - overallAverage;

  // Display overall average on LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Status: ");
  lcd.print(overallAverage);
  lcd.print("%");
  

  // Check if overall average is 0%, if yes, send notification
  if (overallAverage == 100) {
    // Send notification to LINE Notify
    //TridentTD_LineNotify::send(LINE_TOKEN, "Trash Full");
    LINE.notify("Trash Full");
    delay(1000);
  }
  if (client.connected()) {
    String message = String(overallAverage); // Convert the value to a string
    client.publish(topic, message.c_str()); // Publish the message
  }

  delay(500); // Update the display every 1 second (adjust as needed)
}