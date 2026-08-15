# StayFit

A basic ESP32 sketch that reads heart rate (MAX30102) and motion/steps (MPU6050) over I2C and prints the results to the Serial Monitor.

## Hardware
- ESP32 
- MAX30102 heart rate/pulse oximeter sensor
- MPU6050 accelerometer/gyroscope


## What it does
- Reads raw IR/Red light values from the MAX30102 and calculates heart rate (BPM) using peak/beat detection, with a rolling average to smooth out noise
- Flags whether the sensor is making skin contact (based on IR signal strength)
- Reads acceleration from the MPU6050 and does basic step detection using a threshold on acceleration magnitude
- Prints IR value, contact status, BPM, average BPM, acceleration magnitude, and step count to Serial every ~20ms


## Tuning notes
- **BPM readings noisy or wrong**: make sure the MAX30102 is pressed firmly against skin. Give it 5–10 seconds after contact to stabilize.
- **Step count inaccurate**: adjust the `threshold` value in the code based on real accelerometer data while walking — current value is a placeholder.


## Working On
- BLE syncing to a phone
- OLED display output
- Battery/power management
- Enclosure/strap design

## Files

- `wrist_tracker_starter.ino` — main sketch
