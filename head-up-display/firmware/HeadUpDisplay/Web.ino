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

    server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/style.css", "text/css");
    });

    server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/script.js", "text/javascript");
    });

    server.on("/canvasjs.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/canvasjs.min.js", "text/javascript");
    });

    server.on("/jquery.canvasjs.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/jquery.canvasjs.min.js", "text/javascript");
    });

    server.on("/jquery-3.6.3.min.js", HTTP_GET, [](AsyncWebServerRequest *request){
        request->send(SPIFFS, "/jquery-3.6.3.min.js", "text/javascript");
    });


    if (battlog_cardReady) {
        server.serveStatic("/", SD, "/");
    }

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
            }
            break;
        case WS_EVT_DISCONNECT:
            Serial.printf("WebSocket client #%u disconnected\n", client->id());
            log_printer.destinations[LOGPRINTER_IDX_WEBSOCK] = NULL;
            break;
        case WS_EVT_DATA:
            web_handleWebSocketMessage(arg, data, len);
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
    }
}