final_code_attendance
#include <WiFi.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <MFRC522.h>
#include <time.h>
#include <Preferences.h>

// ============================================
// ATTENDANCE SCANNER - ESP32 #1
// Device ID: ATT-ESP32-001
// Purpose: ONLY MARKS ATTENDANCE
// ============================================

// ---------------- WIFI ----------------
#define WIFI_SSID "Airtel_Youngovator"
#define WIFI_PASSWORD "Youngovator@130589"

// ---------------- DEVICE ID - DO NOT CHANGE ----------------
#define DEVICE_ID "ATT-ESP32-001"

// ---------------- GOOGLE SHEETS URL ----------------
// PASTE YOUR WEB APP URL HERE (ends with /exec)
String GOOGLE_SCRIPT_URL = "https://script.google.com/macros/s/AKfycbwtpgn4zlX_clmc_CMpxOZ_wOEehMl08Oqi2uccdp2YaeTlMkabppKFYZ15Bhp0D5WK/exec";

// ---------------- RFID PINS ----------------
#define SS_PIN 21
#define RST_PIN 22

// ---------------- BUZZER ----------------
#define BUZZER_PIN 5

MFRC522 rfid(SS_PIN, RST_PIN);
Preferences preferences;

struct CardState {
  bool hasLogin;
  bool hasLogout;
  String date;
};

CardState cardStates[10];
String cardUIDs[10];
int cardCount = 0;

void setup() {
  Serial.begin(115200);
  pinMode(BUZZER_PIN, OUTPUT);
  
  // Startup beep
  resetBeep();
  
  preferences.begin("attendance", false);
  
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
  Serial.println("   ATTENDANCE SCANNER READY");
  Serial.println("   Device ID: ATT-ESP32-001");
  Serial.println("   Purpose: MARKS ATTENDANCE ONLY");
  Serial.println("   Screen: WILL NOT OPEN");
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
  Serial.println("Device: ATT-ESP32-001");
  Serial.println("Date: " + date);
  Serial.println("Time: " + timeNow);

  // Get card index
  int cardIndex = getCardIndex(uid);

  // Check if new day
  if (cardStates[cardIndex].date != date) {
    Serial.println("→ New day detected - resetting state");
    cardStates[cardIndex].hasLogin = false;
    cardStates[cardIndex].hasLogout = false;
    cardStates[cardIndex].date = date;
    saveCardState(uid, date, false, false);
  }

  // Determine action
  if (!cardStates[cardIndex].hasLogin) {
    // First scan - LOGIN
    Serial.println("→ ACTION: LOGIN");
    cardStates[cardIndex].hasLogin = true;
    saveCardState(uid, date, true, false);
    sendToSheets(uid, date, timeNow);
    loginTone();
    Serial.println("✅ LOGIN SENT TO GOOGLE SHEETS");
  }
  else if (!cardStates[cardIndex].hasLogout) {
    // Second scan - LOGOUT
    Serial.println("→ ACTION: LOGOUT");
    cardStates[cardIndex].hasLogout = true;
    saveCardState(uid, date, true, true);
    sendToSheets(uid, date, timeNow);
    logoutTone();
    Serial.println("✅ LOGOUT SENT TO GOOGLE SHEETS");
  }
  else {
    // Already completed
    Serial.println("⚠ Already scanned twice today");
    invalidTone();
    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    delay(2000);
    return;
  }

  Serial.println("✅ ATTENDANCE MARKED");
  Serial.println("❌ SCREEN WILL NOT OPEN");
  Serial.println("==================================\n");

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(3000);
}

void sendToSheets(String uid, String date, String time) {
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

int getCardIndex(String uid) {
  // Find existing card
  for (int i = 0; i < cardCount; i++) {
    if (cardUIDs[i] == uid) return i;
  }

  // Add new card
  if (cardCount < 10) {
    cardUIDs[cardCount] = uid;
    cardStates[cardCount].hasLogin = false;
    cardStates[cardCount].hasLogout = false;
    cardStates[cardCount].date = "";
    loadCardState(uid, cardCount);
    return cardCount++;
  }

  return 0;
}

void saveCardState(String uid, String date, bool hasLogin, bool hasLogout) {
  String key = uid + "_" + date;
  String value = String(hasLogin) + "," + String(hasLogout);
  preferences.putString(key.c_str(), value);
}

void loadCardState(String uid, int index) {
  String date = getDate();
  String key = uid + "_" + date;
  String value = preferences.getString(key.c_str(), "");

  if (value.length() > 0) {
    int commaIndex = value.indexOf(',');
    cardStates[index].hasLogin = value.substring(0, commaIndex) == "1";
    cardStates[index].hasLogout = value.substring(commaIndex + 1) == "1";
    cardStates[index].date = date;
  }
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

// BUZZER TONES
void loginTone() {
  tone(BUZZER_PIN, 1500, 100);
  delay(150);
  tone(BUZZER_PIN, 2000, 200);
  delay(250);
  noTone(BUZZER_PIN);
}

void logoutTone() {
  tone(BUZZER_PIN, 2000, 200);
  delay(250);
  tone(BUZZER_PIN, 1500, 100);
  delay(150);
  noTone(BUZZER_PIN);
}

void invalidTone() {
  for (int i = 0; i < 3; i++) {
    tone(BUZZER_PIN, 400);
    delay(120);
    noTone(BUZZER_PIN);
    delay(80);
  }
  tone(BUZZER_PIN, 200);
  delay(500);
  noTone(BUZZER_PIN);
}

void resetBeep() {
  for (int i = 1000; i <= 2000; i += 150) {
    tone(BUZZER_PIN, i, 60);
    delay(60);
  }
  noTone(BUZZER_PIN);
}