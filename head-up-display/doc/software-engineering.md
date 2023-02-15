## Threading

The ESP32 is dual core inside, but when you use Arduino, the loop function only runs on a single core. To maximize the LED strip's refresh rate and minimize lag (and for fun), I decided to make this project run on two main threads, one per core.

The first thread is responsible for taking system inputs as fast as possible. This means querying the car, parsing data from the car, reading the light sensor (implemented non-blocking so it's very fast), handling command line commands (which isn't used in normal operation). The CAN bus IO operations are on this thread, but most of that should be non-blocking and performed by the CAN peripheral inside the ESP32.

The second thread handles the main state machine, the LED strip, data logging, and Wi-Fi related functions. The LED strip is wired to be driven via bit-bang SPI so it will occupy a lot of time to refresh. The SD card for data logging is wired to a native SPI interface, and is expected to have huge transactions when the cache needs to be flushed to the card. This thread will make sure the SD card cache flushing only happens immediately after the LED strip has been updated (only write to SD card at the start of long pauses between LED strip refresh).

Since this project is written within the Arduino ecosystem, the Wi-Fi related functionality is running on a background thread that I do not have much control over. The web server is running asynchronously relative to the rest of the code. The web page interface utilizes Javascript as much as possible to minimize work by the ESP32.

## Code Modules

 * main loop
   * actually on the second thread
   * runs state machine
   * calls the LED strip animation function
   * handles timing for animation (uses deep sleep when appropriate)
 * main state machine
   * transitions between car states
   * picks animations when the car turns on or turns off
 * CAN bus
   * interfaces my code to the ISOTP-C library, to handle CAN bus communications
 * OBD2 querying
   * times queries to not overwhelm the car's ECU
   * parses replies to the queries
   * packets are stored in a big database, so the data can be easily examined for other purposes (can be logged)
 * settings
   * reads settings in fake-EEPROM (and checks data integrity)
   * saves settings, but rate limited
   * formats settings to be sent over websocket
 * web server
   * DNS redirect for captive portal
   * HTTP server, serves from SPIFFS
   * websocket server, sends settings and log data to the web page client
 * command line
   * handles commands from serial port, mostly for debugging
   * handles commands from websocket client
 * logger
   * implemented a multiple-destination `Print` class, so calling `printf` once can write to both the websocket and SD card
   * both websocket and SD card heavily utilizes caching to improve performance
   * log is formatted as CSV compatible with spreadsheet software
   * raw log of CAN bus packet database is possible
 * ambient-light
   * reads a photocell
   * performs filtering and calculations
   * adjusts overall brightness of LED strip
 * speed calibration
   * converts electric motor RPM to MPH
   * able to live-calibrate based on simultaneous RPM and KHM data, with averaging and filtering
   * live-calibration adds 100ms of lag, so it's disabled most of the time
 * speed prediction
   * tracks last 3 samples of speed, but each sample is about 100ms apart
   * predicts (extrapolates) the current speed using acceleration and jerk from the prevous 3 samples
   * very adjustable, uses filtering and slew rate limiting to prevent jitter, but eliminates lag
   * makes the speedometer animation appear smoother
 * LED strip animation
   * each call to animation state machine is one frame
   * has many animations for fade-in and fade-out
   * speedometer display, the most important part of this project!
 * heartbeat
   * there's a RGB LED on the TinyPICO board, it blinks according to the state of the car, and state of the SD card

## Big Thanks

 * https://github.com/lishen2/isotp-c
   * handles large CAN bus packet reassembly required for large OBD2 packets
 * https://github.com/Esprit1st/Hyundai-Ioniq-5-Torque-Pro-PIDs/
   * provides PIDs and indicies of data field specifically for the IONIQ 5
 * https://github.com/FastLED/FastLED/
   * for the LED strip
 * https://github.com/me-no-dev/ESPAsyncWebServer
   * for the web server
 * https://jquery.com/ and https://jqueryui.com/
 * https://github.com/mlcheng/js-toast
 * https://canvasjs.com/

## Other Libraries I Wrote

 * command line
   * has a dictionary of command strings and assigned functions
   * each function can have arguments
 * multiple-destination `Print`
   * a way to call `printf` once, but have the result be printed to both websocket client and SD card file
 * websocket printer
   * a way to call `printf` to all websocket clients
 * StringPrinter
   * a cached string builder that can use `printf` to build a string
 * AsyncADC
   * non-blocking ADC reading for ESP32
