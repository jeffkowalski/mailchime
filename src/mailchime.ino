#include "Arduino.h"
#include "credentials.h"
#include <ELECHOUSE_CC1101_SRC_DRV.h>
#include <EMailSender.h>
#include <ESP8266WiFi.h>
#include <RCSwitch.h>

#define INDICATOR LED_BUILTIN

static EMailSender emailSend (GMAIL_USERNAME, GMAIL_PASSWORD);  // defined in credentials.h
static RCSwitch    mySwitch = RCSwitch ();

void mail (char const * recipient, char const * subject, char const * body) {
    EMailSender::EMailMessage message;
    message.subject            = subject;
    message.message            = body;
    EMailSender::Response resp = emailSend.send (recipient, message);
    Serial.print ("Sending status: code ");
    Serial.print (resp.code);
    Serial.print (" desc ");
    Serial.print (resp.desc);
    Serial.print (" status ");
    Serial.println (resp.status);
}

unsigned long heartbeat;

void setup () {
    heartbeat = millis ();

    pinMode (INDICATOR, OUTPUT);
    digitalWrite (INDICATOR, LOW);  // negative logic

    Serial.begin (9600);

    ELECHOUSE_cc1101.Init ();           // must be set to initialize the cc1101!
    ELECHOUSE_cc1101.setRxBW (1);       // set Receive filter bandwidth (default = 812khz) 1=58khz, 16=812khz.
    ELECHOUSE_cc1101.setMHZ (433.895);  // empirically measured; nominally 433.92
    mySwitch.enableReceive (D2);
    ELECHOUSE_cc1101.SetRx ();

    WiFi.begin (WIFI_SSID, WIFI_PSK);  // defined in credentials.h
    WiFi.waitForConnectResult ();      // so much neater than those stupid loops and dots

    digitalWrite (INDICATOR, HIGH);  // negative logic

    mail ("jeff.kowalski@gmail.com", "mailchime is up and running", "Ready!");
}

#define ADDRESS 9367300

void loop () {
    if (millis () - heartbeat >= 1000 * 60 * 60 * 8) {  // 8 hours
        mail ("jeff.kowalski@gmail.com", "mailchime is listening", "waiting for mail");
        heartbeat = millis ();
    }

    if (mySwitch.available ()) {
        unsigned long address = mySwitch.getReceivedValue();
        if (1 || mySwitch.getReceivedValue () == ADDRESS) {
            digitalWrite (INDICATOR, LOW);  // negative logic

            Serial.print ("Received ");
            Serial.print (mySwitch.getReceivedValue ());
            Serial.print (" / ");
            Serial.print (mySwitch.getReceivedBitlength ());
            Serial.print ("bit ");
            Serial.print ("Protocol: ");
            Serial.println (mySwitch.getReceivedProtocol ());
            Serial.print ("RSSI: ");
            Serial.println (ELECHOUSE_cc1101.getRssi ());

            char body[256];
            snprintf (body, sizeof (body), "Looks like some mail has arrived.<br><i>address = %lu, rssi = %d</i>", address, ELECHOUSE_cc1101.getRssi ());
            mail ("jeff.kowalski@gmail.com", "The mailbox was opened", body);

            delay (10000);                   // debounce the transmitter a bit
            digitalWrite (INDICATOR, HIGH);  // negative logic
        }
        mySwitch.resetAvailable ();
    }
}
