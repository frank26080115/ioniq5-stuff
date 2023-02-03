#include "HeadUpDisplay.h"

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>
#include <DNSServer.h>
#include <SPIFFS.h>

#include <WebSocketPrinter.h>
#include <MultiDestPrinter.h>

const char* ssid = "ioniq5-hud";
const char* password = "1234567890";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
IPAddress apIP(8, 8, 8, 8);
IPAddress netMsk(255, 255, 255, 0);
const byte DNS_PORT = 53; 
DNSServer dnsServer;

WebSocketPrinter ws_printer(&ws);
extern MultiDestPrinter log_printer;

bool web_active = false;
bool wifi_isOff = false;

void web_init()
{
    WiFi.softAPConfig(apIP, apIP, netMsk);
    WiFi.softAP(ssid, password);

    dnsServer.setErrorReplyCode(DNSReplyCode::NoError); 
    dnsServer.start(DNS_PORT, "*", apIP);

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", "text/html");
    });

    server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", "text/html");
    });

    server.on("/index.htm", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/index.html", "text/html");
    });

    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send(SPIFFS, "/script.js", "text/javascript");
    });

    server.on("/get_spiffs_file", HTTP_GET, [](AsyncWebServerRequest *request) {
        AsyncWebParameter* p = request->getParam(0);
        String s = p->value();
        char ss[128];
        strncpy(&(ss[1]), s.c_str(), 125);
        ss[0] = '/';
        if (str_endswith(ss, ".css") == 0) {
            request->send(SPIFFS, ss, "text/css");
        }
        else if (str_endswith(ss, ".js") == 0) {
            request->send(SPIFFS, ss, "text/javascript");
        }
        else if (str_endswith(ss, ".png") == 0) {
            request->send(SPIFFS, ss, "image/png");
        }
        else if (str_endswith(ss, ".svg") == 0) {
            request->send(SPIFFS, ss, "image/svg+xml");
        }
        else {
            request->send(SPIFFS, ss, "application/octet-stream");
        }
    });

    server.on("/get_usd_file", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (battlog_cardReady == false) {
            request->send(404);
            return;
        }
        AsyncWebParameter* p = request->getParam(0);
        String s = p->value();
        char ss[128];
        strncpy(&(ss[1]), s.c_str(), 125);
        ss[0] = '/';
        request->send(SD, ss, "application/octet-stream");
    });

    ws.onEvent(web_onEvent);
    server.addHandler(&ws);
}

String web_fnameReplacer(const String& var)
{
    if (var == "FNAME") {
        return battlog_filename;
    }
    return String();
}

void web_task(uint32_t tnow)
{
    if (wifi_isOff) {
        return;
    }

    if (web_active == false)
    {
        if (tnow >= 5 * 60 * 1000)
        {
            if (wifi_isOff == false)
            {
                WiFi.disconnect(true);
                WiFi.mode(WIFI_OFF);
                wifi_isOff = true;
            }
            return;
        }
    }

    dnsServer.processNextRequest();

    web_sendSettingsReport(tnow);
}

static uint32_t web_settingsRptTime = 0;

void web_sendSettingsReport(uint32_t tnow)
{
    if (web_settingsRptTime != 0 && (tnow - web_settingsRptTime) >= 2000) {
        web_settingsRptTime = 0;
        settings_report(log_printer.destinations[LOGPRINTER_IDX_WEBSOCK]);
    }
}

void web_queueSettingsReport()
{
    web_settingsRptTime = millis();
}

void web_handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
    {
        data[len] = 0; // null terminate
        int i;
        for (i = 0; i <= len; i++)
        {
            cmdline.side_enter(data[i]);
        }
    }
}

void web_onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType etype, void *arg, uint8_t *data, size_t len)
{
    switch (etype)
    {
        case WS_EVT_CONNECT:
            {
                web_active = true;
                ws_printer.enabled = true;
                Print* p = dynamic_cast<Print*>(&ws_printer);
                log_printer.destinations[LOGPRINTER_IDX_WEBSOCK] = p;
                Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
                settings_report(p);
                web_settingsRptTime = 0;
            }
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("WebSocket client #%u disconnected\n", client->id());
            log_printer.destinations[LOGPRINTER_IDX_WEBSOCK] = NULL;
            web_settingsRptTime = 0;
            break;
        case WS_EVT_DATA:
            web_handleWebSocketMessage(arg, data, len);
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}