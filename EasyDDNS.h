#ifndef EasyDDNS_H
#define EasyDDNS_H

#include "Arduino.h"
#include "stdlib_noniso.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

// Handler to notify user about new public IP
typedef std::function<void(const char* old_ip, const char* new_ip)> DDNSUpdateHandler;

class EasyDDNSClass {
public:
    void service(String ddns_service);
    void client(String ddns_domain, String ddns_username, String ddns_password = "");
    void update(unsigned long ddns_update_interval, bool use_local_ip = false);
    String getRegisteredIP(); // New method to check the current IP registered with No-IP

    // Callback
    void onUpdate(DDNSUpdateHandler handler) {
        _ddnsUpdateFunc = handler;
    }

private:
    DDNSUpdateHandler _ddnsUpdateFunc = nullptr;

    unsigned long interval;
    unsigned long previousMillis = 0;

    String new_ip;
    String old_ip;
    String update_url;
    String ddns_u;
    String ddns_p;
    String ddns_d;
    String ddns_choice;
};

extern EasyDDNSClass EasyDDNS;

#endif
