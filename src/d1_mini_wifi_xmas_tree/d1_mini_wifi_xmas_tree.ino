#include <Arduino.h>
#include <ArduinoOTA.h>
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FastLED.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

#define DEBUG

#ifdef DEBUG
 #define PRINT(x)     Serial.print (x)
 #define PRINTLN(x)  Serial.println (x)
#else
 #define PRINT(x)
 #define PRINTLN(x) 
#endif

const char* ssid = "munichmakerlab";
const char* password = "***topSecret***";
const char* hostName = "WiFi-LED-XMAS-tree";

const String backToMenuHtml = "<a href=\"/\">go back to menu</a>";

const int led = 13;

#if defined(FASTLED_VERSION) && (FASTLED_VERSION > 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN    D4
#define LED_TYPE    WS2812B
#define COLOR_ORDER GRB
#define NUM_LEDS    1
CRGB topLed[NUM_LEDS];

#define FULL_BRIGHTNESS  255
#define HALF_BRIGHTNESS  127
#define ONE_SECOND  1000

#define Red  D1
#define Yellow  D2
#define Green  D3

#define ANIM_OFF 0
#define ANIM_SIMPLE 1
#define ANIM_SINCOS 2
#define ANIM_CHEN 3

bool topOn = true;
bool animationTopOn = true;
bool redOn = true;
bool yellowOn = true;
bool greenOn = true;
uint8_t animationCycle = 2;
uint8_t outerAnimationType = ANIM_SINCOS;

const double CHEN_A = 36.0;
const double CHEN_B = 3.0;
const double CHEN_C = 20.0;
const double CHEN_M = 300.0;
const double CHEN_DT = .002;
double chenX = 0.01;
double chenY = 0.011;
double chenZ = 25.012;
double minChenX = -60.0;
double maxChenX = 40.0;
double minChenY = -80.0;
double maxChenY = 80.0;
double minChenZ = 0.0;
double maxChenZ = 100.0;

const uint8_t COLOR_RED = 0;
const uint8_t COLOR_GREEN = 96;

uint8_t gHue = 0;
uint8_t brightTop = FULL_BRIGHTNESS;
uint8_t brightRed = 0;
uint8_t brightYellow = 0;
uint8_t brightGreen = 0;

int loopCounter = 0;

ESP8266WebServer server(80);

void setup() {
  delay(3000);
  Serial.begin(115200);
  PRINTLN("Setting up...");
  Serial.printf("Using FASTLED version [%u]: ", FASTLED_VERSION);
  initTopLED();
  initOuterLEDs();
  setupServer();
  PRINTLN("...finished with setup");
}

void loop() {
  loopCounter++;
  server.handleClient();
  ArduinoOTA.handle();
  handleTreeTopLoop();
  handleLEDsLoop();
}

void initTopLED() {
  PRINTLN("Initializing top LED");
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(topLed, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(HALF_BRIGHTNESS);
  fill_rainbow(topLed, NUM_LEDS, COLOR_RED, 1);
  FastLED.show();  
}

void initOuterLEDs() {
  PRINTLN("Initializing outer LEDs");
  pinMode(Red, OUTPUT);
  pinMode(Green, OUTPUT);
  pinMode(Yellow, OUTPUT);  
}

void handleTreeTopLoop() {
  //topLed = CRGB::Red;
  if (animationTopOn) {
    gHue++;
  }
  if (!topOn) {
    FastLED.setBrightness(0);
  } else {
    FastLED.setBrightness(FULL_BRIGHTNESS);
  }
  // FastLED's built-in rainbow generator
  fill_rainbow(topLed, NUM_LEDS, gHue, 1);
  FastLED.show();
}

void handleLEDsLoop() {
  // TODO: determine LED animation algorithm
  switch(outerAnimationType) {
    case ANIM_SIMPLE:
      calculateSimpleAnimation();
      break;
    case ANIM_SINCOS:
      calculateSinCosAnimation();
      break;
    case ANIM_CHEN:
      calculateChenAnimation();
      break;
    default:
      break;
  }      
  // Check if one of the outer LEDs is configured to be turned off
  if (!redOn) {
    brightRed = 0;
  }
  if (!yellowOn) {
    brightYellow = 0;
  }
  if (!greenOn) {
    brightGreen = 0;
  }
  // write outputs to LEDs
  analogWrite(Red, brightRed);
  analogWrite(Yellow, brightYellow);
  analogWrite(Green, brightGreen);
}

// paper on chen system http://lsc.amss.ac.cn/~ljh/02LCZ2.pdf
void calculateChenAnimation() {
  chenX += (CHEN_A * ( chenY - chenX)) * CHEN_DT;
  chenY += ( -1.0 * (chenX * chenZ) + (CHEN_C * chenY)) * CHEN_DT;
  chenZ += ((chenX * chenY) - ( CHEN_B * chenZ) - CHEN_M) * CHEN_DT;
  
  minChenX = min(chenX, minChenX);
  minChenY = min(chenY, minChenY);
  minChenZ = min(chenZ, minChenZ);
  
  maxChenX = max(chenX, maxChenX);
  maxChenY = max(chenY, maxChenY);
  maxChenZ = max(chenZ, maxChenZ);
  
  brightRed = (Red, 255. * (chenX - minChenX) / (maxChenX - minChenX));
  brightYellow = (Yellow, 255. * (chenZ - minChenZ) / (maxChenZ - minChenZ));
  brightGreen = (Green, 255. * (chenY - minChenY) / (maxChenY - minChenY));
}

// animate LEDs by using smooth sine and cosine waves
void calculateSinCosAnimation() {
  EVERY_N_MILLISECONDS (5) {
    uint8_t redness = pow((1.0 + sin(.025 * loopCounter + 4.0 / 3.0 * M_PI)) / 2.0, 1.7) * 255.;
    uint8_t yellowness = pow((1.0 + sin(.02 * loopCounter + 2.0 / 3.0 * M_PI)) / 2.0, 1.7) * 255.;
    uint8_t greenness = pow((1.0 + sin(.015 * loopCounter)) / 2.0, 1.7) * 255.;
    brightRed = redness;
    brightYellow = yellowness;
    brightGreen = greenness;
  }
}

// simple yet effective edgy basic animation
void calculateSimpleAnimation() {
  EVERY_N_MILLISECONDS (5) {
    brightRed = brightRed - 3;
    brightYellow = brightYellow - 2;
    brightGreen = brightGreen - 1;
  }
}

void toggleLEDs() {
  redOn = !redOn;
  yellowOn = !yellowOn;
  greenOn = !greenOn;  
}

void setupServer() {
  WiFi.mode(WIFI_STA);
  WiFi.hostname(hostName);
  WiFi.begin(ssid, password);
  PRINTLN("Connecting");
  // Wait for connection
  long waiting = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(ONE_SECOND);
    waiting = waiting + ONE_SECOND;
    PRINT(".");
    if(waiting > (15 * ONE_SECOND)) {
      break;
    }
  }
  if(WiFi.status() != WL_CONNECTED) {
    PRINTLN("Unable to connect to WiFi. Starting standard blinking procedure...");
  }
  else {
    PRINT("Hostname: ");
    PRINTLN(hostName);
    PRINT("Connected ESP8266 to ");
    PRINTLN(ssid);
    PRINT("IP address: ");
    PRINTLN(WiFi.localIP());
  
    if (MDNS.begin("esp8266")) {
      PRINTLN("MDNS responder started");
    }
    server.on("/toggleleds", []() {
      digitalWrite(led, 1);
      PRINTLN("toggleLEDs received");
      server.send(200, "text/html", renderCommandMenu() + "toggle LEDs received <br/>");
      toggleLEDs();
      digitalWrite(led, 0);
    });
  
    server.on("/animationtop", []() {
      digitalWrite(led, 1);
      PRINTLN("animationtop received");
      animationTopOn = !animationTopOn;
      server.send(200, "text/html", renderCommandMenu() + "animationtop <br/>");
      digitalWrite(led, 0);
    });
  
    server.on("/toggletop", []() {
      digitalWrite(led, 1);
      PRINTLN("toggletop received");
      topOn = !topOn;
      server.send(200, "text/html", renderCommandMenu() + "toggletop <br/>");
      digitalWrite(led, 0);
    });
  
    server.on("/togglered", []() {
      digitalWrite(led, 1);
      PRINTLN("togglered received");
      redOn = !redOn;
      server.send(200, "text/html", renderCommandMenu() + "togglered <br/>");
      digitalWrite(led, 0);
    });
  
    server.on("/toggleyellow", []() {
      digitalWrite(led, 1);
      PRINTLN("toggleyellow received");
      yellowOn = !yellowOn;
      server.send(200, "text/html", renderCommandMenu() + "toggleyellow <br/>");
      digitalWrite(led, 0);
    });
  
    server.on("/togglegreen", []() {
      digitalWrite(led, 1);
      PRINTLN("togglegreen received");
      greenOn = !greenOn;
      server.send(200, "text/html", renderCommandMenu() + "togglegreen <br/>");
      digitalWrite(led, 0);
    });
  
    server.on("/greentree", []() {
      digitalWrite(led, 1);
      PRINTLN("greentree received");
      gHue = COLOR_GREEN;
      brightTop = FULL_BRIGHTNESS;
      animationTopOn = false;
      topOn = true;
      greenOn = true;
      redOn = false;
      yellowOn = false;
      server.send(200, "text/html", renderCommandMenu() + "greentree <br/>");
      digitalWrite(led, 0);
    });
  
    server.on("/off", []() {
      digitalWrite(led, 1);
      PRINTLN("off received");
      FastLED.setBrightness(0);
      topOn = false;
      greenOn = false;
      redOn = false;
      yellowOn = false;
      server.send(200, "text/html", renderCommandMenu() + "switching all lights off <br/>");
      digitalWrite(led, 0);
    });
  
    server.on("/on", []() {
      digitalWrite(led, 1);
      PRINTLN("on received - restoring default values");
      FastLED.setBrightness(FULL_BRIGHTNESS);
      topOn = true;
      animationTopOn = true;
      outerAnimationType = ANIM_SINCOS;
      greenOn = true;
      redOn = true;
      yellowOn = true;
      server.send(200, "text/html", renderCommandMenu() + "restoring default values <br/>" );
      digitalWrite(led, 0);
    });
  
    server.on("/simple", []() {
      digitalWrite(led, 1);
      PRINTLN("simple received");
      outerAnimationType = ANIM_SIMPLE;
      server.send(200, "text/html", renderCommandMenu() + "changing outer animation to simple<br/>" );
      digitalWrite(led, 0);
    });

    server.on("/sincos", []() {
      digitalWrite(led, 1);
      PRINTLN("sin cos received");
      outerAnimationType = ANIM_SINCOS;
      server.send(200, "text/html", renderCommandMenu() + "changing outer animation to smooth sine cosine wave<br/>" );
      digitalWrite(led, 0);
    });

    server.on("/chen", []() {
      digitalWrite(led, 1);
      PRINTLN("chen received");
      outerAnimationType = ANIM_CHEN;
      server.send(200, "text/html", renderCommandMenu() + "changing outer animation to random chen attractor<br/>" );
      digitalWrite(led, 0);
    });

    server.on("/stop", []() {
      digitalWrite(led, 1);
      PRINTLN("stop received");
      outerAnimationType = ANIM_OFF;
      server.send(200, "text/html", renderCommandMenu() + "stopping outer animation<br/>" );
      digitalWrite(led, 0);
    });

    server.on("/", []() {
      digitalWrite(led, 1);
      String commandMenu = renderCommandMenu();
      commandMenu += "<p>Challenge: If you manage to sync the leds to the song <a href=\"https://www.youtube.com/watch?v=rmgf60CI_ks\" target=\"_blank\">Trans-Siberian Orchestra: Wizards in Winter</a> until XMAS eve 2018, you will be rewarded either with a box of Club Mate or a cask of beer by Uli.</p>";
      server.send(200, "text/html", commandMenu);
      digitalWrite(led, 0);
    });
  
    server.onNotFound(handleNotFound);
    server.begin();
    PRINTLN("HTTP server started");
  
    ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH) {
        type = "sketch";
      } else { // U_SPIFFS
        type = "filesystem";
      }
    
      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      PRINTLN("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
      PRINTLN("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) {
        PRINTLN("Auth Failed");
      } else if (error == OTA_BEGIN_ERROR) {
        PRINTLN("Begin Failed");
      } else if (error == OTA_CONNECT_ERROR) {
        PRINTLN("Connect Failed");
      } else if (error == OTA_RECEIVE_ERROR) {
        PRINTLN("Receive Failed");
      } else if (error == OTA_END_ERROR) {
        PRINTLN("End Failed");
      }
    });
    ArduinoOTA.setHostname(hostName);
    ArduinoOTA.begin();
  }
}

String renderCommandMenu(){
    String menu = "<h1>Welcome to the LED WiFi XMAS TREE!</h1><br>";
    menu += "<p>The following commands are available:</p>";
    menu += "<ul>";
    menu += "<li><a href=\"/\">'/'</a>: show this page</li>";
    menu += "<li><a href=\"/off\">'/off'</a>: turn all lights off</li>";
    menu += "<li><a href=\"/on\">'/on'</a>: turn all lights on (default)</li>";
    menu += "<li><a href=\"/toggleleds\">'/toggleleds'</a>: toogle status of red, yellow and green leds</li>";
    menu += "<li><a href=\"/toggletop\">'/toggletop'</a>: turn top LED on/off</li>";
    menu += "<li><a href=\"/animationtop\">'/animationtop'</a>: stop/continue top LED animation</li>";
    menu += "<li><a href=\"/togglered\">'/togglered'</a>: turn red LEDs on/off</li>";
    menu += "<li><a href=\"/toggleyellow\">'/toggleyellow'</a>: turn yellow LEDs on/off</li>";
    menu += "<li><a href=\"/togglegreen\">'/togglegreen'</a>: turn green LEDs on/off</li>";
    menu += "<li><a href=\"/greentree\">'/greentree'</a>: make top LED green, activate outer green LEDS and deactivate other LEDS</li>";
    menu += "<li><a href=\"/simple\">'/simple'</a>, <a href=\"/sincos\">'/sincos'</a>, <a href=\"/chen\">'/chen'</a>, <a href=\"/stop\">'/stop'</a>: change animation of outer LEDs</li>";
    menu += "</ul>";
    menu += "<p><em><b>Merry Christmas =)</b></em></p>";
    return menu;
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "404 - File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}
