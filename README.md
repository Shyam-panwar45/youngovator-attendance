# 🎓 Youngovator Smart RFID Attendance System

A complete IoT-based Smart Attendance System built using:

- 📡 ESP32 + RC522 RFID Module  
- ☁ Google Apps Script (Backend API)  
- 📊 Google Sheets (Cloud Database)  
- 🌐 Vercel Hosted Web Application  
- 🖥 Raspberry Pi Kiosk System  
- 🖨 Direct IP Printing via CUPS  

This system works as a fully automated commercial-grade attendance kiosk.

---

# 🧠 System Architecture Overview

RFID Card
↓
RC522 (connected to ESP32)
↓
ESP32 reads UID
↓
ESP32 sends UID via Web URL
↓
Google Apps Script (Web API)
↓
Google Sheets (Database)
↓
Web UI fetches latest scan
↓
Dashboard opens
↓
User can print attendance
↓
Node Print Server (Raspberry Pi)
↓
CUPS (IPP Direct IP)
↓
EPSON L3560 Printer


---

# 🔌 Hardware Components

## 1️⃣ RFID Scanner Unit (Attendance Scanner)

- RC522 RFID Module
- ESP32 Microcontroller
- RFID Cards

### Working Process

1. User taps RFID card.
2. RC522 reads the unique UID.
3. ESP32 captures the UID.
4. ESP32 sends UID to Google Apps Script using a Web URL.
5. Apps Script processes the UID and updates Google Sheets.

---

# ☁ Backend – Google Apps Script

Google Apps Script works as the cloud API.

When ESP32 sends a UID:

- Apps Script checks **Sheet 1** for registered UID.
- If found:
  - Records attendance in **Sheet 2**
  - Logs kiosk scan in **Sheet 3**

---

# 📊 Google Sheets Structure

## 📁 Sheet 1 – Master Student Database

Contains:

- UID
- Name
- Father’s Name
- Address
- Student ID
- Photo Reference
- Other details

This sheet acts as the primary database.

---

## 📁 Sheet 2 – Attendance Records

Stores:

- UID
- Date
- Time
- Status (Full Day / Half Day / Absent)

Every attendance scanner tap updates this sheet.

---

## 📁 Sheet 3 – Kiosk Scanner Logs

Stores:

- UID
- Name
- Timestamp

Used for:

- Detecting kiosk scans
- Triggering dashboard opening in UI

---

# 🖥 Kiosk System (Raspberry Pi)

The Raspberry Pi acts as a smart kiosk device.

It runs:

- Chromium in Kiosk Mode
- Node.js Print Server
- CUPS Printing Service

---

# 🌐 Web Application (Hosted on Vercel)

The Web App:

- Continuously polls Google Apps Script
- Detects new UID entry in Sheet 3
- Fetches student data from Sheets
- Opens dashboard automatically
- Displays attendance summary
- Allows report viewing
- Allows report printing

---

# 🖨 Printing System

## How Printing Works

1. User clicks **Print Report**
2. Web app sends formatted report to:http://<raspberry-pi-ip>:3000/print


3. Node.js server receives the content
4. Server:
   - Cleans special characters
   - Saves report to `/tmp/attendance_report.txt`
5. CUPS prints file using: lp -d EPSON_L3560_Series /tmp/attendance_report.txt

---

# 🖨 Printer Configuration (Permanent Fix)

Printer is configured using Direct IP: ipp://192.168.1.**/ipp/print

⚠ Important:  
Do NOT use `implicitclass://`  

It causes:
- Auto-disable
- Queue stuck
- cups-browsed dependency issues

Direct IP ensures stable printing.

---

# ⚙️ Automatic Startup Configuration

## 1️⃣ Print Server (systemd)

Service file location: /etc/systemd/system/attendance-print.service


Starts automatically on boot.

---

## 2️⃣ Chromium Kiosk Mode

Startup script: /home/anandsir/kiosk.sh


Autostart location: ~/.config/lxsession/LXDE-pi/autostart


Chromium runs with: 
--kiosk
--kiosk-printing
--disable-print-preview
--noerrdialogs
--disable-infobars





---

## 📌 Conclusion

This project demonstrates a complete real-world implementation of an IoT-based Smart Attendance System integrating embedded hardware, cloud backend, kiosk interface, and direct IP printing.

It is designed to operate as a fully automated commercial-grade attendance solution with minimal maintenance and maximum reliability.

---

## 📄 License

This project is developed for internal institutional use.

---

## 👨‍💻 Author

Developed by **Shyam Panwar**  
Under the guidance of **Mr. Anand Kumar Payasi**
Robotics & Automation Engineer  
Youngovator Education Pvt Ltd


