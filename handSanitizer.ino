#include "RTClib.h"
#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Servo.h>

Servo sanitizerServo;  // Create a Servo object
int servoPin = 9;      // Define the servo control pin

const int trigPin = 3; // Define the trigger pin of the ultrasonic sensor
const int echoPin = 4; // Define the echo pin of the ultrasonic sensor

long duration;
int distance;

RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

const int chipSelect = 10;
File myFile;

void setup() {
  sanitizerServo.attach(servoPin);  // Attaches the servo on pin 9
  pinMode(trigPin, OUTPUT); // Set the trigger pin as an OUTPUT
  pinMode(echoPin, INPUT);  // Set the echo pin as an INPUT
  Serial.begin(115200);  // Initialize serial communication for debugging

  Wire.begin();
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  Serial.print("Initializing SD card...");
  if (!SD.begin(chipSelect)) {
    Serial.println("Initialization failed!");
    return;
  }
  Serial.println("Initialization done.");

  // Open file
  myFile = SD.open("DATALOG.txt", FILE_WRITE);

  if (myFile) {
    Serial.println("File opened OK");
    // Print the headings for our data
    myFile.println("Date,Time,LPG,Alcohol");
    myFile.close();
  } else {
    Serial.println("Error opening file");
  }
  
}

void loop() {
  DateTime now = rtc.now();
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  // Read the duration of the echo pulse
  duration = pulseIn(echoPin, HIGH);
  
  // Calculate the distance in centimeters
  distance = duration * 0.0343 / 2; // Speed of sound is 343 m/s or 0.0343 cm/µs
  
  // Print the distance for debugging
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" cm");
  logData(now, distance);
  printData(now, distance);
  checkServo();
  delay(100);
}

void checkServo(){
    // Check if an object is within a certain range (adjust the threshold as needed)
  if (distance < 20 && distance > 0) {
    // Move the servo to the desired position (e.g., 90 degrees)
    sanitizerServo.write(90);
    delay(1500);  // Delay to allow time for sanitizer dispensing
    // Return the servo to its initial position (0 degrees)
    sanitizerServo.write(0);
    delay(1000);  // Delay before checking again
  }
  else {
    // If no object is detected or the sensor gives a reading of 0 (indicating an issue), keep the servo at its initial position
    sanitizerServo.write(0);
  }
}



void logData(DateTime timestamp, int distance) {
  // Open the data file for appending
  myFile = SD.open("DATALOG.txt", FILE_WRITE);

  if (myFile) {
    myFile.print(timestamp.year(), DEC);
    myFile.print('/');
    myFile.print(timestamp.month(), DEC);
    myFile.print('/');
    myFile.print(timestamp.day(), DEC);
    myFile.print(' ');
    myFile.print(timestamp.hour(), DEC);
    myFile.print(':');
    myFile.print(timestamp.minute(), DEC);
    myFile.print(':');
    myFile.print(timestamp.second(), DEC);
    myFile.print(", ");
    myFile.println(distance);
    myFile.close();
  } else {
    Serial.println("Error opening file for writing");
  }
}

void printData(DateTime timestamp, int distance) {
  Serial.print("Concentration Units (standard)");
  Serial.print("Distance (CM): ");
  Serial.println(distance);
  Serial.println("---------------------------------------");
  Serial.print("Date: ");
  Serial.print(timestamp.year(), DEC);
  Serial.print('/');
  Serial.print(timestamp.month(), DEC);
  Serial.print('/');
  Serial.print(timestamp.day(), DEC);
  Serial.print(" Time: ");
  Serial.print(timestamp.hour(), DEC);
  Serial.print(':');
  Serial.print(timestamp.minute(), DEC);
  Serial.print(':');
  Serial.print(timestamp.second(), DEC);
  Serial.print(" Distance data logged.");
  Serial.println("---------------------------------------");
}
