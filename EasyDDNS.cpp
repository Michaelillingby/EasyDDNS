#include "EasyDDNS.h"

void EasyDDNSClass::service(String ddns_service) {
    ddns_choice = ddns_service;
}

void EasyDDNSClass::client(String ddns_domain, String ddns_username, String ddns_password) {
    ddns_d = ddns_domain;
    ddns_u = ddns_username;
    ddns_p = ddns_password;
}

String EasyDDNSClass::getRegisteredIP() {
    String registered_ip = "";
    WiFiClient client;
    HTTPClient http;
    String check_url = "http://checkip.dyndns.org/";
    http.begin(client, check_url);
    http.setAuthorization(ddns_u.c_str(), ddns_p.c_str());
    int httpCode = http.GET();
    if (httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        int start = payload.indexOf(": ") + 2;
        int end = payload.indexOf("</body>");
        if (start > 0 && end > start) {
            registered_ip = payload.substring(start, end);
            registered_ip.trim();
        }
    } else {
        Serial.print("Failed to check registered IP, HTTP code: ");
        Serial.println(httpCode);
    }
    http.end();
    return registered_ip;
}

void EasyDDNSClass::update(unsigned long ddns_update_interval, bool use_local_ip) {
    interval = ddns_update_interval;
    unsigned long currentMillis = millis(); // Calculate Elapsed Time & Trigger

    if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;

        Serial.println("Fetching current IP...");

        if (use_local_ip) {
            IPAddress ipAddress = WiFi.localIP();
            new_ip = String(ipAddress[0]) + "." +
                     String(ipAddress[1]) + "." +
                     String(ipAddress[2]) + "." +
                     String(ipAddress[3]);
        } else {
            // Fetch public IP
            WiFiClient client;
            HTTPClient http;
            http.setTimeout(10000);  // Set timeout to 10 seconds
            http.begin(client, "http://api.ipify.org"); // Alternative service to fetch public IP
            int httpCode = http.GET();
            if (httpCode > 0) {
                if (httpCode == HTTP_CODE_OK) {
                    new_ip = http.getString();
                    new_ip.trim();
                    Serial.print("Fetched public IP: ");
                    Serial.println(new_ip);
                } else {
                    Serial.print("Failed to fetch public IP, HTTP code: ");
                    Serial.println(httpCode);
                    http.end();
                    return;
                }
            } else {
                Serial.print("HTTP request failed, error: ");
                Serial.println(http.errorToString(httpCode));
                http.end();
                return;
            }
            http.end();
        }

        // Generate update URL
        if (ddns_choice == "duckdns") {
            update_url = "http://www.duckdns.org/update?domains=" + ddns_d + "&token=" + ddns_u + "&ip=" + new_ip;
        } else if (ddns_choice == "noip") {
            update_url = "http://" + ddns_u + ":" + ddns_p + "@dynupdate.no-ip.com/nic/update?hostname=" + ddns_d + "&myip=" + new_ip;
        } else {
            Serial.println("## INPUT CORRECT DDNS SERVICE NAME ##");
            return;
        }

        String registered_ip = getRegisteredIP();
        Serial.print("Registered IP with No-IP: ");
        Serial.println(registered_ip);

        // Check & Update
        if (registered_ip != new_ip) {
            Serial.print("IP has changed from ");
            Serial.print(registered_ip);
            Serial.print(" to ");
            Serial.println(new_ip);

            WiFiClient client;
            HTTPClient http;
            http.setTimeout(10000);  // Set timeout to 10 seconds
            http.begin(client, update_url);
            int httpCode = http.GET();
            if (httpCode == HTTP_CODE_OK) {
                Serial.println("DDNS update successful");
                if (_ddnsUpdateFunc != nullptr) {
                    _ddnsUpdateFunc(registered_ip.c_str(), new_ip.c_str());
                }
                old_ip = new_ip; // Replace Old IP with new one to detect further changes.
            } else {
                Serial.print("DDNS update failed, HTTP code: ");
                Serial.println(httpCode);
            }
            http.end();
        } else {
            Serial.println("IP has not changed.");
        }
    }
}

EasyDDNSClass EasyDDNS;
