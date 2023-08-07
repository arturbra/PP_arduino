const int hallSensorPin = 36; // Connect your sensor to digital pin 36
unsigned long rain = 0; // Counter for magnet passes
bool lastSensorState = HIGH; // Used to store the previous sensor state



int sumSensorValues = 0; // Variable to hold the sum of sensor values
int numReadings = 0; // Variable to count the number of readings

void setup() {
  Serial.begin(115200); // Start the serial communication with the baud rate of 9600
  pinMode(hallSensorPin, INPUT_PULLUP); // Set the Hall effect sensor pin as input
}

void loop() {
  precipitation();
}

void precipitation() {
  unsigned long currentRainfallMillis = millis(); // Get the current time

  // If 100ms have passed since the last reading
  if (currentRainfallMillis - lastRainfallMillis >= debounceTime) {
    int averageSensorValue = 0; // Default value
    
    if (numReadings != 0) {
      averageSensorValue = sumSensorValues / numReadings;
    }

    // If the average sensor value is greater than zero, consider the state as HIGH, else consider it as LOW
    bool currentSensorState = (averageSensorValue > 0) ? HIGH : LOW;

    // If sensor state changes from HIGH to LOW, consider it a magnet pass
    if (lastSensorState == HIGH && currentSensorState == LOW) {
      rain++;
      Serial.println("Magnet Detected");
      Serial.print("Total Magnet Passes: ");
      Serial.println(rain);
    }

    // Update the lastSensorState and lastMillis for the next iteration
    lastSensorState = currentSensorState;
    lastRainfallMillis = currentRainfallMillis;

    // Reset the sum and the count
    sumSensorValues = 0;
    numReadings = 0;
  }
  else {
    // Add the current sensor value to the sum
    sumSensorValues += analogRead(hallSensorPin);

    // Increment the number of readings
    numReadings++;
  }
}
