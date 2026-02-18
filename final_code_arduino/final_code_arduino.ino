#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <time.h>

// ============================================
// KIOSK SCANNER - ESP32 #2
// Device ID: KIOSK-ESP32-002
// Purpose: ONLY UPDATES DISPLAY SCREEN
// ============================================

// ---------------- WIFI ----------------
#define WIFI_SSID "Airtel_Youngovator"
#define WIFI_PASSWORD "Youngovator@130589"

// ---------------- DEVICE ID - DO NOT CHANGE ----------------
#define DEVICE_ID "KIOSK-ESP32-002"

// ---------------- GOOGLE SHEETS URL ----------------
// PASTE YOUR WEB APP URL HERE (ends with /exec)
String GOOGLE_SCRIPT_URL = "https://script.google.com/macros/s/AKfycbz1iUmBfoTAGcmAqthsODGVczo824zW-YAcKDLC688Y8DwMICqJ7oEvOMRjJJPzmaQK/exec";

// ---------------- RFID PINS ----------------
#define SS_PIN 21
#define RST_PIN 22

// ---------------- BUZZER ----------------
#define BUZZER_PIN 5

MFRC522 rfid(SS_PIN, RST_PIN);

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Startup beep (different from attendance scanner)
  kioskBeep();
  
  // Connect WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n✅ WiFi Connected");
  
  // Sync time
  configTime(19800, 0, "pool.ntp.org");
  delay(2000);
  
  // Initialize RFID
  SPI.begin();
  rfid.PCD_Init();
  
  Serial.println("========================================");
  Serial.println("   KIOSK SCANNER READY");
  Serial.println("   Device ID: KIOSK-ESP32-002");
  Serial.println("   Purpose: DISPLAY SCREEN ONLY");
  Serial.println("   Attendance: WILL NOT BE MARKED");
  Serial.println("========================================\n");
}

void loop() {
  // Wait for RFID card
  if (!rfid.PICC_IsNewCardPresent() || !rfid.PICC_ReadCardSerial()) {
    return;
  }

  String uid = getUID();
  String date = getDate();
  String timeNow = getTime();

  Serial.println("\n========== CARD SCANNED ==========");
  Serial.println("UID: " + uid);
  Serial.println("Device: KIOSK-ESP32-002");
  Serial.println("Date: " + date);
  Serial.println("Time: " + timeNow);

  // Send to Google Sheets
  sendToKiosk(uid, date, timeNow);
  
  Serial.println("✅ SCREEN WILL OPEN");
  Serial.println("❌ ATTENDANCE NOT MARKED");
  Serial.println("==================================\n");

  // Confirmation beep
  kioskBeep();

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(2000);
}

void sendToKiosk(String uid, String date, String time) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("❌ WiFi not connected");
    return;
  }

  HTTPClient http;

  // Create JSON with device ID
  String json = "{\"uid\":\"" + uid + 
                "\",\"date\":\"" + date + 
                "\",\"time\":\"" + time + 
                "\",\"deviceId\":\"" + String(DEVICE_ID) + "\"}";

  Serial.println("→ Sending to Google Sheets...");
  Serial.println("→ JSON: " + json);

  http.begin(GOOGLE_SCRIPT_URL);
  http.addHeader("Content-Type", "application/json");

  int code = http.POST(json);

  if (code == 200 || code == 302) {
    Serial.println("✅ Successfully sent (HTTP " + String(code) + ")");
  } else {
    Serial.println("❌ HTTP Error: " + String(code));
  }

  http.end();
}

String getUID() {
  String uid = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    if (rfid.uid.uidByte[i] < 0x10) uid += "0";
    uid += String(rfid.uid.uidByte[i], HEX);
  }
  uid.toUpperCase();
  return uid;
}

String getDate() {
  struct tm t;
  if (!getLocalTime(&t)) return "unknown";
  char buf[11];
  strftime(buf, sizeof(buf), "%Y-%m-%d", &t);
  return String(buf);
}

String getTime() {
  struct tm t;
  if (!getLocalTime(&t)) return "unknown";
  char buf[9];
  strftime(buf, sizeof(buf), "%H:%M:%S", &t);
  return String(buf);
}

// KIOSK BEEP (different from attendance scanner)
void kioskBeep() {
  tone(BUZZER_PIN, 2500, 150);
  delay(200);
  tone(BUZZER_PIN, 3000, 100);
  delay(150);
  noTone(BUZZER_PIN);
}
