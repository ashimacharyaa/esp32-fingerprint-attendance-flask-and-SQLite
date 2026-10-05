# esp32-fingerprint-attendance-flask-and-SQLite

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
<img width="1600" height="1200" alt="flask image" src="https://github.com/user-attachments/assets/8cac1859-9c63-45f0-9c8c-777b34fe4e83" />

<img width="1440" height="1080" alt="terrible design" src="https://github.com/user-attachments/assets/67da30c0-003d-408b-895f-d297c1a9ec18" />
