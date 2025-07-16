// AC & DC Current Sensor for ESP32
#include <Arduino.h>

int decimalPrecision = 2;
int currentAnalogInputPin = 34;  // GPIO34 (ADC1_CHANNEL_6) for current sensor (A1)
int calibrationPin = 32;         // GPIO32 (ADC1_CHANNEL_4) for calibration (A2)
float manualOffset = 10.15;
float mVperAmpValue = 62.5;
float supplyVoltage = 3300;      // ESP32 uses 3.3V (3300mV) supply voltage
float offsetSampleRead = 0;
float currentSampleRead = 0;
float currentLastSample = 0;
float currentSampleSum = 0;
float currentSampleCount = 0;
float currentMean;
float RMSCurrentMean;
float FinalRMSCurrent;

unsigned long previousMillis = 0;
const long interval = 100;  // Print every 100 milliseconds (0.1 second)

float smoothingFactor = 0.1;  // Smoothing factor for current readings
static float smoothedRMS = 0;

void setup() {
  Serial.begin(115200);  // Initialize serial communication with 115200 baud rate

  // Configure ADC characteristics
  analogReadResolution(12); // ESP32 default ADC resolution is 12 bits (0-4095)
  analogSetAttenuation(ADC_0db); // Default is 0dB, this gives a range of 0-3.3V
}

void loop() {
  if (micros() >= currentLastSample + 50) {  // Sample every 50 milliseconds (or faster)
    currentSampleRead = analogRead(currentAnalogInputPin) - analogRead(calibrationPin);  // Read sample with offset
    currentSampleSum += sq(currentSampleRead);  // Accumulate squared sample values
    currentSampleCount += 1;  // Increment sample count
    currentLastSample = micros();  // Reset time for the next cycle
  }

  if (currentSampleCount >= 500) {  // After 500 samples (this will be quicker than 8000)
    currentMean = currentSampleSum / currentSampleCount;  // Calculate average of accumulated samples
    RMSCurrentMean = sqrt(currentMean);  // Calculate RMS current
    FinalRMSCurrent = (((RMSCurrentMean / 4095) * supplyVoltage) / mVperAmpValue) - manualOffset;

    if (FinalRMSCurrent <= (625 / mVperAmpValue / 100)) {
      FinalRMSCurrent = 0;  // Set current to 0 if below threshold
    }

    // Apply smoothing to the current reading
    smoothedRMS = smoothingFactor * FinalRMSCurrent + (1 - smoothingFactor) * smoothedRMS;

    // Print timestamp and current to Serial Monitor
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {  // Print every 100ms
      previousMillis = currentMillis;
      Serial.print("Timestamp: ");
      Serial.print((currentMillis / 1000));  // Print time in seconds
      Serial.print(" s - The Current RMS value is: ");
      Serial.print(smoothedRMS, decimalPrecision);  // Print current with set decimal precision
      Serial.println(" A");

      // Reset for next cycle
      currentSampleSum = 0;
      currentSampleCount = 0;
    }
  }
}
