function getTimeNum()
{
    // generates epoch time but with a local timezone offset
    // this is sent to the ESP32 so it's own internal time library can generate the correct local date string
    return Math.floor((Date.now() / 1000) - ((new Date()).getTimezoneOffset() * 60));
}

function make_coloured_span(val, val_min, val_avg, val_max, hue_min, hue_avg, hue_max)
{
    var h;
    if (val < val_avg) {
        h = (val - val_min) * (hue_avg - hue_min) / (val_avg - val_min) + hue_min;
    }
    else {
        h = (val - val_avg) * (hue_max - hue_avg) / (val_max - val_avg) + hue_avg;
    }
    if (hue_min <= hue_max) {
        h = h < hue_min ? hue_min : (h > hue_max ? hue_max : h);
    }
    else {
        h = h < hue_max ? hue_max : (h > hue_min ? hue_min : h);
    }
    return "<span style=\"color: hsl(" + h + "deg 100% 75%);\">" + val + "</span>";
}

// dynamic loader

var files_to_load = [
    "jquery-3.6.3.min.js",
    "jquery-ui.min.js",
    "toast.js",
    "jquery-ui.css",
    "jquery-ui.structure.min.css",
    "jquery-ui.theme.css",
    "style.css",
    "chartscript.js",
];

var dyn_load_prefix = "/get_spiffs_file?file=";

var files_loaded = 0;

var dynamic_load_onDone;

function dynamic_load_files(doneFunc)
{
    if (window.location.href.startsWith("file")) {
        dyn_load_prefix = "";
    }

    files_to_load.forEach(ele => {
        if (ele.endsWith(".js")) {
            var s = document.createElement('script'); 
            s.setAttribute("type", "application/javascript");
            s.src = dyn_load_prefix + ele;
            s.onload = function() { 
                files_loaded += 1;
            };
            document.head.appendChild(s);
        }
        else if (ele.endsWith(".css")) {
            var s = document.createElement('link'); 
            s.setAttribute("rel", "stylesheet");
            s.href = ele;
            s.onload = function() { 
                files_loaded += 1;
            };
            document.head.appendChild(s);
        }
    });
    dynamic_load_onDone = doneFunc;
    setTimeout(dynamic_load_wait, 1000);
}

var dynamic_load_wait_seconds = 0;

function dynamic_load_wait()
{
    dynamic_load_wait_seconds += 1;
    if (dynamic_load_wait_seconds >= 5) {
        script_file_loaded = false;
        dynamic_load_onDone(false);
    }
    else {
        if (files_loaded >= files_to_load.length) {
            dynamic_load_onDone(true);
        }
        else {
            setTimeout(dynamic_load_wait, 1000);
        }
    }
}

// websocket stuff

var ws_gateway = `ws://${window.location.hostname}/ws`;
var ws_websocket = null;
var ws_lastRxTime = null;
var ws_periodicTimer = null;

function ws_initWebSocket()
{
    console.log('Trying to open a WebSocket connection...');
    ws_websocket = new WebSocket(ws_gateway);
    ws_websocket.onopen    = ws_onOpen;
    ws_websocket.onclose   = ws_onClose;
    ws_websocket.onmessage = ws_onMessage;
    ws_websocket.onerror   = ws_onError;
    if (ws_periodicTimer == null) {
        ws_periodicTimer = setTimeout(ws_periodicCheck, 2000);
    }
}

function ws_periodicCheck()
{
    if (ws_websocket == null) {
        ws_initWebSocket();
    }
    setTimeout(ws_periodicCheck, 2000);
}

function ws_onOpen(evt)
{
    console.log('ws connection opened');
    ws_userOnOpen();
}

function ws_onClose(evt)
{
    console.log('ws connection closed');
    ws_websocket = null;
}

function ws_onError(evt)
{
    console.log('ws connection error');
}

var ws_msgBuff = "";

function ws_onMessage(evt)
{
    ws_lastRxTime = Date.now();
    var msgStr = evt.data;
    for (var i = 0; i < msgStr.length; i++) {
        var c = msgStr.charAt(i);
        if (c == '\n') {
            ws_userFunc(ws_msgBuff);
            ws_msgBuff = "";
        }
        else {
            ws_msgBuff += c;
        }
    }
}

function ws_send(msg)
{
    console.log("ws send: " + msg);
    if (ws_websocket == null) {
        return;
    }
    ws_websocket.send(msg);
}

var script_file_loaded = false;