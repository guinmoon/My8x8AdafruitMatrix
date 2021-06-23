// IF ESP-01 PIN
#define PIN 02

// IF NODEMCUv2 PIN
// #define PIN D1

// Define Your WiFi Connection Information
const char *WiFi_SSID = "gHomeNet_5G";
const char *WiFi_PASS = "Schooln7";

// const char* WiFi_SSID = "energom";
// const char* WiFi_PASS = "moysohn7";

int default_brightness = 15;

struct URL_Settigns
{
    char mode;
    byte R;
    byte G;
    byte B;
    const char *url;
};

const URL_Settigns request_urls[] = {{'t', 220, 80, 20, "http://192.168.1.45:1880/get_temp1"},
                                     {'t', 220, 180, 20, "http://192.168.1.45:1880/get_temp2"},
                                     {'h', 60, 80, 140, "http://192.168.1.45:1880/get_hum1"},
                                     {'t', 60, 180, 60, "http://192.168.1.45:1880/temp_w1"},
                                     {'w', 0, 0, 0, "http://192.168.1.45:1880/wmode"}};
