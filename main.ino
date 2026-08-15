/*
  Wrist Tracker - Starter Code
  ESP32 + MAX30102 (heart rate/SpO2) + MPU6050 (accelerometer/gyro)

  Wiring (ESP32 DevKitC default I2C pins):
    SDA -> GPIO21
    SCL -> GPIO22
    VCC -> 3.3V (both sensors)
    GND -> GND (both sensors)

  Required libraries (install via Arduino Library Manager):
    - "SparkFun MAX3010x Pulse and Proximity Sensor Library" (for MAX30102)
    - "Adafruit MPU6050" + "Adafruit Unified Sensor" (for MPU6050)
*/

#include <Wire.h>
#include "MAX30105.h"          // SparkFun library, also covers MAX30102
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

MAX30105 particleSensor;
Adafruit_MPU6050 mpu;

// simple step-detection state
float lastMagnitude = 0;
float threshold = 1.5;      // tune this based on real data
unsigned long lastStepTime = 0;
unsigned long stepCount = 0;

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Wire.begin(21, 22); // SDA, SCL

  // --- MAX30102 setup ---
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 not found. Check wiring.");
  } else {
    particleSensor.setup(); // default settings: LED brightness, sample rate, pulse width
    particleSensor.setPulseAmplitudeRed(0x0A);
    particleSensor.setPulseAmplitudeGreen(0);
    Serial.println("MAX30102 initialized.");
  }

  // --- MPU6050 setup ---
  if (!mpu.begin()) {
    Serial.println("MPU6050 not found. Check wiring.");
  } else {
    mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    Serial.println("MPU6050 initialized.");
  }

  Serial.println("Setup complete. Starting readings...");
}

void loop() {
  // --- Heart rate sensor raw reading ---
  long irValue = particleSensor.getIR();
  long redValue = particleSensor.getRed();

  // --- Accelerometer/gyro reading ---
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // magnitude of acceleration vector, minus gravity baseline (~9.8)
  float magnitude = sqrt(a.acceleration.x * a.acceleration.x +
                          a.acceleration.y * a.acceleration.y +
                          a.acceleration.z * a.acceleration.z);
  float delta = magnitude - 9.8;

  // very basic peak-detection step counter (placeholder - needs tuning)
  unsigned long now = millis();
  if (abs(delta) > threshold && (now - lastStepTime) > 300) {
    stepCount++;
    lastStepTime = now;
  }

  // --- Print everything ---
  Serial.print("IR: ");
  Serial.print(irValue);
  Serial.print(" | Red: ");
  Serial.print(redValue);
  Serial.print(" | AccelMag: ");
  Serial.print(magnitude, 2);
  Serial.print(" | Steps: ");
  Serial.println(stepCount);

  delay(50); // ~20Hz loop
}
