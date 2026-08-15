/*
  Health Tracker
  ESP32 + MAX30102 (heart rate/SpO2) + MPU6050 (accelerometer/gyro)
*/

#include <Wire.h>
#include "MAX30105.h"          // SparkFun library -> also covers MAX30102
#include "heartRate.h"         // SparkFun beat-detection algorithm
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

MAX30105 particleSensor;
Adafruit_MPU6050 mpu;

// simple step-detection state
float lastMagnitude = 0;
float threshold = 1.5;     
unsigned long lastStepTime = 0;
unsigned long stepCount = 0;

// heart rate calculation 
const byte RATE_SIZE = 4;  
byte rates[RATE_SIZE];
byte rateSpot = 0;
long lastBeat = 0;          // timestamp of last detected beat
float beatsPerMinute;
int beatAvg;
long irBaseline = 50000;    // below this no finger/skin contact

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(10);

  Wire.begin(21, 22); // SDA, SCL

  //  MAX30102 setup 
  if (!particleSensor.begin(Wire, I2C_SPEED_FAST)) {
    Serial.println("MAX30102 not found. Check wiring.");
  } else {
    particleSensor.setup();             // default settings: LED brightness, sample rate, pulse width
    particleSensor.setPulseAmplitudeRed(0x0A);
    particleSensor.setPulseAmplitudeGreen(0);
    Serial.println("MAX30102 initialized.");
  }

  //  MPU6050 setup 
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
  //  Heart rate sensor raw reading 
  long irValue = particleSensor.getIR();
  long redValue = particleSensor.getRed();

  //  Beat detection 
  if (checkForBeat(irValue) == true) {
    long delta = millis() - lastBeat;
    lastBeat = millis();

    beatsPerMinute = 60 / (delta / 1000.0);

    if (beatsPerMinute < 255 && beatsPerMinute > 20) {
      rates[rateSpot++] = (byte)beatsPerMinute;
      rateSpot %= RATE_SIZE;

      // average the last few readings for a stabler number
      beatAvg = 0;
      for (byte x = 0; x < RATE_SIZE; x++) beatAvg += rates[x];
      beatAvg /= RATE_SIZE;
    }
  }

  bool fingerDetected = irValue > irBaseline;

  //  Accelerometer/gyro reading 
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  // magnitude of acceleration vector, minus gravity baseline 
  float magnitude = sqrt(a.acceleration.x * a.acceleration.x +
                          a.acceleration.y * a.acceleration.y +
                          a.acceleration.z * a.acceleration.z);
  float delta = magnitude - 9.8;

  // peak-detection step counter (placeholder - needs tuning)
  unsigned long now = millis();
  if (abs(delta) > threshold && (now - lastStepTime) > 300) {
    stepCount++;
    lastStepTime = now;
  }


  Serial.print("IR: ");
  Serial.print(irValue);
  Serial.print(fingerDetected ? " (contact)" : " (no contact)");
  Serial.print(" | BPM: ");
  Serial.print(beatsPerMinute);
  Serial.print(" | AvgBPM: ");
  Serial.print(beatAvg);
  Serial.print(" | AccelMag: ");
  Serial.print(magnitude, 2);
  Serial.print(" | Steps: ");
  Serial.println(stepCount);

  delay(20); 
}
