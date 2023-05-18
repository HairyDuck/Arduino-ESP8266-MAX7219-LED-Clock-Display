#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_LEDBackpack.h>
#include <WiFiUdp.h>
#include <NTPClient.h>

// WiFi credentials
const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";

// NTP server settings
const char* ntpServer = "pool.ntp.org";
const int timeZone = 0; // London time zone offset (in hours)

// LED display object
Adafruit_7segment matrix = Adafruit_7segment();

// NTP client object
WiFiUDP udp;
NTPClient timeClient(udp, ntpServer, timeZone * 3600, 60000);

// Variables to store current time
int currentHour, currentMinute, currentSecond;
bool isDaylightSaving = false;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize LED display
  matrix.begin(0x00);
  matrix.setBrightness(7); // Adjust brightness level (0-15)

  // Initialize NTP client
  timeClient.begin();
  timeClient.update();
  updateLocalTime();
}

void loop() {
  // Update NTP time every hour
  if (currentMinute == 0 && currentSecond == 0) {
    timeClient.forceUpdate();
    updateLocalTime();
  }

  // Update LED display every second
  displayTime();

  delay(1000);
  currentSecond++;
  if (currentSecond >= 60) {
    currentSecond = 0;
    currentMinute++;
    if (currentMinute >= 60) {
      currentMinute = 0;
      currentHour++;
      if (currentHour >= 24) {
        currentHour = 0;
      }
    }
  }
}

void updateLocalTime() {
  // Get the current NTP time
  time_t utcTime = timeClient.getEpochTime();

  // Apply daylight saving time offset
  struct tm *timeinfo;
  timeinfo = gmtime(&utcTime);
  isDaylightSaving = timeinfo->tm_isdst;

  // Adjust time for London time zone
  time_t localTime = utcTime + (timeZone * 3600);
  timeinfo = gmtime(&localTime);

  // Extract hours, minutes, and seconds
  currentHour = timeinfo->tm_hour;
  currentMinute = timeinfo->tm_min;
  currentSecond = timeinfo->tm_sec;

  Serial.println("Local time updated");
  Serial.print("Current time: ");
  Serial.print(currentHour);
  Serial.print(":");
  Serial.print(currentMinute);
  Serial.print(":");
  Serial.println(currentSecond);
}

void displayTime() {
  // Display hours
  matrix.writeDigitNum(0, currentHour / 10);
  matrix.writeDigitNum(1, currentHour % 10);

  // Display minutes
  matrix.writeDigitNum(3, currentMinute / 10);
  matrix.writeDigitNum(4, currentMinute % 10);

  // Display seconds with flashing separators
  if (currentSecond % 2 == 0) {
    matrix.writeDigitRaw(2, 0); // Flashing off
    matrix.writeDigitRaw(5, 0); // Flashing off
  } else {
    matrix.writeDigitRaw(2, 0x80); // Flashing on (: separator)
    matrix.writeDigitRaw(5, 0x80); // Flashing on (: separator)
  }

  // Display the changes
  matrix.writeDisplay();
}
