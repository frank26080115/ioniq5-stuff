function getTimeNum()
{
    // generates epoch time but with a local timezone offset
    // this is sent to the ESP32 so it's own internal time library can generate the correct local date string
    return Math.floor((Date.now() / 1000) - ((new Date()).getTimezoneOffset() * 60));
}
