#include <WiFi.h>
#include <HTTPClient.h>

#include <Adafruit_Fingerprint.h>

#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>


// =====================================================
// WIFI SETTINGS
// =====================================================

const char* WIFI_NAME = "homnath25_2";
const char* WIFI_PASSWORD = "HSAA@2024";


// Laptop Flask server
const char* SERVER_URL =
  "http://192.168.1.66:5000/api/attendance";


// =====================================================
// FINGERPRINT SENSOR
// =====================================================

HardwareSerial mySerial(2);

Adafruit_Fingerprint finger(&mySerial);


// =====================================================
// LCD
// =====================================================

hd44780_I2Cexp lcd;


// =====================================================
// SETUP
// =====================================================

void setup() {

  Serial.begin(115200);

  delay(500);


  // -------------------------------
  // LCD
  // -------------------------------

  int lcdStatus = lcd.begin(16, 2);

  if (lcdStatus == 0) {

    lcd.backlight();

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Attendance");

    lcd.setCursor(0, 1);
    lcd.print("Starting...");

  } else {

    Serial.println("LCD initialization failed");

  }


  // -------------------------------
  // FINGERPRINT SENSOR
  // -------------------------------

  mySerial.begin(
    57600,
    SERIAL_8N1,
    16,
    17
  );

  finger.begin(57600);

  delay(500);


  if (finger.verifyPassword()) {

    Serial.println("Fingerprint sensor ready");

    finger.getTemplateCount();

    Serial.print("Stored fingerprints: ");
    Serial.println(finger.templateCount);

  } else {

    Serial.println("Fingerprint sensor NOT found");

    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Sensor Error");

    while (1) {
      delay(1000);
    }
  }


  // -------------------------------
  // CONNECT WIFI
  // -------------------------------

  connectWiFi();


  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Ready");

  lcd.setCursor(0, 1);
  lcd.print("Place Finger");
}


// =====================================================
// LOOP
// =====================================================

void loop() {

  int fingerprintID = getFingerprintID();


  if (fingerprintID > 0) {

    Serial.print("Fingerprint ID detected: ");
    Serial.println(fingerprintID);


    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Checking...");

    lcd.setCursor(0, 1);
    lcd.print("ID: ");
    lcd.print(fingerprintID);


    String result = sendAttendance(fingerprintID);


    if (result.length() > 0) {

      lcd.clear();

      lcd.setCursor(0, 0);

      if (result.length() > 16) {
        lcd.print(result.substring(0, 16));
      } else {
        lcd.print(result);
      }


      lcd.setCursor(0, 1);

      lcd.print("ID: ");
      lcd.print(fingerprintID);


      Serial.println("Attendance success");

      Serial.print("Name: ");
      Serial.println(result);

    } else {

      lcd.clear();

      lcd.setCursor(0, 0);
      lcd.print("Server Error");

      lcd.setCursor(0, 1);
      lcd.print("Try Again");
    }


    delay(3000);


    // Wait until finger is removed
    while (
      finger.getImage() !=
      FINGERPRINT_NOFINGER
    ) {

      delay(100);

    }


    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Ready");

    lcd.setCursor(0, 1);
    lcd.print("Place Finger");
  }


  delay(100);
}


// =====================================================
// READ FINGERPRINT
// =====================================================

int getFingerprintID() {

  uint8_t p = finger.getImage();


  if (p == FINGERPRINT_NOFINGER) {
    return -1;
  }


  if (p != FINGERPRINT_OK) {

    Serial.println(
      "Fingerprint image error"
    );

    return -1;
  }


  p = finger.image2Tz();


  if (p != FINGERPRINT_OK) {

    Serial.println(
      "Fingerprint conversion error"
    );

    return -1;
  }


  p = finger.fingerFastSearch();


  if (p == FINGERPRINT_OK) {

    Serial.print("Match found. ID: ");
    Serial.println(finger.fingerID);

    Serial.print("Confidence: ");
    Serial.println(finger.confidence);

    return finger.fingerID;

  }


  if (p == FINGERPRINT_NOTFOUND) {

    Serial.println(
      "Fingerprint not registered"
    );


    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Not Registered");


    delay(1500);


    lcd.clear();

    lcd.setCursor(0, 0);
    lcd.print("Ready");

    lcd.setCursor(0, 1);
    lcd.print("Place Finger");

  }


  return -1;
}


// =====================================================
// WIFI CONNECTION
// =====================================================

void connectWiFi() {

  Serial.println();
  Serial.println("Connecting to WiFi...");


  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("Connecting WiFi");


  WiFi.begin(
    WIFI_NAME,
    WIFI_PASSWORD
  );


  int attempt = 0;


  while (
    WiFi.status() != WL_CONNECTED
  ) {

    delay(500);

    Serial.print(".");

    attempt++;


    // restart WiFi connection every ~10 seconds
    if (attempt > 20) {

      Serial.println();

      Serial.println(
        "Retrying WiFi..."
      );


      WiFi.disconnect();

      delay(500);


      WiFi.begin(
        WIFI_NAME,
        WIFI_PASSWORD
      );


      attempt = 0;
    }
  }


  Serial.println();
  Serial.println("WiFi connected");


  Serial.print("ESP32 IP: ");
  Serial.println(WiFi.localIP());


  lcd.clear();

  lcd.setCursor(0, 0);
  lcd.print("WiFi Connected");

  delay(1200);
}


// =====================================================
// SEND ATTENDANCE TO FLASK
// =====================================================

String sendAttendance(int id) {

  // reconnect if WiFi disconnected
  if (
    WiFi.status() != WL_CONNECTED
  ) {

    connectWiFi();

  }


  HTTPClient http;


  Serial.println();
  Serial.println("Sending attendance...");


  Serial.print("Server: ");
  Serial.println(SERVER_URL);


  http.begin(SERVER_URL);


  http.addHeader(
    "Content-Type",
    "application/x-www-form-urlencoded"
  );


  String postData =
    "id=" + String(id);


  Serial.print("POST data: ");
  Serial.println(postData);


  int responseCode =
    http.POST(postData);


  Serial.print(
    "HTTP response code: "
  );

  Serial.println(responseCode);


  String response = "";


  if (responseCode == 200) {

    response = http.getString();

    response.trim();


    Serial.print(
      "Server response: "
    );

    Serial.println(response);

  }


  else if (
    responseCode == 404
  ) {

    response = "Unknown ID";


    Serial.println(
      "Employee not found in database"
    );

  }


  else if (
    responseCode > 0
  ) {

    Serial.print(
      "Server error response: "
    );

    Serial.println(
      http.getString()
    );

  }


  else {

    Serial.print(
      "HTTP connection error: "
    );

    Serial.println(
      http.errorToString(
        responseCode
      )
    );

  }


  http.end();


  return response;
}