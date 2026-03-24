#include <Arduino.h>
#include <U8g2lib.h>
#include <WiFi.h>
#include <WebServer.h>

// OLED setup
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// Web server
WebServer server(80);

// System values received from laptop
int cpuUsage = 0;
int ramUsage = 0;
int netTx = 0;
int netRx = 0;
unsigned long uptimeSec = 0;

// Built-in LED for high usage
const int ledPin = 2;

// Web server handlers
void handleRoot() {
  server.send(200, "text/plain", "ESP32 Monitor Running");
}

void handleUpdate() {
  if(server.hasArg("cpu")) cpuUsage = server.arg("cpu").toInt();
  if(server.hasArg("ram")) ramUsage = server.arg("ram").toInt();
  if(server.hasArg("tx")) netTx = server.arg("tx").toInt();
  if(server.hasArg("rx")) netRx = server.arg("rx").toInt();
  if(server.hasArg("uptime")) uptimeSec = server.arg("uptime").toInt();
  server.send(200, "text/plain", "OK");
}

// Boot animation with progress bar
void drawBootScreen() {
  const char* bootText[] = {
    "Initializing system...",
    "Loading drivers...",
    "Starting kernel...",
    "Mounting FS...",
    "Starting services...",
    "System Ready"
  };
  int totalWidth = 108;
  for(int i=0;i<6;i++){
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_6x10_tr);
    u8g2.drawStr(18,12,"ESP32 SYS MON");
    u8g2.setFont(u8g2_font_5x8_tr);
    u8g2.drawStr(5,35,bootText[i]);
    u8g2.drawFrame(10,45,totalWidth,10);
    int fillWidth = totalWidth*(i+1)/6;
    u8g2.drawBox(12,47,fillWidth,6);
    u8g2.sendBuffer();
    delay(700);
  }
  delay(500);
}

void setup() {
  Serial.begin(115200);
  u8g2.begin();
  pinMode(ledPin, OUTPUT);

  drawBootScreen();

  // Start ESP32 AP
  WiFi.softAP("ESP32_MONITOR", "12345678");
  Serial.println("ESP32 AP started: ESP32_MONITOR");
  Serial.println("Connect your laptop to ESP32 Wi-Fi");

  // Web server routes
  server.on("/", handleRoot);
  server.on("/update", handleUpdate);
  server.begin();
}

void loop() {
  server.handleClient();

  // LED ON if CPU or RAM >= 80%
  digitalWrite(ledPin, (cpuUsage >= 80 || ramUsage >= 80) ? HIGH : LOW);

  // Draw OLED dashboard
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(20,10,"LAPTOP MONITOR");

  u8g2.setFont(u8g2_font_5x8_tr);
  char buf[16];
  int barX = 30, barWidth = 80, barHeight = 6;

  // CPU
  u8g2.drawStr(0,22,"CPU:");
  u8g2.drawFrame(barX,18,barWidth,barHeight);
  u8g2.drawBox(barX,18,cpuUsage*barWidth/100,barHeight);
  sprintf(buf,"%d%%",cpuUsage);
  u8g2.drawStr(barX + barWidth + 2,22,buf);

  // RAM
  u8g2.drawStr(0,32,"RAM:");
  u8g2.drawFrame(barX,28,barWidth,barHeight);
  u8g2.drawBox(barX,28,ramUsage*barWidth/100,barHeight);
  sprintf(buf,"%d%%",ramUsage);
  u8g2.drawStr(barX + barWidth + 2,32,buf);

  // Network TX/RX
  u8g2.drawStr(0,42,"TX:");
  sprintf(buf,"%d KB/s",netTx);
  u8g2.drawStr(barX,42,buf);

  u8g2.drawStr(0,52,"RX:");
  sprintf(buf,"%d KB/s",netRx);
  u8g2.drawStr(barX,52,buf);

  // Uptime
  int hours = uptimeSec / 3600;
  int minutes = (uptimeSec % 3600) / 60;
  int seconds = uptimeSec % 60;
  sprintf(buf,"%02d:%02d:%02d", hours, minutes, seconds);
  u8g2.drawStr(0,62,"UP:");
  u8g2.drawStr(barX,62,buf);

  u8g2.sendBuffer();
  delay(500);
}