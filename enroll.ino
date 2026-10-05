#include <Adafruit_Fingerprint.h>

HardwareSerial mySerial(2);
Adafruit_Fingerprint finger(&mySerial);

uint8_t id;

void setup() {
  Serial.begin(115200);

  mySerial.begin(
    57600,
    SERIAL_8N1,
    16,
    17
  );

  finger.begin(57600);

  delay(500);

  Serial.println();
  Serial.println("==========================");
  Serial.println("Fingerprint Enrollment");
  Serial.println("==========================");

  if (!finger.verifyPassword()) {
    Serial.println("Fingerprint sensor NOT found");
    while (1) {
      delay(1000);
    }
  }

  Serial.println("Fingerprint sensor ready");

  finger.getTemplateCount();

  Serial.print("Stored fingerprints: ");
  Serial.println(finger.templateCount);

  Serial.println();
  Serial.println("Enter fingerprint ID:");
}

void loop() {
  if (Serial.available()) {

    id = Serial.parseInt();

    while (Serial.available()) {
      Serial.read();
    }

    if (id < 1 || id > 300) {
      Serial.println("Invalid ID");
      return;
    }

    Serial.print("Enrolling ID: ");
    Serial.println(id);

    if (enrollFingerprint()) {
      Serial.print("SUCCESS. Stored ID: ");
      Serial.println(id);

      Serial.println();
      Serial.println("Enter another fingerprint ID:");
    }
  }
}

bool enrollFingerprint() {
  int p = -1;

  Serial.println("Place finger...");

  while (p != FINGERPRINT_OK) {
    p = finger.getImage();

    if (p == FINGERPRINT_NOFINGER) {
      delay(100);
      continue;
    }
  }

  Serial.println("First image captured");

  p = finger.image2Tz(1);

  if (p != FINGERPRINT_OK) {
    Serial.println("First image conversion failed");
    return false;
  }

  Serial.println("Remove finger");

  delay(1500);

  while (finger.getImage() != FINGERPRINT_NOFINGER) {
    delay(100);
  }

  Serial.println("Place SAME finger again");

  p = -1;

  while (p != FINGERPRINT_OK) {
    p = finger.getImage();

    if (p == FINGERPRINT_NOFINGER) {
      delay(100);
      continue;
    }
  }

  Serial.println("Second image captured");

  p = finger.image2Tz(2);

  if (p != FINGERPRINT_OK) {
    Serial.println("Second image conversion failed");
    return false;
  }

  p = finger.createModel();

  if (p != FINGERPRINT_OK) {
    Serial.println("Fingerprints did not match");
    return false;
  }

  p = finger.storeModel(id);

  if (p == FINGERPRINT_OK) {
    Serial.println("Stored successfully");
    return true;
  }

  Serial.println("Storage failed");
  return false;
}