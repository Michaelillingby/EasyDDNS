#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <EasyDDNS.h>

const char* ssid = "SSID";
const char* password = "PASSWORD";

IPAddress staticIP(10, 0, 1, 9);
IPAddress gateway(10, 0, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress primaryDNS(8, 8, 8, 8);  // Google's Public DNS
IPAddress secondaryDNS(8, 8, 4, 4);  // Google's Public DNS

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);

  if (!WiFi.config(staticIP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("STA Failed to configure");
  }

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  EasyDDNS.service("noip");
  EasyDDNS.client("NOIP SITE FQDN ", "USERNAME", "PASSWORD/TOKEN");

  EasyDDNS.onUpdate([](const char* oldIP, const char* newIP) {
    Serial.print("***************************EasyDDNS - IP Change Detected: ");
    Serial.println(newIP);
  });

  // Perform initial tests
  testDNSResolution();
  testHTTPClient();
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    EasyDDNS.update(10000);  // Check every 10 seconds
  } else {
    Serial.println("WiFi not connected. Reconnecting...");
    WiFi.reconnect();
    delay(10000); // Wait 10 seconds before trying again
  }
  delay(1000); // Add delay to slow down the loop
}

void testDNSResolution() {
  Serial.println("Testing DNS resolution...");
  IPAddress ip;
  if (WiFi.hostByName("example.com", ip)) {
    Serial.print("DNS resolution successful. IP: ");
    Serial.println(ip);
  } else {
    Serial.println("DNS resolution failed.");
  }
}

void testHTTPClient() {
  Serial.println("Testing HTTP client...");
  WiFiClient client;
  HTTPClient http;
  http.setTimeout(10000);  // Set timeout to 10 seconds

  Serial.print("Testing HTTP client with URL: http://httpbin.org/ip\n");
  http.begin(client, "http://httpbin.org/ip");
  int httpCode = http.GET();
  if (httpCode > 0) {
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println("HTTP GET request successful\nResponse:");
      Serial.println(payload);
    } else {
      Serial.print("HTTP GET request failed, HTTP code: ");
      Serial.println(httpCode);
    }
  } else {
    Serial.print("HTTP request failed, error: ");
    Serial.println(http.errorToString(httpCode));
  }
  http.end();
}
