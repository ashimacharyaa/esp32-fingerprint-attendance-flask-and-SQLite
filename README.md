# esp32-fingerprint-attendance-flask-and-SQLite
ESP32 Fingerprint Attendance System with Flask Dashboard and SQLite- IoT
# ESP32 Fingerprint Attendance System

A simple fingerprint-based attendance system using:

- ESP32
- Fingerprint sensor
- 16x2 I2C LCD
- Wi-Fi
- Flask
- SQLite
- Web dashboard

The ESP32 reads a fingerprint ID, sends it through Wi-Fi to a Flask server, stores attendance in SQLite, and shows the result on a web dashboard.

---

## Features

- Fingerprint enrollment
- Fingerprint recognition
- Employee ID and name mapping
- Real-time attendance logging
- SQLite database
- Flask web dashboard
- Employee management page
- CSV export
- LCD status display
- Wi-Fi communication between ESP32 and server

---

## System Architecture

```text
Fingerprint Sensor
        |
        v
      ESP32
        |
      Wi-Fi
        |
        v
    Flask API
        |
        v
      SQLite
        |
        v
  Web Dashboard
