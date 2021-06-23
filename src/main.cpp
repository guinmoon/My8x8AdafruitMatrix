#include <string>
// Include NeoPixel Specific Libraries
#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_NeoMatrix.h>
//#include <Fonts/Tiny3x3a2pt7b.h>
#include <Fonts/Picopixel.h>

#include <ESP8266httpUpdate.h>
#include "ESP8266WiFi.h"
#include "ESP8266mDNS.h"
#include "WiFiUdp.h"

#include "ArduinoOTA.h"

// #include <HTTPClient.h>

#include <ESPAsyncTCP.h>
// #include <ESPAsyncWebServer.h>
#include <asyncHTTPrequest.h>

// #include <ArduinoJson.h>

#include <Ticker.h>

#include "espneotext.h"
#include "picture_sets.h"

#include "Config.h"

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(8, 8, PIN, NEO_MATRIX_LEFT, NEO_GRB + NEO_KHZ800);

Ticker get_data_ticker;

#define LED_BLACK 0

// Set Defaults
String dispText;
String dispColor;
int disp = 0;
int pixelsInText;
int x = matrix.width();
int scene_delay = 3000;
int after_wmode_delay = 1000;
bool Stop_Scene = false;

String Text_to_Show = "";
String Pixels_to_Show = "";
byte TColor[3] = {220, 180, 20};
int TextX = 0;

int data_refresh_delay = 5;

String anim_repeat_3 = "03";
String anim_repeat_6 = "06";

const int Requests_Count = sizeof(request_urls) / 8;
asyncHTTPrequest async_requests[Requests_Count];
int req_indexes[Requests_Count];
String Responses[Requests_Count];
byte TColors[Requests_Count][3];
char InfoTypres[Requests_Count];
// AsyncWebServer server(8080);

void init_anims()
{
    fire_anim = anim_repeat_3 + "|" + fire_anim;
    chupacabra = anim_repeat_3 + "|" + chupacabra;
    cloud = anim_repeat_3 + "|" + cloud;
    sun = anim_repeat_6 + "|" + sun;
    snow = anim_repeat_6 + "|" + snow;
}

String urlDecode(String input)
{
    String s = input;
    s.replace("%20", " ");
    s.replace("+", " ");
    s.replace("%21", "!");
    s.replace("%22", "\"");
    s.replace("%23", "#");
    s.replace("%24", "$");
    s.replace("%25", "%");
    s.replace("%26", "&");
    s.replace("%27", "\'");
    s.replace("%28", "(");
    s.replace("%29", ")");
    s.replace("%30", "*");
    s.replace("%31", "+");
    s.replace("%2C", ",");
    s.replace("%2E", ".");
    s.replace("%2F", "/");
    s.replace("%2C", ",");
    s.replace("%3A", ":");
    s.replace("%3A", ";");
    s.replace("%3C", "<");
    s.replace("%3D", "=");
    s.replace("%3E", ">");
    s.replace("%3F", "?");
    s.replace("%40", "@");
    s.replace("%5B", "[");
    s.replace("%5C", "\\");
    s.replace("%5D", "]");
    s.replace("%5E", "^");
    s.replace("%5F", "-");
    s.replace("%60", "`");
    return s;
}

std::vector<std::string> split_str(std::string input_str, std::string delimiter)
{
    std::vector<std::string> splited_str;
    size_t pos = 0;
    std::string part;
    // Serial.println("BEGIN SPLIT");
    while ((pos = input_str.find(delimiter)) != std::string::npos)
    {
        part = input_str.substr(0, pos);
        input_str.erase(0, pos + delimiter.length());
        splited_str.push_back(part);
    }
    splited_str.push_back(input_str);
    // Serial.println("DONE SPLIT");
    return splited_str;
}

void print_img(String pixels_full, bool only_prepare = false)
{
    int byte_pixel5[5];
    // Serial.println(pixels_full);
    std::vector<std::string> frames = split_str(pixels_full.c_str(), "|");
    int f_count = frames.size();
    int repeat_time = 1;
    char byte_num[2] = {0, 0};
    for (int loop_num = 0; loop_num < repeat_time; loop_num++)
    {
        for (int fnum = 0; fnum < f_count; fnum++)
        {
            std::string pixels_raw = frames[fnum];
            if (f_count > 1 && fnum == 0)
            { //если больше 1 кадра то 1й кадр это количество кадров
                char byte_num[2] = {pixels_raw[0], pixels_raw[1]};
                repeat_time = (int)strtol(byte_num, NULL, 16);
                continue;
            }
            int px_raw_len = pixels_raw.length();
            // Serial.println(pixels_raw.c_str());
            if (px_raw_len == 0)
            {
                matrix.fillScreen(0);
                continue;
            }
            for (int i = 0; i < px_raw_len - 7; i += 8)
            {
                byte_pixel5[0] = pixels_raw[i] - '0';
                byte_pixel5[1] = pixels_raw[i + 1] - '0';
                byte_num[0] = pixels_raw[i + 2];
                byte_num[1] = pixels_raw[i + 3];
                byte_pixel5[2] = (int)strtol(byte_num, NULL, 16);
                byte_num[0] = pixels_raw[i + 4];
                byte_num[1] = pixels_raw[i + 5];
                byte_pixel5[3] = (int)strtol(byte_num, NULL, 16);
                byte_num[0] = pixels_raw[i + 6];
                byte_num[1] = pixels_raw[i + 7];
                byte_pixel5[4] = (int)strtol(byte_num, NULL, 16);
                matrix.drawPixel(byte_pixel5[0], byte_pixel5[1], matrix.Color(byte_pixel5[2], byte_pixel5[3], byte_pixel5[4]));
            }
            if (!only_prepare)
                matrix.show();
            if (f_count > 1)
            {
                delay(200);
                matrix.fillScreen(0);
            }
        }
    }
}

void print_text(String text, int x, int R, int G, int B, bool only_prepare = false)
{
    matrix.setTextColor(matrix.Color(R, G, B));
    matrix.setCursor(x, 7);
    matrix.print(text);
    if (!only_prepare)
        matrix.show();
}

void get_data_sets_async()
{
    for (int r_ind = 0; r_ind < Requests_Count; r_ind++)
    {
        if (async_requests[r_ind].readyState() == 0 || async_requests[r_ind].readyState() == 4)
        {
            async_requests[r_ind].open("GET", request_urls[r_ind].url);
            async_requests[r_ind].send();
        }
    }
}

void requestCB(void *optParm, asyncHTTPrequest *request, int readyState)
{
    if (readyState == 4)
    {
        String resp_text = request->responseText();
        int req_index = *(int *)optParm;
        Serial.println(resp_text + " " + req_index);
        Responses[req_index] = resp_text;
    }
}

void get_and_show_payload(char mode, byte R, byte G, byte B, String text)
{
    // Serial.print(mode + " " + text + " ");
    // Serial.print(R);
    // Serial.print(G);
    // Serial.println(B);
    if (mode == 'w')
    {
        matrix.fillScreen(0);
        // print_img(fire_anim);
        // Serial.println("SHOW WMODE");
        if (text.indexOf("rain") > 0 || text.indexOf("drizzle") > 0)
        {
            print_img(cloud_rain);
        }
        else if (text.indexOf("cloud") > 0)
        {
            print_img(cloud);
        }
        else if (text.indexOf("snow") > 0)
        {
            print_img(snow);
        }
        else
        {
            print_img(sun);
        }
    }
    else
    {
        matrix.fillScreen(0);
        byte tcolor[3] = {R, G, B};
        int x = 0;
        if (mode == 't')
            print_img(temperature_sign, true);
        if (mode == 'h')
            print_img(humidity_sign, true);
        print_text(text, x, tcolor[0], tcolor[1], tcolor[2]);
    }
}

// void notFound(AsyncWebServerRequest *request)
// {
//     request->send(404, "text/plain", "Not found");
// }

// void parse_requset(AsyncWebServerRequest *request)
// {
//     String message = "empty";
//     bool fill_screen = false;
//     int bright = 15;
//     String pixels = "";
//     String text = "";
//     if (request->hasParam("fs"))
//     {
//         message = request->getParam("fs")->value();
//         fill_screen = true;
//     }
//     if (request->hasParam("b"))
//     {
//         message = request->getParam("b")->value();
//         bright = message.toInt();
//     }
//     if (request->hasParam("x"))
//     {
//         message = request->getParam("x")->value();
//         TextX = message.toInt();
//     }
//     if (request->hasParam("tcolor"))
//     {
//         message = request->getParam("x")->value();
//         std::vector<std::string> str_text_color = split_str(message.c_str(), ":");
//         TColor[0] = std::atoi(str_text_color[0].c_str());
//         TColor[1] = std::atoi(str_text_color[1].c_str());
//         TColor[2] = std::atoi(str_text_color[2].c_str());
//     }
//     if (request->hasParam("text"))
//     {
//         text = request->getParam("text")->value();
//     }
//     if (request->hasParam("pixels"))
//     {
//         pixels = request->getParam("pixels")->value();
//     }
//     request->send(200, "text/plain", "Hello, GET: " + message);
//     if (bright != 15)
//     {
//         matrix.setBrightness(bright);
//     }
//     if (pixels != "")
//     {
//         if (fill_screen)
//             matrix.fillScreen(0);
//         Pixels_to_Show = pixels;
//     }
//     if (text != "")
//     {
//         if (fill_screen)
//             matrix.fillScreen(0);
//         Text_to_Show = text;
//     }
// }

void setup()
{
    init_anims();
    for (int i = 0; i < Requests_Count; i++)
        req_indexes[i] = i;
    Serial.begin(115200);
    randomSeed(analogRead(0));
    matrix.begin();
    delay(10);
    WiFi.begin(WiFi_SSID, WiFi_PASS);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(300);
    }
    //OTA
    ArduinoOTA.onStart([]()
                       { Serial.println("Start"); });
    ArduinoOTA.onEnd([]()
                     { Serial.println("\nEnd"); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
                          { Serial.printf("Progress: %u%%\r", (progress / (total / 100))); });
    ArduinoOTA.onError([](ota_error_t error)
                       {
                           //        Serial.printf("Error[%u]: ", error);
                           if (error == OTA_AUTH_ERROR)
                               Serial.println("Auth Failed");
                           else if (error == OTA_BEGIN_ERROR)
                               Serial.println("Begin Failed");
                           else if (error == OTA_CONNECT_ERROR)
                               Serial.println("Connect Failed");
                           else if (error == OTA_RECEIVE_ERROR)
                               Serial.println("Receive Failed");
                           else if (error == OTA_END_ERROR)
                               Serial.println("End Failed");
                       });
    ArduinoOTA.begin();

    matrix.setBrightness(default_brightness);
    //  matrix.setTextColor(matrix.Color(180,20,20));
    matrix.setTextColor(matrix.Color(20, 180, 20));
    matrix.setCursor(1, 1);
    matrix.setTextWrap(false);
    matrix.setFont(&Picopixel);
    matrix.print("HI");
    matrix.show();

    // ASYNC WEB SERVER
    // server.on("/", HTTP_GET, [](AsyncWebServerRequest *in_request)
    //           { in_request->send(200, "text/plain", "Hello, world 8x8"); });
    // server.on("/get", HTTP_GET, [](AsyncWebServerRequest *in_request)
    //           { parse_requset(in_request); });
    // server.onNotFound(notFound);
    // server.begin();

    ///ИСПОЛЬЗОВАТЬ AsyncHTTPRequest_Generic
    for (int r_ind = 0; r_ind < Requests_Count; r_ind++)
        async_requests[r_ind].onReadyStateChange(requestCB, &req_indexes[r_ind]);
    get_data_ticker.attach(data_refresh_delay, get_data_sets_async);
}

void loop()
{
    ArduinoOTA.handle();
    if (Pixels_to_Show != "")
    {
        String tmp_px = Pixels_to_Show;
        Pixels_to_Show = "";
        print_img(tmp_px);
    }
    else if (Text_to_Show != "")
    {
        String tmp_tx = Text_to_Show;
        Text_to_Show = "";
        print_text(tmp_tx, TextX, TColor[0], TColor[1], TColor[2]);
    }
    else
    {
        int show_payload = random(0, 4);
        if (show_payload != 1)
        {
            for (int scene_ind = 0; scene_ind < Requests_Count; scene_ind++)
            {
                if (Text_to_Show != "" || Pixels_to_Show != "")
                {
                    return;
                }
                get_and_show_payload(request_urls[scene_ind].mode, request_urls[scene_ind].R, request_urls[scene_ind].G, request_urls[scene_ind].B, Responses[scene_ind]);
                if (request_urls[scene_ind].mode != 'w')
                    delay(scene_delay);
                else
                    delay(after_wmode_delay);
            }
        }
        else
        {
            if (Text_to_Show != "" || Pixels_to_Show != "")
            {
                return;
            }
            int anim_index = random(0, 2);
            if (anim_index == 0)
                print_img(chupacabra);
            if (anim_index == 1)
                print_img(fire_anim);
        }
    }
}
