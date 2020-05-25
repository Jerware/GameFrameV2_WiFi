/***************************************************
  Game Frame V3 Source Code
  Jeremy Williams, 5-21-2020

  NOTE: Altering your firmware voids your warranty. Have fun.

  BMP parsing code based on example sketch for the Adafruit
  1.8" SPI display library by Adafruit.
  http://www.adafruit.com/products/358

  In the SD card, place 24 bit color BMP files
****************************************************/
#define firmwareVersion 20200521 // firmware version

#include "SdFat.h"
#include "ds1307.h"
#include "FastLED.h"
FASTLED_USING_NAMESPACE;
#include "APA102Controller_WithBrightness.h"
#include "IRremote.h"
#include "IniFileLite.h"
#define WEBDUINO_FAVICON_DATA ""
#define WEBDUINO_SERIAL_DEBUGGING 0
#include "WebServerPM.h"
#include <math.h>
#include "effects.h"
#include "colorNameToRGB.h"
#include "HttpClient.h"

SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(SEMI_AUTOMATIC);
//SerialLogHandler logHandler(LOG_LEVEL_ALL); // get WIFI debug prints

// HTTP setup
HttpClient http;

// Headers currently need to be set at init, useful for API keys etc.
http_header_t headers[] = {
    //  { "Content-Type", "application/json" },
    //  { "Accept" , "application/json" },
    { "Accept" , "*/*"},
    { NULL, NULL } // NOTE: Always terminate headers will NULL
};

http_request_t request;
http_response_t response;

// function prototypes required by disabling the spark preprocessor
void setCycleTime();
void stripSetBrightness();
void clearStripBuffer();
void testScreen();
void sdErrorMessage(uint8_t r, uint8_t g, uint8_t b); // green = init, blue = read file, red = save remote codes, yellow = refreshImageDimensions
void readABC();
void initGameFrame();
void closeMyFile();
void applyCurrentABC();
bool readIniFile();
void readRemoteIni();
void irReceiver();
void recordIRCodes();
void drawFrame();
void getCurrentTime();
int minuteCounter(byte h, byte m);
void saveSettingsToEEPROM();
void yellowDot(byte x, byte y);
uint8_t getIndex(byte x, byte y);
void initClock();
void framePowerDown();
void runABC();
void mainLoop();
void breakoutLoop();
void nextImage();
void buttonDebounce();
void bmpDraw(char *filename, int x, int y);
void showClock();
void refreshImageDimensions(char *filename);
uint16_t read16(SdFile& f);
uint32_t read32(SdFile& f);
uint8_t dim8_jer( uint8_t x );
void printDigits(int digits);
void readClockIni();
void networkConnect();
void drawDigits();
void storeSecondHandColor();
void secondHand();
int secondsIntoHour();
void clockDigit_1();
void clockDigit_2();
void clockDigit_3();
void clockDigit_4();
void getSecondHandIndex();
float degToRad(float deg);
void swapXdirection();
void swapYdirection();
uint8_t getScreenIndex(byte x, byte y);
boolean winCheck();
void chdirFirework();
void syncClock(boolean force);
void indexCmd(WebServer &server, WebServer::ConnectionType type, char *, bool);
void commandCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete);
void playCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete);
void setCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete);
void failureCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete);
void uploadCmd(WebServer & server, WebServer::ConnectionType type, char * url_tail, bool tail_complete);
void helpCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete);
void processWebServer();
void WebServerLaunch();
void readHTML(WebServer &server, const String& htmlfile);
uint16_t get_mime_type_from_filename(const char* filename);
void toggleWebServerPriority();
void listFiles(WebServer &server, uint8_t flags, uint8_t indent);
void webServerDisplayManager();
void scrollAddress();
void setClockFace(byte face);
void storeGalleryState();
void resumeGalleryState();
int cloudCommand(String c);
int cloudNext(String c);
int cloudPower(String c);
int cloudPlayFolder(String c);
int cloudAlert(String c);
int cloudBrightness(String c);
int cloudColor(String c);
int cloudScore(String c);
void setTimeZone();
void saveTimeZone();
void systemRecover();
void setClock();
void setClockHour();
void setClockMinute();
void adjustClock();
void remoteTest();
void drawBox(uint8_t r, uint8_t g, uint8_t b);
void flashBox(uint8_t r, uint8_t g, uint8_t b);
void APANativeBrightnessCheck();
void runEffects();
void plasmaPrime();
void plasmaSlow();
void nextEffect();
int realRandom(int max);
int realRandom(int min, int max);
void colorNameToRGB(String colorName, byte *r, byte *g, byte *b);
void firmwareUpdate_handler(system_event_t event, int param, void* moredata);
void clearEEPROM();
void showCoindeskBTC();
void drawChart(float arry[]);
void getChart(char chartFunction[], char chartSymbol[], char chartInterval[]);
void refreshChart();
void refreshChartLatest();
void fill_with_gradient(CRGB top, CRGB bottom);
void fastledshow();
void fadeControl();
void checkUDP();
void autoDisplayModeManager();

// Pick an SPI configuration.
// See SPI configuration section below (comments are for photon).
#define SPI_CONFIGURATION 0
//------------------------------------------------------------------------------
// Setup SPI configuration.
#if SPI_CONFIGURATION == 0
// Primary SPI with DMA
// SCK => A3, MISO => A4, MOSI => A5, SS => A2 (default)
SdFat sd;
const uint8_t SD_CS = SS;
#elif SPI_CONFIGURATION == 1
// Secondary SPI with DMA
// SCK => D4, MISO => D3, MOSI => D2, SS => D1
SdFat sd(1);
const uint8_t SD_CS = D1;
#elif SPI_CONFIGURATION == 2
// Primary SPI with Arduino SPI library style byte I/O.
// SCK => A3, MISO => A4, MOSI => A5, SS => A2 (default)
SdFatLibSpi sd;
const uint8_t SD_CS = SS;
#elif SPI_CONFIGURATION == 3
// Software SPI.  Use any digital pins.
// MISO => D5, MOSI => D6, SCK => D7, SS => D0
SdFatSoftSpi<D5, D6, D7> sd;
const uint8_t SD_CS = D0;
#endif  // SPI_CONFIGURATION
//------------------------------------------------------------------------------

SdFile myFile; // set filesystem

#define BUFFPIXEL 32 // number of pixels to buffer when reading BMP files

#define RTC_SYNC_TIME (60 * 60 * 1000) // sync with Internet time once an hour

/* This creates an instance of the webserver.  By specifying a prefix
 * of "", all pages will be at the root of the server. */
#define PREFIX ""
WebServer webserver(PREFIX, 80);

// IR setup
const byte RECV_PIN = D6; // Formerly 4
IRrecv irrecv(RECV_PIN);
decode_results results;
char irCommand;
char irLastCommand;
boolean understood = false;

// LED setup
boolean APANativeBrightness = false; // true = hardware, false = software
#define DATA_PIN D2 // SmartMatrix D4 -> D2
#define CLOCK_PIN D4 // SmartMatrix D5 -> D4
#define NUM_LEDS 256
#define LED_TYPE APA102
#define COLOR_ORDER BGR
APA102Controller_WithBrightness<DATA_PIN, CLOCK_PIN, COLOR_ORDER, DATA_RATE_MHZ(30)> ledController;
CRGB leds[NUM_LEDS];
CRGB leds_buf[NUM_LEDS];

//Button setup
#define buttonNextPin D5  // "Next" button SmartMatrix D2 -> D5
#define buttonMenuPin D3  // "Menu" button

#define STATUS_LED A7 // D7 for Photon LED, A7 for PCB

//Enable verbose prints?
const boolean debugMode = false;

//rtc
RTC_DS1307 rtc;
DateTime now;

// UDP setup
// Sender ip and port
IPAddress ipAddress;
int port = -1;
int lastUpdate = 0;
unsigned int localPort = 8888;
long printTime = 0;
long lastOK = 0;
// A UDP instance to let us send and receive packets over UDP
UDP Udp;

//Global variables
boolean
pongclock = false, // pong clock mode
pong_reset = false,
pong_celebrate = false,
pong_scored_hour = false,
pong_scored_minute = false,
pong_ball_direction = 0,
displayModeCycle = false, // automaticly cycle through display modes
scoreBoard = false,
chartStock = false, // false = crypto, true = stock
firmwareUpdateReady = false, // use to speedup firmware updates when using system thread
wifiFolderFound = false, // found the /wifi folder on SD?
enableWifi = false, // enable wifi?
resumeFile = false, // are we mid-file when pausing play?
browserVerified = false,
webServerActive = false,
nestedRootIni = false, // config.ini in root of nested folders
clockIniRead = false,
framePowered = true, // virtual screen power, used w/remote power button
irMenuRepeat = false,
irNextRepeat = false,
irPowerRepeat = false,
abc = false, // auto brightness control toggle
logoPlayed = false, // plays logo animation correctly reardless of playMode
folderLoop = true, // animation looping
moveLoop = false, // translation/pan looping
buttonPressed = false, // control button check
buttonEnabled = true, // debounce guard
menuActive = false, // showing menus (set brightness, playback mode, etc.)
panoff = true, // movement scrolls off screen
singleGraphic = false, // single BMP file
singleGraphicBuffer = false, // buffer for alerts
abortImage = false, // image is corrupt; abort, retry, fail?
displayFolderCount = false, // output extra info to LEDs
statusLedState = false, // flicker tech
clockShown = false, // clock mode?
clockSet = true, // have we set the time?
clockSetBlink = false, // flicker digit
clockDigitSet = false, // is hours/minutes set?
enableSecondHand = true,
clockAnimationActive = false, // currently showing clock anim
clockAdjustState = false, // is clock in an adjustment state?
hour12 = true, // 12-hour clock?
finishBeforeProgressing = false, // finish the animation before progressing?
timerLapsed = false, // timer lapsed, queue next animation when current one finishes
breakout = false, // breakout playing?
ballMoving = false,
galleryStateStored = false, // state stored?
gameInitialized = false;

uint8_t
pong_speed = 100,
pong_paddle_left_y = 0,
pong_paddle_right_y = 0,
pong_paddle_left_start = 0,
pong_paddle_right_start = 0,
pong_paddle_left_target = 0,
pong_paddle_right_target = 0,
chartStyle = 1, // 0 = area, 1 = candles
readPhase = 0, // debug info for reporting reading from SD
alertPhase = 0, // 0 = inactive, 1 = running, 2 = ending
cylonLastLED = 0,
cylonHUE = 0, // I'm alive indicator during web server
cylonSineValue = 0,
abcMinute, // last minute ABC was checked
playMode = 0, // 0 = sequential, 1 = random, 2 = pause animations
displayMode = 0, // 0 = slideshow, 1 = clock, 2 = fx, 3 = chart
displayModeMax = 1, // max display mode
brightness = 4, // LED brightness
brightnessMultiplier = 8, // DO NOT CHANGE THIS!
cycleTimeSetting = 2, // time before next animation: 1=10 secs, 2=30 secs, 3=1 min... 8=infinity
menuMode = 0, // 0 = brightness, 1 = play mode, 2 = cycle time
clockAnimationLength = 5, // seconds to play clock animations
secondHandX = 0,
secondHandY = 0,
secondOffset = 0,
lastSecond = 255, // a moment ago
currentHour = 12,
currentMinute = 0,
currentSecond = 255, // current second
paddleIndex = 230,
ballX = 112,
ballY = 208,
ballIndex = 216;

int16_t
folderIndex = 0; // current folder

int
delayPaddle = 0, // ms delay for movement -- set in game code
delayBall = 0, // ms delay for ball -- set in game code
fadeLengthGlobal = 0, // global value
fadeLength = 0, // length in ms for fade up effect
currentEffect = 0, // current effect (plasma, etc.)
secondCounter = 0, // counts up every second
secondCounterBuffer = 0, // buffered when entering alert
chartRefresh = 0, // # of seconds to refresh chart
fileIndex = 0, // current frame
fileIndexBuffer = 0, // store index during alerts
chainIndex = -1, // for chaining multiple folders
chainIndexBuffer = -1, // buffer for alerts
numFolders = 0, // number of folders on sd
cycleTime = 30, // seconds to wait before progressing to next folder
clockAdjust = 0, // seconds to adjust clock every 24 hours
offsetBufferX = 0, // for storing offset when entering menu
offsetBufferY = 0, // for storing offset when entering menu
offsetSpeedX = 0, // number of pixels to translate each frame
offsetSpeedY = 0, // number of pixels to translate each frame
offsetX = 0, // for translating images x pixels
offsetY = 0, // for translating images y pixels
imageWidth = 0,
imageHeight = 0,
imageWidthBuffer = 0,
imageHeightBuffer = 0,
ballAngle;

float
timeZone = -7.0f,
chartValues[18] = {0}; // arrray to store stock/coin values

unsigned long
pong_celebration_end = 0,
pong_showtime = 0,
displayModeCycleTime = 0, // Time to auto change display mode
displayModeCycleLength = 15000, // Delay length in ms for auto display mode
moveBall = 0, // time to move the ball
movePaddle = 0, // time to move the paddle
fadeStartTime = 0, // millis to end fade
prevRemoteMillis = 0, // last time a valid remote code was received
lastRTCCheck = 0, // last DS3231 RTC check timestamp
currentFilePosition = 0, // stores current position when using web server
cylonTime = 0, // used for web server cylon delay
lastSync = millis(), // last time sync
colorCorrection = 0xFFFFFF, // color correction setting
colorTemperature = 0xFFFFFF, // color Temperature
remoteCodeMenu = 2155864095, // defaults, overridden by remote.ini
remoteCodeNext = 2155831455,
remoteCodePower = 2155819215,
menuPowerCounter = 0, // counter for holding menu button to turn off power
lastTime = 0, // used to calculate draw time
drawTime = 0, // time to read from sd
holdTime = 200, // millisecods to hold each .bmp frame
swapTime = 0, // system time to advance to next frame
baseTime = 0, // system time logged at start of each new image sequence
buttonTime = 0, // time the last button was pressed (debounce code)
menuEndTime = 0, // pause animation while in menu mode
menuEnterTime = 0; // time we enter menu

char
scoreString[32], // xx-xx#FFFFFF#FFFFFF for displaying a game score
currentFile[13], // stores active file when accessing web server
uploadTargetFolder[32], // temp
clockFace[25], // clock face
chainRootFolder[9], // chain game
nextFolder[32], // dictated next animation
currentDirectory[32], // store active folder when accessing web server
chartSymbol[10] = "BTC"; // stock/coin

CRGB secondHandColor = 0; // color grabbed from digits.bmp for second hand
CRGB secondHands[64]; // second hand colors

// globals for automatic brightness control
struct abc {
  int m; // minute
  byte b; // brightness
};

typedef struct abc Abc;
Abc abc0 { -1, 0};
Abc abc1 { -1, 0};
Abc abc2 { -1, 0};
Abc abc3 { -1, 0};
Abc abc4 { -1, 0};
Abc abc5 { -1, 0};
Abc abc6 { -1, 0};
Abc abc7 { -1, 0};
Abc abc8 { -1, 0};
Abc abc9 { -1, 0};

// setup application watchdog to reboot if something freezes
ApplicationWatchdog wd(60000, systemRecover);

/*
EEPROM MAP
0 = brightness
1 = playMode
2 = cycleTime
3 = displayMode
4 = clockSet
5 = clockFace
6 = timezone
7 = pongclock
10 = networkPass
80 = networkSSID
120 = networkAuth
124 = networkCipher
130 = staticIP
134 = netmask
138 = gateway
142 = dns
146 = chartStyle (not stored)
147 = chartStock (boolean)
148 = chartSymbol (10 bytes)
201 = power state (128 = off, 255 = on)
*/

void systemRecover()
{
  Particle.publish("systemStatus","recover");
  Serial.print("Crash! Read phase: ");
  Serial.println(readPhase);

  System.reset();
}

void firmwareUpdate_handler(system_event_t event, int param, void* moredata)
{
  if (param==firmware_update_failed)
  {
    firmwareUpdateReady = false;
    flashBox(255, 0, 0);
  }
  else firmwareUpdateReady = true;
}

void setup(void) {
  // register the cloud functions
  Particle.function("Command", cloudCommand);
  Particle.function("Next", cloudNext);
  Particle.function("Power", cloudPower);
  Particle.function("Play", cloudPlayFolder);
  Particle.function("Alert", cloudAlert);
  Particle.function("Brightness", cloudBrightness);
  Particle.function("Color", cloudColor);

  System.on(firmware_update, firmwareUpdate_handler);

  // debug LED setup
  pinMode(STATUS_LED, OUTPUT);
  digitalWrite(STATUS_LED, HIGH);
  pinMode(D7, OUTPUT); // turn on Photon LED for debug
  digitalWrite(D7, HIGH);

  pinMode(buttonNextPin, INPUT_PULLUP);    // button as input
  pinMode(buttonMenuPin, INPUT_PULLUP);    // button as input

  delay(5000);
  if (debugMode == true)
  {
    Serial.begin(57600);
    delay(2000);
    Serial.println("Hello there!");
  }

  // IR enable
  irrecv.enableIRIn();

  // LED Init
  FastLED.addLeds((CLEDController*) &ledController, leds, NUM_LEDS).setDither(0);
  stripSetBrightness();
  clearStripBuffer();
  fastledshow();

  // run burn in test if both tactile buttons held on cold boot
  if ((digitalRead(buttonNextPin) == LOW) && (digitalRead(buttonMenuPin) == LOW))
  {
    brightness = 7;
    stripSetBrightness();
    while (true)
    {
      testScreen();
    }
  }

  // revert to these values if MENU tactile button held on cold boot
  if (digitalRead(buttonMenuPin) == LOW)
  {
    clearEEPROM();
    brightness = 1;
    stripSetBrightness();
    playMode = 0;
    cycleTimeSetting = 2;
    setCycleTime();
    displayMode = 0;
  }

  // SD Init
  Serial.print("Init SD: ");
  if (!sd.begin(SD_CS, SPI_HALF_SPEED)) {
    Serial.println("fail");
    // SD error message
    sdErrorMessage(0, 255, 0);
    return;
  }
  Serial.println("OK!");

  APANativeBrightnessCheck();

  // load automatic brightness control settings
  readABC();

  FastLED.setCorrection(colorCorrection);
  FastLED.setTemperature(colorTemperature);

  // read wifi .INI and connect to network
  networkConnect();
  if (!wifiFolderFound)
  {
    // flash orange box
    flashBox(255, 165, 0);
  }

  if (Particle.connected())
  {
    // get time zone
    EEPROM.get(6, timeZone);
    setTimeZone();
    // web server setup
    WebServerLaunch();
  }
  else
  {
    Wire.begin();
    rtc.begin();
    now = rtc.now();
    if (!rtc.isrunning()) {
      Serial.println("RTC is NOT running!");
      rtc.adjust(DateTime(2016, 1, 1, 12, 0, 0)); // year, month, day, hour, min, sec
    }
  }

  // enable effects?
  if (sd.exists("/00system/mode_2.bmp"))
  {
    displayModeMax = 2;
  }

  // enable chart?
  if (sd.exists("/00system/mode_3.bmp"))
  {
    displayModeMax = 3;
  }

  byte output = 0;

  // load last settings
  // read brightness setting from EEPROM
  output = EEPROM.read(0);
  if (output >= 1 && output <= 7) brightness = output;
  stripSetBrightness();

  // read playMode setting from EEPROM
  output = EEPROM.read(1);
  if (output >= 0 && output <= 2) playMode = output;

  // read cycleTimeSetting setting from EEPROM
  output = EEPROM.read(2);
  if (output >= 1 && output <= 8) cycleTimeSetting = output;
  setCycleTime();

  // read displayMode setting from EEPROM
  output = EEPROM.read(3);
  if (output >= 0 && output <= displayModeMax) displayMode = output;

  // DS3231 EEPROM check
  output = EEPROM.read(4);
  if (output == 1) clockSet = true;

  // read clock face setting from EEPROM
  output = EEPROM.read(5);
  if (output >= 1 && output <= 5) setClockFace(output);
  else setClockFace(1);

  // read clock face setting from EEPROM
  output = EEPROM.read(7);
  if (output >= 0 && output <= 1) pongclock = output;
  else pongclock = false;

  char eeSymbol[10];
  EEPROM.get(148, eeSymbol);
  if (isDigit(eeSymbol[0]) || isAlpha(eeSymbol[0])) // seems legit
  {
    strcpy(chartSymbol, eeSymbol);
    EEPROM.get(147, chartStock);
  }

  // load IR codes
  readRemoteIni();

  // Init stuff
  initGameFrame();

  if (Particle.connected())
  {
    Particle.publish("systemStatus","boot");
  }
  Udp.begin(localPort);
  digitalWrite(D7, LOW); // turn off Photon LED
}

void setTimeZone()
{
  if (timeZone >= -12 || timeZone <= 13)
  {
    Time.zone(timeZone);
  }
}

void saveTimeZone()
{
  if (timeZone >= -12 || timeZone <= 13)
  {
    EEPROM.put(6, timeZone);
  }
}

// This runs every time power is restored (cold or warm boot)
void initGameFrame()
{
  Serial.println("Init Game Frame");
  uint8_t powerState;
  EEPROM.get(201, powerState);
  // scroll the IP address across the screen (if not recovering from power down state)
  if (Particle.connected() && powerState != 128)
  {
    scrollAddress();
  }

  closeMyFile();
  // apply automatic brightness if enabled
  if (abc) applyCurrentABC();
  // reset vars
//  currentEffect = 0;
  secondCounter = 0;
  chartRefresh = 0;
  menuMode = 0;
  displayFolderCount = false;
  webServerActive = false;
  logoPlayed = false;
  fileIndex = 0;
  offsetX = 0;
  offsetY = 0;
  folderIndex = 0;
  singleGraphic = false;
  clockAnimationActive = false;
  clockShown = false;
  chainIndex = -1;
  nextFolder[0] = '\0';
  sd.chdir("/");

  // show test screens and folder count if NEXT tactile button held on boot
  if (digitalRead(buttonNextPin) == LOW)
  {
    displayFolderCount = true;
    testScreen();
  }

  char folder[13];

  if (numFolders == 0 || displayFolderCount)
  {
    numFolders = 0;
    // file indexes appear to loop after 2048
    for (int fileIndex = 0; fileIndex < 2048; fileIndex++)
    {
      myFile.open(sd.vwd(), fileIndex, O_READ);
      if (myFile.isDir()) {
        Serial.println(F("---"));
        if (displayFolderCount == true)
        {
          leds[numFolders] = CRGB(128, 255, 0);
          fastledshow();
        }
        numFolders++;
        Serial.print(F("File Index: "));
        Serial.println(fileIndex);
        myFile.getName(folder, 13);
        Serial.print(F("Folder: "));
        Serial.println(folder);
        closeMyFile();
      }
      else closeMyFile();
    }
    Serial.print(numFolders);
    Serial.println(F(" folders found."));
  }
  if (displayFolderCount == true)
  {
    delay(5000);
    irrecv.resume(); // Receive the next value
    remoteTest();
  }

  // sync clock with RTC if offline
  if (!Particle.connected()) syncClock(true);

  // check for reboot recovery from off state
  if (powerState == 128) framePowered = false;
  else framePowered = true;
  if (!framePowered)
  {
    framePowerDown();
  }

  else
  {
    // play logo animation
    sd.chdir("/00system/logo");
    readIniFile();
    drawFrame();
  }
}

void applyCurrentABC()
{
  getCurrentTime();
  int dayInMinutes = minuteCounter(currentHour, currentMinute);
  int newBrightness = -1;
  int triggerTimes[10] = {abc0.m, abc1.m, abc2.m, abc3.m, abc4.m, abc5.m, abc6.m, abc7.m, abc8.m, abc9.m};
  byte BrightnessSettings[10] = {abc0.b, abc1.b, abc2.b, abc3.b, abc4.b, abc5.b, abc6.b, abc7.b, abc8.b, abc9.b};
  for (byte x = 0; x < 9; x++)
  {
    if (dayInMinutes > triggerTimes[x] && triggerTimes[x] != -1) newBrightness = BrightnessSettings[x];
  }
  if (newBrightness == -1)
  {
    // current time is earlier than any triggers; use latest trigger before midnight
    newBrightness = BrightnessSettings[0];
    for (byte x = 1; x < 9; x++)
    {
      if (triggerTimes[x] > triggerTimes[x - 1]) newBrightness = BrightnessSettings[x];
    }
  }
  if (newBrightness > -1)
  {
    if (newBrightness == 0) newBrightness = 1;
    brightness = newBrightness;
    stripSetBrightness();
    saveSettingsToEEPROM();
  }
}

void stripSetBrightness()
{
  if (brightness > 7) brightness = 7;
  else if (brightness < 0) brightness = 0;

  // APA102 native brightness
  if (APANativeBrightness)
  {
    ledController.setAPA102Brightness(brightness);
    FastLED.setBrightness(255);
  }

  // FastLED brightness
  else FastLED.setBrightness(brightness * brightnessMultiplier);
  if (brightness == 0)
  {
    clearStripBuffer();
    fastledshow();
  }
}

void remoteTest()
{
  alertPhase = 1; // disable alerts
  clearStripBuffer();
  fastledshow();
  sd.chdir("/00system");
  int graphicShown = 0;
  bmpDraw("irZapr.bmp", 0, 0);
  long nextIRCheck = 0;
  while (true)
  {
    Particle.process();
    irReceiver();
    if (irCommand == 'P' || irCommand == 'M' || irCommand == 'N' || irPowerRepeat || irMenuRepeat || irNextRepeat)
    {
      nextIRCheck = millis() + 125;
      if (irCommand == 'P' || irPowerRepeat && graphicShown != 1)
      {
        bmpDraw("irPowr.bmp", 0, 0);
        graphicShown = 1;
      }
      else if (irCommand == 'M' || irMenuRepeat && graphicShown != 2)
      {
        bmpDraw("irMenu.bmp", 0, 0);
        graphicShown = 2;
      }
      else if (irCommand == 'N' || irNextRepeat && graphicShown != 3)
      {
        bmpDraw("irNext.bmp", 0, 0);
        graphicShown = 3;
      }
    }
    // no remote button pressed
    if (millis() > nextIRCheck && graphicShown != 0)
    {
      bmpDraw("irZapr.bmp", 0, 0);
      graphicShown = 0;
    }
    // record new IR codes if NEXT tactile button is presssed
    if (digitalRead(buttonNextPin) == LOW)
    {
      recordIRCodes();
      bmpDraw("irZapr.bmp", 0, 0);
    }
    // exit if MENU tactile button is presssed
    if (digitalRead(buttonMenuPin) == LOW)
    {
      alertPhase = 0; // enable alerts
      return;
    }
  }
}

// flash a box
void flashBox(uint8_t r, uint8_t g, uint8_t b)
{
  for (int i=0; i<8; i++)
  {
    drawBox(r, g, g);
    delay(75);
    drawBox(0, 0, 0);
    delay(75);
  }
}

// draw a box around the border of the screen
void drawBox(uint8_t r, uint8_t g, uint8_t b)
{
  for (int i = 0; i < 15; i++)
  {
    leds[i] = CRGB(r, g, b);
  }
  for (int i = 0; i < 16; i++)
  {
    leds[getIndex(16, i)] = CRGB(r, g, b);
  }
  for (int i = 0; i < 15; i++)
  {
    leds[getIndex(i, 15)] = CRGB(r, g, b);
  }
  for (int i = 15; i > 0; i--)
  {
    leds[getIndex(0, i)] = CRGB(r, g, b);
  }
  fastledshow();
}

void recordIRCodes()
{
  remoteCodeMenu = 0;
  remoteCodeNext = 0;
  remoteCodePower = 0;

  // record POWR
  bmpDraw("irPowr.bmp", 0, 0);
  drawBox(255, 0, 0);
  while (remoteCodePower == 0)
  {
    if (irrecv.decode(&results)) {
      if (results.decode_type == NEC && results.value != 4294967295) // ignore repeat code (0xFFFFFFFF)
      {
        Serial.print("IR code: ");
        Serial.println(results.value, DEC);
        remoteCodePower = results.value;
      }
      irrecv.resume(); // Receive the next value
    }
  }

  // record MENU
  bmpDraw("irMenu.bmp", 0, 0);
  drawBox(255, 0, 0);
  while (remoteCodeMenu == 0)
  {
    if (irrecv.decode(&results)) {
      if (results.decode_type == NEC && results.value != 4294967295 && results.value != remoteCodePower) // ignore repeat code (0xFFFFFFFF)
      {
        Serial.print("IR code: ");
        Serial.println(results.value, DEC);
        remoteCodeMenu = results.value;
      }
      irrecv.resume(); // Receive the next value
    }
  }

  // record NEXT
  bmpDraw("irNext.bmp", 0, 0);
  drawBox(255, 0, 0);
  while (remoteCodeNext == 0)
  {
    if (irrecv.decode(&results)) {
      if (results.decode_type == NEC && results.value != 4294967295 && results.value != remoteCodePower && results.value != remoteCodeMenu) // ignore repeat code (0xFFFFFFFF)
      {
        Serial.print("IR code: ");
        Serial.println(results.value, DEC);
        remoteCodeNext = results.value;
      }
      irrecv.resume(); // Receive the next value
    }
  }

  // save codes to SD
  if (!myFile.open("remote.ini", O_CREAT | O_RDWR)) {
    Serial.println("File open failed");
    sdErrorMessage(255, 0, 0);
    return;
  }
  myFile.rewind();
  myFile.seekSet(352);
  myFile.println("[remote]");
  myFile.println("");
  myFile.println("# Learned Codes");
  myFile.print("power = ");
  myFile.println(remoteCodePower);
  myFile.print("menu = ");
  myFile.println(remoteCodeMenu);
  myFile.print("next = ");
  myFile.println(remoteCodeNext);
  closeMyFile();
}

void printRemoteCode()
{
  if (irrecv.decode(&results)) {
    Serial.print("IR code: ");
    Serial.println(results.value, DEC);
    irrecv.resume(); // Receive the next value
  }
}

void testScreen()
{
  // white
  for (int i = 0; i < 256; i++)
  {
    leds[i] = CRGB(255, 255, 255);
  }
  fastledshow();
  delay(5000);

  // red
  for (int i = 0; i < 256; i++)
  {
    leds[i] = CRGB(255, 0, 0);
  }
  fastledshow();
  delay(1000);

  // green
  for (int i = 0; i < 256; i++)
  {
    leds[i] = CRGB(0, 255, 0);
  }
  fastledshow();
  delay(1000);

  // blue
  for (int i = 0; i < 256; i++)
  {
    leds[i] = CRGB(0, 0, 255);
  }
  fastledshow();
  delay(1000);
}

void sdErrorMessage(uint8_t r, uint8_t g, uint8_t b)
{
  // red bars
  for (int index = 64; index < 80; index++)
  {
    leds[index] = CRGB(r, g, b);
  }
  for (int index = 80; index < 192; index++)
  {
    leds[index] = CRGB(0, 0, 0);
  }
  for (int index = 192; index < 208; index++)
  {
    leds[index] = CRGB(r, g, b);
  }
  // S
  yellowDot(7, 6);
  yellowDot(6, 6);
  yellowDot(5, 6);
  yellowDot(4, 7);
  yellowDot(5, 8);
  yellowDot(6, 8);
  yellowDot(7, 9);
  yellowDot(6, 10);
  yellowDot(5, 10);
  yellowDot(4, 10);

  // D
  yellowDot(9, 6);
  yellowDot(10, 6);
  yellowDot(11, 7);
  yellowDot(11, 8);
  yellowDot(11, 9);
  yellowDot(10, 10);
  yellowDot(9, 10);
  yellowDot(9, 7);
  yellowDot(9, 8);
  yellowDot(9, 9);

  stripSetBrightness();
  fastledshow();

  while (true)
  {
    for (int i = 255; i >= 0; i--)
    {
      analogWrite(STATUS_LED, i);
      delay(1);
    }
    for (int i = 0; i <= 254; i++)
    {
      analogWrite(STATUS_LED, i);
      delay(1);
    }

    // simple blink
    /*digitalWrite(STATUS_LED, LOW);
    delay(500);
    digitalWrite(STATUS_LED, HIGH);
    delay(500);*/
  }
}

void yellowDot(byte x, byte y)
{
  leds[getIndex(x, y)] = CRGB(255, 255, 0);
}

void setCycleTime()
{
  if (cycleTimeSetting == 2)
  {
    cycleTime = 30;
  }
  else if (cycleTimeSetting == 3)
  {
    cycleTime = 60;
  }
  else if (cycleTimeSetting == 4)
  {
    cycleTime = 300;
  }
  else if (cycleTimeSetting == 5)
  {
    cycleTime = 900;
  }
  else if (cycleTimeSetting == 6)
  {
    cycleTime = 1800;
  }
  else if (cycleTimeSetting == 7)
  {
    cycleTime = 3600;
  }
  else if (cycleTimeSetting == 8)
  {
    cycleTime = -1;
  }
  else
  {
    cycleTime = 10;
  }
}

void statusLedFlicker()
{
  if (statusLedState == false)
  {
    digitalWrite(STATUS_LED, LOW);
  }
  else
  {
    digitalWrite(STATUS_LED, HIGH);
  }
  statusLedState = !statusLedState;
}

void saveSettingsToEEPROM()
{
  // save any new settings to EEPROM
  EEPROM.update(0, brightness);
  EEPROM.update(1, playMode);
  EEPROM.update(2, cycleTimeSetting);
  EEPROM.update(3, displayMode);
}

void irReceiver()
{
  irMenuRepeat = false;
  irNextRepeat = false;
  irPowerRepeat = false;
  irCommand = 'Z';
  if (irrecv.decode(&results)) {
    if (debugMode)
    {
      Serial.print("IR code received: ");
      Serial.println(results.value);
    }
    understood = true;
    // menu
    if (results.value == remoteCodeMenu)
    {
      irCommand = 'M';
      prevRemoteMillis = millis();
    }
    // next
    else if (results.value == remoteCodeNext)
    {
      irCommand = 'N';
      prevRemoteMillis = millis();
    }
    // power
    else if (results.value == remoteCodePower)
    {
      irCommand = 'P';
      prevRemoteMillis = millis();
    }
    // repeat/held
    else if (results.value == 4294967295 && millis() - prevRemoteMillis < 250) // 250 allows us to survive missing one signal
    {
      irCommand = 'R';
      prevRemoteMillis = millis();
      if (irLastCommand == 'M') irMenuRepeat = true;
      else if (irLastCommand == 'N') irNextRepeat = true;
      else if (irLastCommand == 'P') irPowerRepeat = true;
    }
    else understood = false;
    if (understood)
    {
      Serial.print("IR code interpreted as: ");
      Serial.println(irCommand);
      if (irCommand != 'R') irLastCommand = irCommand;
    }

    irrecv.resume(); // Receive the next value
  }
}

void powerControl()
{
  if (firmwareUpdateReady)
  {
    // draw WIFI graphic
    if (!framePowered) EEPROM.update(201, 255); // store power on state
    closeMyFile();
    offsetX = 0;
    offsetY = 0;
    singleGraphic = false;
    Serial.println("Firmware update!");
    bmpDraw("/00system/wifi/wifi.bmp", 0, 0);
    fastledshow();
    while(firmwareUpdateReady)
    {
      // update and chill
      Particle.process();
    }
  }
  if (irCommand == 'P')
  {
    // abc brightness set to zero; turn display on
    if (brightness == 0)
    {
      brightness = 1;
      stripSetBrightness();
      if (displayMode == 0) drawFrame(); // refrech the screen
      else initClock();
    }
    // toggle system power
    else
    {
      framePowered = !framePowered;
      // power down
      if (!framePowered)
      {
        framePowerDown();
      }
      // power restored
      else
      {
        EEPROM.update(201, 255); // store power state
        initGameFrame();
      }
    }
  }
  // allow power toggle by holding down physical Menu button on PCB
  if (framePowered)
  {
    if (millis() > menuPowerCounter && digitalRead(buttonMenuPin) == LOW && menuActive)
    {
      framePowered = false;
      framePowerDown();
    }
  }
  // reset MENU button after power off
  else if (digitalRead(buttonMenuPin) == HIGH && buttonPressed == true) buttonPressed = false;
  // power on
  else if (digitalRead(buttonMenuPin) == LOW && buttonPressed == false)
  {
    framePowered = true;
  }
}

void framePowerDown()
{
  Serial.println("Powering down.");
  webServerActive = false;
  menuActive = false;
  if (breakout == true)
  {
    breakout = false;
    ballMoving = false;
  }
  EEPROM.update(201, 128); // store power down state
  clearStripBuffer();
  fastledshow();
}

void loop() {
  irReceiver();
  checkUDP();
  processWebServer();
  powerControl();
  getCurrentTime();
  syncClock(false);

  if (framePowered)
  {
    runABC();
    if (brightness > 0 && !webServerActive)
    {
      if (breakout == false)
      {
        mainLoop();
      }

      else
      {
        breakoutLoop();
        if (breakout == false)
        {
          // exit chaining if necessary
          if (chainIndex > -1)
          {
            chainIndex = -1;
            chainRootFolder[0] = '\0';
            sd.chdir("/");
          }
          if (displayMode == 0)
          {
            nextImage();
            drawFrame();
          }
          else if (displayMode == 1) initClock();
          else if (displayMode == 3) drawChart(chartValues);
        }
      }
    }
  }
}

void mainLoop()
{
  buttonDebounce();

  // next button
  if ((digitalRead(buttonNextPin) == LOW || irCommand == 'N') && buttonPressed == false && buttonEnabled == true && !clockShown)
  {
    buttonPressed = true;
    if (menuActive == false)
    {
      Serial.println("Next Button Pressed.");
      scoreBoard = false; // just in case

      // exit chaining if necessary
      if (chainIndex > -1)
      {
        chainIndex = -1;
        chainRootFolder[0] = '\0';
        sd.chdir("/");
      }
      if (displayMode == 0)
      {
        nextImage();
        drawFrame();
      }
      else if (displayMode == 1)
      {
        // just displayed logo, enter clock mode
        initClock();
        return;
      }
      else if (displayMode == 2)
      {
        nextEffect();
        return;
      }
      else if (displayMode == 3)
      {
        chartStyle++;
        if (chartStyle > 1) chartStyle = 0;
        drawChart(chartValues);
      }
    }
    else
    {
      menuEndTime = millis() + 3000;

      // adjust brightness
      if (menuMode == 0)
      {
        brightness += 1;
        if (brightness > 7) brightness = 1;
        char brightChar[2];
        char brightFile[23];
        strcpy_P(brightFile, PSTR("/00system/bright_"));
        itoa(brightness, brightChar, 10);
        strcat(brightFile, brightChar);
        strcat(brightFile, ".bmp");
        stripSetBrightness();
        bmpDraw(brightFile, 0, 0);
      }

      // adjust play mode
      else if (menuMode == 1)
      {
        playMode++;
        if (playMode > 2) playMode = 0;
        char playChar[2];
        char playFile[21];
        strcpy_P(playFile, PSTR("/00system/play_"));
        itoa(playMode, playChar, 10);
        strcat(playFile, playChar);
        strcat(playFile, ".bmp");
        bmpDraw(playFile, 0, 0);
      }

      // adjust cycle time
      else if (menuMode == 2)
      {
        cycleTimeSetting++;
        if (cycleTimeSetting > 8) cycleTimeSetting = 1;
        setCycleTime();
        char timeChar[2];
        char timeFile[21];
        strcpy_P(timeFile, PSTR("/00system/time_"));
        itoa(cycleTimeSetting, timeChar, 10);
        strcat(timeFile, timeChar);
        strcat(timeFile, ".bmp");
        bmpDraw(timeFile, 0, 0);
      }

      // clock/gallery mode
      else if (menuMode == 3)
      {
        displayMode++;
        if (displayMode > displayModeMax) displayMode = 0;
        char modeChar[2];
        char modeFile[21];
        strcpy_P(modeFile, PSTR("/00system/mode_"));
        itoa(displayMode, modeChar, 10);
        strcat(modeFile, modeChar);
        strcat(modeFile, ".bmp");
        bmpDraw(modeFile, 0, 0);
      }

      // breakout time
      else if (menuMode == 4)
      {
        menuActive = false;
        saveSettingsToEEPROM();

        // return to brightness menu next time
        menuMode = 0;

        buttonTime = millis();
        breakout = true;
        gameInitialized = false;
        buttonEnabled = false;

        char tmp[23];
        strcpy_P(tmp, PSTR("/00system/breakout.bmp"));
        bmpDraw(tmp, 0, 0);

        paddleIndex = 230,
        ballX = 112,
        ballY = 208,
        ballIndex = 216;
        holdTime = 0;
        fileIndex = 0;
        leds[ballIndex] = CRGB(175, 255, 15);
        leds[paddleIndex] = CRGB(200, 200, 200);
        leds[paddleIndex + 1] = CRGB(200, 200, 200);
        leds[paddleIndex + 2] = CRGB(200, 200, 200);
        fastledshow();
        return;
      }
    }
  }

  // increment clock face
  if ((digitalRead(buttonNextPin) == LOW || irCommand == 'N') && buttonPressed == false && buttonEnabled == true && clockShown)
  {
    Serial.println(pongclock);
    // toggle pongclock
    pongclock = !pongclock;

    // turn on pong clock
    if (pongclock)
    {
      pongclock = true;
      pong_reset = true;
    }

    // increment clock face
    else
    {
      pongclock = false;
      uint8_t output = EEPROM.read(5);
      output++;
      if (output > 5) output = 1;
      setClockFace(output);
    }
    initClock();
    EEPROM.put(7, pongclock);
  }

  // menu button
  else if ((digitalRead(buttonMenuPin) == LOW || irCommand == 'M') && buttonPressed == false && buttonEnabled == true)
  {
    fadeStartTime = 0; // cancel any pending fades
    buttonPressed = true;
    menuEndTime = millis() + 3000;
    menuPowerCounter = millis() + 1500;

    if (menuActive == false)
    {
      menuActive = true;
      menuEnterTime = millis();
      offsetBufferX = offsetX;
      offsetBufferY = offsetY;
      offsetX = 0;
      offsetY = 0;
      closeMyFile();
      if (clockShown || clockAnimationActive)
      {
        clockAnimationActive = false;
        clockShown = false;
        abortImage = true;
        nextFolder[0] = '\0';
      }
    }
    else
    {
      menuMode++;
      if (menuMode > 4) menuMode = 0;
    }
    if (menuMode == 0)
    {
      char brightChar[2];
      char brightFile[23];
      strcpy_P(brightFile, PSTR("/00system/bright_"));
      itoa(brightness, brightChar, 10);
      strcat(brightFile, brightChar);
      strcat(brightFile, ".bmp");
      bmpDraw(brightFile, 0, 0);
    }
    else if (menuMode == 1)
    {
      char playChar[2];
      char playFile[21];
      strcpy_P(playFile, PSTR("/00system/play_"));
      itoa(playMode, playChar, 10);
      strcat(playFile, playChar);
      strcat(playFile, ".bmp");
      bmpDraw(playFile, 0, 0);
    }
    else if (menuMode == 2)
    {
      char timeChar[2];
      char timeFile[21];
      strcpy_P(timeFile, PSTR("/00system/time_"));
      itoa(cycleTimeSetting, timeChar, 10);
      strcat(timeFile, timeChar);
      strcat(timeFile, ".bmp");
      bmpDraw(timeFile, 0, 0);
    }
    else if (menuMode == 3)
    {
      if (displayMode > displayModeMax) displayMode = 0; // make sure the user has proper menu graphics installed
      char modeChar[2];
      char modeFile[21];
      strcpy_P(modeFile, PSTR("/00system/mode_"));
      itoa(displayMode, modeChar, 10);
      strcat(modeFile, modeChar);
      strcat(modeFile, ".bmp");
      bmpDraw(modeFile, 0, 0);
    }
    else if (menuMode == 4)
    {
      char gameFile[21];
      strcpy_P(gameFile, PSTR("/00system/game.bmp"));
      bmpDraw(gameFile, 0, 0);
    }
  }

  // time to exit menu mode?
  if (menuActive == true)
  {
    if (millis() > menuEndTime)
    {
      if (displayMode == 1)
      {
        initClock();
        return;
      }

      menuActive = false;

      saveSettingsToEEPROM();

      // return to brightness menu next time
      menuMode = 0;

      offsetX = offsetBufferX;
      offsetY = offsetBufferY;
      if (playMode == 2)
      {
        offsetX = imageWidth / -2 + 8;
        offsetY = imageHeight / 2 - 8;
      }
      swapTime = swapTime + (millis() - menuEnterTime);
      baseTime = baseTime + (millis() - menuEnterTime);
      if (abortImage == false && displayMode == 0)
      {
        drawFrame();
      }
      else clearStripBuffer();
      if (displayMode == 3)
      {
        drawChart(chartValues);
        refreshChart();
      }
      else if (scoreBoard) cloudScore(scoreString);
    }
  }

  // currently playing images?
  if (menuActive == false && breakout == false)
  {
    autoDisplayModeManager();

    if (clockShown == false || clockAnimationActive == true)
    {
      fadeControl();
      // advance counter
      if (currentSecond != lastSecond)
      {
        lastSecond = currentSecond;
        secondCounter++;
        // revert to clock display if animation played for 5 seconds
        if (clockAnimationActive == true && secondCounter >= clockAnimationLength)
        {
          initClock();
        }
      }

      // did image load fail?
      if (abortImage == true && clockShown == false && logoPlayed == true)
      {
        Serial.println("Abort Image Requested. Aborting.");
        abortImage = false;
        // stop folder chain if necessary
        if (chainIndex > -1) chainIndex = -1;
        nextImage();
        drawFrame();
      }

      // progress if cycleTime is up
      // check for infinite mode
      else if (cycleTimeSetting != 8  && clockShown == false && clockAnimationActive == false)
      {
        if (secondCounter >= cycleTime)
        {
          if (finishBeforeProgressing == true)
          {
            if (timerLapsed == false) timerLapsed = true;
          }
          else if (displayMode == 2 && alertPhase == 0)
          {
            nextEffect();
          }
          else if (displayMode == 3 && alertPhase == 0)
          {
            secondCounter = 0;
            chartRefresh = 60;
            refreshChart(); // refresh whole chart
          }
          else
          {
            // stop folder chain if necessary
            if (chainIndex > -1) chainIndex = -1;
            Serial.println("CycleTime Expired.");
            nextImage();
            drawFrame();
          }
        }
      }

      if (alertPhase == 0 && logoPlayed == true && displayMode >=2)
      {
        // effects mode
        if (displayMode == 2)
        {
          runEffects();
        }
        // display chart
        else if (displayMode == 3)
        {
          if (secondCounter > chartRefresh)
          {
            // force hourly chart update
            if (secondCounter > 3600 || chartValues[0] == 0.0f)
            {
              secondCounter = 0;
              chartRefresh = 60;
              refreshChart();
            }
            else
            {
              chartRefresh = secondCounter + 60;
              // Refresh right-most column every minute
              if (cycleTimeSetting >= 4)
              {
                refreshChartLatest();
                drawChart(chartValues);
              }
            }
          }
        }
      }

      // animate if not a single-frame & animations are on; always animate the logo
      else if (holdTime != -1 && playMode != 2 && playMode != 3 || logoPlayed == false)
      {
        if (millis() >= swapTime && clockShown == false)
        {
          statusLedFlicker();
          swapTime = millis() + holdTime;
          if (!singleGraphic) fileIndex++;
          drawFrame();
        }
      }
    }

    // show clock
    else if (clockShown == true && clockAnimationActive == false)
    {
      showClock();
    }
  }
}

void autoDisplayModeManager()
{
  if (displayModeCycle)
  {
    long cTime = millis();
    if (cTime > displayModeCycleTime)
    {
      Serial.println("AUTO Display Mode Change!");
      displayModeCycleTime = cTime + displayModeCycleLength;
      displayMode++;
      if (displayMode > displayModeMax) displayMode = 0;

      closeMyFile();
      if (clockShown || clockAnimationActive)
      {
        clockAnimationActive = false;
        clockShown = false;
        abortImage = true;
        nextFolder[0] = '\0';
      }

      if (displayMode == 1)
      {
        initClock();
        return;
      }

      else if (playMode == 2)
      {
        offsetX = imageWidth / -2 + 8;
        offsetY = imageHeight / 2 - 8;
      }

      if (abortImage == false && displayMode == 0)
      {
        drawFrame();
      }

      else clearStripBuffer();

      if (displayMode == 3)
      {
        drawChart(chartValues);
        refreshChart();
      }
    }
  }
}

void nextImage()
{
  Serial.println(F("--->"));
  Serial.println(F("Next Folder..."));
  closeMyFile();
  if (alertPhase == 1)
  {
    Serial.println("Processing Alert...");
    if (nextFolder[0] == '\0')
    {
      alertPhase = 0;
      Serial.println("Alert finished. Resuming.");
      resumeGalleryState();
      if (scoreBoard) cloudScore(scoreString);
      return;
    }
  }
  boolean foundNewFolder = false;
  char folder[32];

  // are we chaining folders?
  if (chainIndex > -1 && alertPhase == 0)
  {
    char chainChar[6];
    char chainDir[23];
    strcpy_P(chainDir, PSTR("/"));
    strcat(chainDir, chainRootFolder);
    strcat(chainDir, "/");
    itoa(chainIndex, chainChar, 10);
    strcat(chainDir, chainChar);
    if (sd.exists(chainDir))
    {
      Serial.print(F("Chaining: "));
      Serial.println(chainDir);
      foundNewFolder = true;
      sd.chdir(chainDir);
      chainIndex++;
    }
    else
    {
      // chaining concluded
      Serial.println(F("Chaining concluded."));
      // check for animation loop
      if (folderLoop == true && timerLapsed == false)
      {
        chainIndex = 0;
        foundNewFolder = true;
        strcpy_P(chainDir, PSTR("/"));
        strcat(chainDir, chainRootFolder);
        strcat(chainDir, "/");
        itoa(chainIndex, chainChar, 10);
        strcat(chainDir, chainChar);
        Serial.print(F("Looping animation. Restarting: "));
        Serial.println(chainDir);
        sd.chdir(chainDir);
        chainIndex++;
      }
      else
      {
        chainIndex = -1;
        chainRootFolder[0] = '\0';
      }
    }
  }

  // not chaining folders
  if (chainIndex == -1)
  {
    // reset secondCounter if not playing clock animations
    if (!clockAnimationActive) secondCounter = 0;
    baseTime = millis();
    fadeStartTime = millis();
    holdTime = 0;
    sd.chdir("/");
    fileIndex = 0;
    offsetX = 0;
    offsetY = 0;
    singleGraphic = false;
    finishBeforeProgressing = false;
    timerLapsed = false;
    nestedRootIni = false;
    if (!logoPlayed) logoPlayed = true;
  }

  // has the next animation been dictated by the previous .INI file?
  if (nextFolder[0] != '\0') // removed && alertPhase == -1
  {
    Serial.print(F("Forcing next: "));
    Serial.println(nextFolder);
    strcpy(folder, nextFolder);
    if (sd.exists(nextFolder))
    {
      sd.chdir(nextFolder);
    }
    else
    {
      nextFolder[0] = '\0';
      Serial.println(F("Not exists!"));
      if (alertPhase == 1)
      {
        alertPhase = 0;
        Serial.println("Alert aborted. Resuming.");
        resumeGalleryState();
        return;
      }
    }
  }

  // next folder not assigned by .INI
  if (nextFolder[0] == '\0' && foundNewFolder == false) // removed && alertPhase == -1
  {
    Serial.println("Finding next folder.");
    // Getting next folder
    // shuffle playback using random number
    if (playMode != 0) // check we're not in a sequential play mode
    {
      int targetFolder = realRandom(0, numFolders);

      // don't repeat the same image, please.
      if (targetFolder <= 0 or targetFolder == numFolders or targetFolder == numFolders - 1)
      {
        // Repeat image detected! Incrementing targetFolder.
        targetFolder = targetFolder + 2;
      }

      Serial.print(F("Randomly advancing "));
      Serial.print(targetFolder);
      Serial.println(F(" folder(s)."));
      int i = 1;
      while (i < targetFolder)
      {
        foundNewFolder = false;
        while (foundNewFolder == false)
        {
          myFile.open(sd.vwd(), folderIndex, O_READ);
          if (myFile.isDir()) {
            foundNewFolder = true;
            i++;
          }
          closeMyFile();
          folderIndex++;
        }
      }
    }

    foundNewFolder = false;

    while (foundNewFolder == false)
    {
      myFile.open(sd.vwd(), folderIndex, O_READ);
      myFile.getName(folder, 13);

      // ignore system folders that start with "00"
      if (myFile.isDir() && folder[0] != 48 && folder[1] != 48) {
        foundNewFolder = true;
        Serial.print(F("Folder Index: "));
        Serial.println(folderIndex);
        Serial.print(F("Opening Folder: "));
        Serial.println(folder);

        sd.chdir(folder);
        closeMyFile();
      }
      else closeMyFile();
      folderIndex++;
    }
  }

  // is this the start of a folder chain?
  char chainDir[2];
  strcpy_P(chainDir, PSTR("0"));
  if (sd.exists(chainDir))
  {
    Serial.print(F("Chaining detected: "));
    Serial.println(folder);
    // use config.ini in nest root if available
    if (sd.exists("config.ini"))
    {
      Serial.print(F("Opening Root File: "));
      Serial.print(folder);
      Serial.println(F("/config.ini"));
      readIniFile();
    }
    memcpy(chainRootFolder, folder, 8);
    sd.chdir(chainDir);
    chainIndex = 1;
  }

  char firstImage[6];
  strcpy_P(firstImage, PSTR("0.bmp"));
  Serial.print("Checking for 0.bmp...");
  if (sd.exists(firstImage))
  {
    Serial.println("Found!");
    // use nested config.ini files if root config missing
    // if alert in progress, ignore buffered nested folder
    // also, don't use nested folders for alerts :)
    if (chainIndex > -1 && alertPhase == 0)
    {
      char rootFolderConfig[20];
      strcpy(rootFolderConfig, "/");
      strcat(rootFolderConfig, chainRootFolder);
      strcat(rootFolderConfig, "/config.ini");
      if (!sd.exists(rootFolderConfig))
      {
        Serial.print(F("Opening File: "));
        Serial.print(folder);
        Serial.print("/");
        Serial.print(chainDir);
        Serial.println(F("/config.ini"));
        readIniFile();
      }
    }
    else
    {
      Serial.print(F("Opening File: "));
      Serial.print(folder);
      Serial.println(F("/config.ini"));
      readIniFile();
    }

    char tmp[6];
    strcpy_P(tmp, PSTR("0.bmp"));
    refreshImageDimensions(tmp);

    Serial.print(F("Hold (in ms): "));
    Serial.println(holdTime);
    swapTime = millis() + holdTime;

    // setup image for x/y translation as needed if animations aren't paused
    if (playMode != 2)
    {
      if (offsetSpeedX > 0)
      {
        if (panoff == true) offsetX = (imageWidth * -1);
        else offsetX = (imageWidth * -1 + 16);
      }
      else if (offsetSpeedX < 0)
      {
        if (panoff == true) offsetX = 16;
        else offsetX = 0;
      }
      if (offsetSpeedY > 0)
      {
        if (panoff == true) offsetY = -16;
        else offsetY = 0;
      }
      else if (offsetSpeedY < 0)
      {
        if (panoff == true) offsetY = imageHeight;
        else offsetY = imageHeight - 16;
      }
    }
    // center image if animations are paused
    else
    {
      offsetX = imageWidth / -2 + 8;
      offsetY = imageHeight / 2 - 8;
    }

    // test for single frame

    char tmp_0[6];
    char tmp_1[6];
    strcpy_P(tmp_0, PSTR("0.bmp"));
    strcpy_P(tmp_1, PSTR("1.bmp"));
    if (sd.exists(tmp_0) && (!sd.exists(tmp_1)))
    {
      singleGraphic = true;

      // disabling the below -- let's not discriminate against single images
      // they should be held to their config.ini just like anything else.
      // NOTE: a missing config.ini defaults to holdTime = 200.
      /*// check for pan settings
      if (offsetSpeedX == 0 && offsetSpeedY == 0)
      {
        // single frame still
        holdTime = -1;
      }*/
    }
  }

  // empty folder
  else
  {
    Serial.println(F("NOT found!"));
    nextImage();
  }
}

void drawFrame()
{
  if (panoff == true)
  {
    if (offsetX > 16 || offsetX < (imageWidth * -1) || offsetY > imageHeight || offsetY < -16)
    {
      if (moveLoop == false || finishBeforeProgressing == true)
      {
        Serial.println("Image panned off.");
        fileIndex = 0;
        if (alertPhase > 0)
        {
          nextImage();
          return;
        }
        else if (clockAnimationActive && secondCounter > 0)
        {
          Serial.println(F("Clock animation finished. Returning to clock.")); // clock animation finished
          initClock();
          return;
        }
        else nextImage();
      }
      else
      {
        if (offsetSpeedX > 0 && offsetX >= 16)
        {
          offsetX = (imageWidth * -1);
        }
        else if (offsetSpeedX < 0 && offsetX <= imageWidth * -1)
        {
          offsetX = 16;
        }
        if (offsetSpeedY > 0 && offsetY >= imageHeight)
        {
          offsetY = -16;
        }
        else if (offsetSpeedY < 0 && offsetY <= -16)
        {
          offsetY = imageHeight;
        }
      }
    }
  }
  else
  {
    if (offsetX > 0 || offsetX < (imageWidth * -1 + 16) || offsetY > imageHeight - 16 || offsetY < 0)
    {
      if (moveLoop == false || finishBeforeProgressing == true)
      {
        Serial.println("Image finished panning (panoff == false).");
        fileIndex = 0;
        if (alertPhase > 0)
        {
          nextImage();
          return;
        }
        else if (clockAnimationActive && secondCounter > 0) // clock animation finished
        {
          Serial.println(F("Clock animation finished. Returning to clock."));
          initClock();
          return;
        }
        else nextImage();
      }
      else
      {
        if (offsetSpeedX > 0 && offsetX >= 0)
        {
          offsetX = (imageWidth * -1 + 16);
        }
        else if (offsetSpeedX < 0 && offsetX <= imageWidth - 16)
        {
          offsetX = 0;
        }
        if (offsetSpeedY > 0 && offsetY >= imageHeight - 16)
        {
          offsetY = 0;
        }
        else if (offsetSpeedY < 0 && offsetY <= 0)
        {
          offsetY = imageHeight - 16;
        }
      }
    }
  }

  if (singleGraphic == false)
  {
    char bmpFile[13]; // 8-digit number + .bmp + null byte
    itoa(fileIndex, bmpFile, 10);
    strcat(bmpFile, ".bmp");
    if (!sd.exists(bmpFile))
    {
      /*Serial.println("~~~~");
      char volumeWorkingDirectory[32];
      sd.vwd()->getName(volumeWorkingDirectory, 31);
      Serial.print("dir == ");
      Serial.println(volumeWorkingDirectory);
      Serial.print("bmpFile == ");
      Serial.println(bmpFile);
      Serial.print("singleGraphic == ");
      Serial.println(singleGraphic);
      Serial.print("folderLoop == ");
      Serial.println(folderLoop);
      Serial.print("timerLapsed == ");
      Serial.println(timerLapsed);*/
      fileIndex = 0;
      itoa(fileIndex, bmpFile, 10);
      strcat(bmpFile, ".bmp");
      if (finishBeforeProgressing && (offsetSpeedX != 0 || offsetSpeedY != 0)); // translating image - continue animating until moved off screen
      else if (folderLoop == false || timerLapsed == true)
      {
        if (displayMode == 0 || displayMode == 2 || displayMode == 3)
        {
          Serial.println("No more images to display and folderLoop == false.");
          if (displayMode == 3) drawChart(chartValues);
          nextImage();
          return;
        }
        else if (displayMode == 1)
        {
          Serial.println(F("Animation finished. Initializing clock."));
          initClock();
          return;
        }
      }
    }
    bmpDraw(bmpFile, 0, 0);
  }
  else bmpDraw("0.bmp", 0, 0);

  if (debugMode)
  {
    // print draw time in milliseconds
    drawTime = millis() - lastTime;
    lastTime = millis();
    Serial.print(F("ttd: "));
    Serial.println(drawTime);
  }

  if (offsetSpeedX != 0) offsetX += offsetSpeedX;
  if (offsetSpeedY != 0) offsetY += offsetSpeedY;
}

void refreshImageDimensions(char *filename) {

  const uint8_t gridWidth = 16;
  const uint8_t gridHeight = 16;

  if ((0 >= gridWidth) || (0 >= gridHeight)) {
    Serial.print(F("Abort."));
    return;
  }

  // storing dimentions for image

  // Open requested file on SD card
  if (!myFile.open(filename, O_READ)) {
    Serial.println(F("File open failed"));
    sdErrorMessage(255, 255, 0);
    return;
  }

  // Parse BMP header
  if (read16(myFile) == 0x4D42) { // BMP signature
    (void)read32(myFile); // Read & ignore file size
    (void)read32(myFile); // Read & ignore creator bytes
    (void)read32(myFile); // skip data
    // Read DIB header
    (void)read32(myFile); // Read & ignore Header size
    imageWidth  = read32(myFile);
    imageHeight = read32(myFile);
    Serial.print(F("Image resolution: "));
    Serial.print(imageWidth);
    Serial.print(F("x"));
    Serial.println(imageHeight);
  }
  closeMyFile();
}

// This function opens a Windows Bitmap (BMP) file and
// displays it at the given coordinates.  It's sped up
// by reading many pixels worth of data at a time
// (rather than pixel by pixel).  Increasing the buffer
// size takes more of the Arduino's precious RAM but
// makes loading a little faster.  20 pixels seems a
// good balance.

void bmpDraw(char *filename, int x, int y) {

  int  bmpWidth, bmpHeight;   // W+H in pixels
  uint8_t  bmpDepth;              // Bit depth (currently must be 24)
  uint32_t bmpImageoffset;        // Start of image data in file
  uint32_t  rowSize;               // Not always = bmpWidth; may have padding
  uint8_t  sdbuffer[3 * BUFFPIXEL]; // pixel buffer (R+G+B per pixel)
  uint8_t  buffidx = sizeof(sdbuffer); // Current position in sdbuffer
  boolean  goodBmp = false;       // Set to true on valid header parse
  boolean  flip    = true;        // BMP is stored bottom-to-top
  int  w, h, row, col;
  uint8_t  r, g, b;
  uint32_t pos = 0;
  const uint8_t gridWidth = 16;
  const uint8_t gridHeight = 16;

  if (x >= gridWidth || y >= gridHeight || x < -3 || y < 0) {
    if (debugMode) Serial.print(F("Abort."));
    return;
  }

  if (!myFile.isOpen())
  {
    Serial.print(F("-=Loading image '"));
    Serial.print(filename);
    Serial.println('\'');
    // Open requested file on SD card
    if (!myFile.open(filename, O_READ)) {
      Serial.println(F("File open failed"));
      sdErrorMessage(0, 0, 255);
      return;
    }
  }
  else myFile.rewind();

  // Parse BMP header
  if (read16(myFile) == 0x4D42) { // BMP signature
    if (debugMode)
    {
      Serial.print(F("File size: ")); Serial.println(read32(myFile));
    }
    else (void)read32(myFile);
    (void)read32(myFile); // Read & ignore creator bytes
    bmpImageoffset = read32(myFile); // Start of image data
    //    Serial.print(F("Image Offset: ")); Serial.println(bmpImageoffset, DEC);
    // Read DIB header
    (void)read32(myFile); // Read & ignore Header size
    bmpWidth  = read32(myFile);
    bmpHeight = read32(myFile);
    if (read16(myFile) == 1) { // # planes -- must be '1'
      bmpDepth = read16(myFile); // bits per pixel
      //      Serial.print(F("Bit Depth: ")); Serial.println(bmpDepth);
      if ((bmpDepth == 24) && (read32(myFile) == 0)) { // 0 = uncompressed

        goodBmp = true; // Supported BMP format -- proceed!
        if (debugMode)
        {
          Serial.print(F("Image size: "));
          Serial.print(bmpWidth);
          Serial.print('x');
          Serial.println(bmpHeight);
        }

        // image smaller than 16x16?
        if ((bmpWidth < 16 && bmpWidth > -16) || (bmpHeight < 16 && bmpHeight > -16))
        {
          clearStripBuffer();
        }

        // BMP rows are padded (if needed) to 4-byte boundary
        rowSize = (bmpWidth * 3 + 3) & ~3;
        //        Serial.print(F("Row size: "));
        //        Serial.println(rowSize);

        // If bmpHeight is negative, image is in top-down order.
        // This is not canon but has been observed in the wild.
        if (bmpHeight < 0) {
          bmpHeight = -bmpHeight;
          flip      = false;
        }

        // initialize our pixel index
        byte index = 0; // a byte is perfect for a 16x16 grid

        // Crop area to be loaded
        w = bmpWidth;
        h = bmpHeight;
        if ((x + w - 1) >= gridWidth)  w = gridWidth - x;
        if ((y + h - 1) >= gridHeight) h = gridHeight - y;

        for (row = 0; row < h; row++) { // For each scanline...

          // Seek to start of scan line.  It might seem labor-
          // intensive to be doing this on every line, but this
          // method covers a lot of gritty details like cropping
          // and scanline padding.  Also, the seek only takes
          // place if the file position actually needs to change
          // (avoids a lot of cluster math in SD library).

          if (flip) // Bitmap is stored bottom-to-top order (normal BMP)
            pos = (bmpImageoffset + (offsetX * -3) + (bmpHeight - 1 - (row + offsetY)) * rowSize);
          else     // Bitmap is stored top-to-bottom
            pos = bmpImageoffset + row * rowSize;
          if (myFile.curPosition() != pos) { // Need seek?
            myFile.seekSet(pos);
            buffidx = sizeof(sdbuffer); // Force buffer reload
          }

          for (col = 0; col < w; col++) { // For each pixel...
            // Time to read more pixel data?
            if (buffidx >= sizeof(sdbuffer)) { // Indeed
              myFile.read(sdbuffer, sizeof(sdbuffer));
              buffidx = 0; // Set index to beginning
            }

            // push to LED buffer
            b = sdbuffer[buffidx++];
            g = sdbuffer[buffidx++];
            r = sdbuffer[buffidx++];

            // apply contrast
            r = dim8_jer(r);
            g = dim8_jer(g);
            b = dim8_jer(b);

            // offsetY is beyond bmpHeight
            if (row >= bmpHeight - offsetY)
            {
              // black pixel
              leds[getIndex(col, row)] = CRGB(0, 0, 0);
            }
            // offsetY is negative
            else if (row < offsetY * -1)
            {
              // black pixel
              leds[getIndex(col, row)] = CRGB(0, 0, 0);
            }
            // offserX is beyond bmpWidth
            else if (col >= bmpWidth + offsetX)
            {
              // black pixel
              leds[getIndex(col, row)] = CRGB(0, 0, 0);
            }
            // offsetX is positive
            else if (col < offsetX)
            {
              // black pixel
              leds[getIndex(col, row)] = CRGB(0, 0, 0);
            }
            // print area is out of bounds
            else if (col + x < 0)
            {
              leds[getIndex(col, row)] = CRGB(0, 0, 0);
            }
            // all good
            else
            {
              leds[getIndex(col + x, row)] = CRGB(r, g, b);
            }
            // paint pixel color
          } // end pixel
        } // end scanline
      } // end goodBmp
    }
  }
  if (!clockShown || breakout == true)
  {
    fastledshow();
  }
  if (singleGraphic == false || menuActive == true || breakout == true)
  {
    closeMyFile();
  }
  if (!goodBmp) Serial.println(F("Format unrecognized"));
}

uint8_t dim8_jer( uint8_t x )
{
  return ((uint16_t)x * (uint16_t)(x) ) >> 8;
}

void closeMyFile()
{
  if (myFile.isOpen())
  {
    if (debugMode) Serial.println(F("Closing Image..."));
    myFile.close();
  }
}

byte getIndex(byte x, byte y)
{
  byte index;
  if (y == 0)
  {
    index = x;
  }
  else if (y % 2 == 0)
  {
    index = y * 16 + x;
  }
  else
  {
    index = (y * 16 + 15) - x;
  }
  return index;
}

void clearStripBuffer()
{
  for (int i = 0; i < 256; i++)
  {
    leds[i] = CRGB(0, 0, 0);
  }
}

void buttonDebounce()
{
  // button debounce -- no false positives
  if (((digitalRead(buttonMenuPin) == HIGH) && digitalRead(buttonNextPin) == HIGH) && buttonPressed == true)
  {
    buttonPressed = false;
    buttonEnabled = false;
    buttonTime = millis();
  }
  if ((buttonEnabled == false) && buttonPressed == false)
  {
    if (millis() > buttonTime + 50) buttonEnabled = true;
  }
}

// automatic brightness control
void runABC()
{
  if (currentSecond == 0)
  {
    if (abc && clockSet && currentMinute != abcMinute)
    {
      abcMinute = currentMinute;
      int dayInMinutes = minuteCounter(currentHour, currentMinute);
      int triggerTimes[10] = {abc0.m, abc1.m, abc2.m, abc3.m, abc4.m, abc5.m, abc6.m, abc7.m, abc8.m, abc9.m};
      byte brightnessSettings[10] = {abc0.b, abc1.b, abc2.b, abc3.b, abc4.b, abc5.b, abc6.b, abc7.b, abc8.b, abc9.b};
      for (byte x = 0; x < 9; x++)
      {
        if (dayInMinutes == triggerTimes[x])
        {
          brightness = brightnessSettings[x];
          stripSetBrightness();
          if (brightness == 0)
          {
            clearStripBuffer();
            fastledshow();
          }
          else if (displayMode == 1)
          {
            initClock();
          }
          // refresh the screen
          else if (displayMode == 0)
          {
            drawFrame();
          }
        }
      }
    }
  }
}

// Clock

void debugClockDisplay()
{
  // digital clock display of the time
  Serial.print(currentHour);
  printDigits(currentMinute);
  printDigits(currentSecond);
  Serial.println();
}

void printDigits(int digits)
{
  // utility function for clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void initClock()
{
  saveSettingsToEEPROM();

  alertPhase = 0;
  secondCounter = 0;
  menuActive = false;
  clockShown = true;
  clockAnimationActive = false;
  buttonTime = millis();
  menuMode = 0;
  lastSecond = 255;
  if (pongclock) pong_reset = true;

  closeMyFile();
  holdTime = 0;
  sd.chdir("/");
  fileIndex = 0;
  singleGraphic = true;
  if (!logoPlayed) logoPlayed = true;

  if (!clockIniRead) readClockIni();

  if (!enableSecondHand && framePowered)
  {
    // 24 hour conversion
    if (hour12 && currentHour > 12) currentHour -= 12;
    if (hour12 && currentHour == 0) currentHour = 12;
    drawDigits();
    fastledshow();
  }
}

void setClock()
{
  clockSet = false;
  clockAdjustState = false;
  currentSecond = 0;
  setClockHour();
  setClockMinute();
  adjustClock();
  lastSecond = 255;
  if (EEPROM.read(4) != 1) EEPROM.write(4, 1);
  // don't adjust second hand at midnight if set manually AT midnight
  if (currentHour == 0 && currentMinute == 0) clockAdjustState = true;
}

void setClockHour()
{
  // set hour
  long lastButtonCheck = 0;
  long lastDigitFlash = 0;
  while (clockDigitSet == false)
  {
    buttonDebounce();
    irReceiver();

    // menu button
    if ((digitalRead(buttonMenuPin) == LOW || irCommand == 'M' || irMenuRepeat == true) && buttonPressed == false && buttonEnabled == true && millis() - lastButtonCheck > 100)
    {
      clockSetBlink = true;
      currentHour++;
      if (currentHour > 23) currentHour = 0;
      lastButtonCheck = millis();
      lastDigitFlash = millis();
      drawDigits();
      fastledshow();
      debugClockDisplay();
    }

    // flash digit
    if (millis() - lastDigitFlash > 250)
    {
      lastDigitFlash = millis();
      drawDigits();
      fastledshow();
      debugClockDisplay();
    }

    // next button
    if ((digitalRead(buttonNextPin) == LOW || irCommand == 'N') && buttonPressed == false && buttonEnabled == true)
    {
      buttonPressed = true;
      clockDigitSet = true;
    }
  }
}

void setClockMinute()
{
  // set minutes
  long lastButtonCheck = 0;
  long lastDigitFlash = 0;
  while (clockDigitSet == true)
  {
    buttonDebounce();
    irReceiver();
    // menu button
    if ((digitalRead(buttonMenuPin) == LOW  || irCommand == 'M' || irMenuRepeat == true) && buttonPressed == false && buttonEnabled == true && millis() - lastButtonCheck > 100)
    {
      clockSetBlink = true;
      currentMinute++;
      if (currentMinute > 59) currentMinute = 0;
      lastButtonCheck = millis();
      lastDigitFlash = millis();
      drawDigits();
      fastledshow();
      debugClockDisplay();
    }

    // flash digit
    if (millis() - lastDigitFlash > 250)
    {
      lastDigitFlash = millis();
      drawDigits();
      fastledshow();
      debugClockDisplay();
    }

    // next button
    if ((digitalRead(buttonNextPin) == LOW || irCommand == 'N') && buttonPressed == false && buttonEnabled == true)
    {
      buttonPressed = true;
      clockDigitSet = false;
      clockSetBlink = true;
    }
  }
}

void adjustClock()
{
  rtc.adjust(DateTime(2016, 1, 1, currentHour, currentMinute, 0)); // year, month, day, hour, min, sec
  syncClock(true);
}

// sync clock once a day
void syncClock(boolean force)
{
  if (Particle.connected())
  {
    if (millis() - lastSync > RTC_SYNC_TIME || force)
    {
      // Request time synchronization from the Particle Cloud
      Serial.print("Syncing Internet Time...");
      Particle.syncTime();
      Serial.println("Done!");
      lastSync = millis();
    }
  }
  else if (enableWifi == false) // don't use DS3231 if Wi-Fi dropped. Causes issues. Instead wait for Wi-Fi to come back online.
  {
    if (millis() - lastSync > RTC_SYNC_TIME || force)
    {
      Serial.print("Syncing DS3231 Time...");
      now = rtc.now();
      Time.setTime(now.unixtime());
      Serial.print("Done! (");
      Serial.print(now.unixtime());
      Serial.println(")");
      lastSync = millis();
    }
  }
}

void getCurrentTime()
{
  currentSecond = Time.second();
  currentMinute = Time.minute();
  currentHour = Time.hour();
}

void showClock()
{
  if (currentSecond != lastSecond)
  {
    secondCounter++;
    lastSecond = currentSecond;
    statusLedFlicker();
    debugClockDisplay();

    // 24 hour conversion
    if (hour12 && currentHour > 12) currentHour -= 12;
    if (hour12 && currentHour == 0) currentHour = 12;

    // draw time
    if (enableSecondHand)
    {
      if (!pongclock)
      {
        // offset second hand if required
        currentSecond = currentSecond + secondOffset;
        if (currentSecond >= 60) currentSecond -= 60;
        storeSecondHandColor();
        drawDigits();
        secondHand();
        fastledshow();
      }
    }

    if (currentSecond == 0)
    {
      // register pong score change
      if (pongclock)
      {
        if (currentMinute == 0) pong_scored_hour = true;
        else pong_scored_minute = true;
      }
      // second hand disabled, so only draw time on new minute
      else if (!enableSecondHand)
      {
        drawDigits();
        fastledshow();
      }
    }

    // show an animation
    if (cycleTime != -1 && clockAnimationLength > 0 && (secondsIntoHour() % cycleTime) == 0 && clockSet == true)
    {
      // exit chaining if necessary
      if (chainIndex > -1)
      {
        chainIndex = -1;
        chainRootFolder[0] = '\0';
        sd.chdir("/");
      }
      secondCounter = 0;
      currentSecond = Time.second();
      clockAnimationActive = true;
      clockShown = false;
      closeMyFile();
      abortImage = true;
//      nextFolder[0] = '\0';
    }
    // this boolean is set here to avoid showing an animation immediately after clock being set
    else if (!clockSet) clockSet = true;
  }

  // pong clock
  if (pongclock)
  {
    // game in progress
    if (!pong_celebrate)
    {
      if (millis() > pong_showtime)
      {
        pong_showtime = millis() + pong_speed;

        // move ball
        // left side
        if ((ballX + sin(degToRad(ballAngle)) * 16) + .5 > 256-16)
        {
          if (!pong_scored_minute) 
          {
            if (getScreenIndex(ballX, ballY) < getScreenIndex(255, pong_paddle_left_y)) ballAngle = realRandom(225-25, 225+25);
            else if (getScreenIndex(ballX, ballY) == getScreenIndex(255, pong_paddle_left_y) + 1) swapXdirection();
            else if (getScreenIndex(ballX, ballY) == getScreenIndex(255, pong_paddle_left_y) - 1) swapXdirection();
            else ballAngle = realRandom(315-25, 315+25);
            pong_ball_direction = 1;
            pong_paddle_right_start = pong_paddle_right_y;
            pong_paddle_right_target = constrain(pong_predict_y(ballX, ballY, ballAngle), 16, 255-16);
          }
          else
          {
            pong_celebrate = true;
            pong_celebration_end = millis() + 2000;
            // 24 hour conversion
            if (hour12 && currentHour > 12) currentHour -= 12;
            if (hour12 && currentHour == 0) currentHour = 12;
            drawDigits();
            fastledshow();
            memcpy( leds_buf, leds, NUM_LEDS * sizeof( CRGB) );
          }
        }

        // right side
        else if ((ballX + sin(degToRad(ballAngle)) * 16) + .5 < 16)
        {
          if (!pong_scored_hour)
          {
            if (getScreenIndex(ballX, ballY) < getScreenIndex(0, pong_paddle_right_y)) ballAngle = realRandom(135-25, 135+25);
            else if (getScreenIndex(ballX, ballY) == getScreenIndex(0, pong_paddle_right_y) + 1) swapXdirection();
            else if (getScreenIndex(ballX, ballY) == getScreenIndex(0, pong_paddle_right_y) - 1) swapXdirection();
            else ballAngle = realRandom(45-25, 45+25);
            pong_ball_direction = 0;
            pong_paddle_left_start = pong_paddle_left_y;
            pong_paddle_left_target = constrain(pong_predict_y(ballX, ballY, ballAngle), 16, 255-16);
          }
          else
          {
            pong_celebrate = true;
            pong_celebration_end = millis() + 2000;            
            // 24 hour conversion
            if (hour12 && currentHour > 12) currentHour -= 12;
            if (hour12 && currentHour == 0) currentHour = 12;
            drawDigits();
            fastledshow();
            memcpy( leds_buf, leds, NUM_LEDS * sizeof( CRGB) );
          }
        }

        if ((ballY + cos(degToRad(ballAngle)) * 16) + .5 < 0) swapYdirection();
        else if ((ballY + cos(degToRad(ballAngle)) * 16) + .5 > 256) swapYdirection();

        ballX = ballX + sin(degToRad(ballAngle)) * 16 + .5;
        ballY = ballY + cos(degToRad(ballAngle)) * 16 + .5;
      }
    }

    // celebrating
    else if (millis() > pong_celebration_end) pong_reset = true;

    // reset?
    if (pong_reset)
    {
      pong_reset = false;
      pong_celebrate = false;
      pong_scored_hour = false;
      pong_scored_minute = false;

      // store second hands
      Serial.println("Storing second hand colors...");
      offsetX = 0;
      offsetY = 176;
      bmpDraw(clockFace, 0, 0);
      uint8_t second_index = 0;
      for (uint8_t x=0; x<16; x++) secondHands[second_index++] = leds[getIndex(x, 0)];
      for (uint8_t y=0; y<16; y++) secondHands[second_index++] = leds[getIndex(15, y)];
      for (uint8_t x=0; x<16; x++) secondHands[second_index++] = leds[getIndex(15-x, 15)];
      for (uint8_t y=0; y<16; y++) secondHands[second_index++] = leds[getIndex(0, 15-y)];

      // 24 hour conversion
      if (hour12 && currentHour > 12) currentHour -= 12;
      if (hour12 && currentHour == 0) currentHour = 12;
      drawDigits();
      fastledshow();
      memcpy( leds_buf, leds, NUM_LEDS * sizeof( CRGB) );
      pong_paddle_left_y = 128;
      pong_paddle_left_start = 128;
      pong_paddle_left_target = 128;
      pong_paddle_right_y = 128;
      pong_paddle_right_start = 128;
      pong_paddle_right_target = 128;
      ballX = 255-16;
      ballY = 128;
      ballAngle = realRandom(225, 315);
      pong_ball_direction = 1;
      pong_paddle_right_target = constrain(pong_predict_y(ballX, ballY, ballAngle), 16, 255-16);
    }

    // manage paddles
    if (pong_ball_direction == 0) 
    {
      int offset = 0;
      if (pong_scored_minute)
      {
        if (pong_paddle_left_target < 3 * 16) offset = 2 * 16;
        else offset = -2 * 16;
      }
      pong_paddle_left_y = map(ballX, 16, 256-16, pong_paddle_left_start, pong_paddle_left_target + offset);
    }
    else
    {
      int offset = 0;
      if (pong_scored_hour)
      {
        if (pong_paddle_right_target < 3 * 16) offset = 2 * 16;
        else offset = -2 * 16;
      }
      pong_paddle_right_y = map(ballX, 256-16, 16, pong_paddle_right_start, pong_paddle_right_target + offset);
    }

    // copy the time back on screen
    memcpy( leds, leds_buf, NUM_LEDS * sizeof( CRGB) );

    // get hue
    CHSV pongHue_ball = rgb2hsv_approximate(secondHands[currentSecond]);
    //CHSV pongHue_ball = CHSV(160, 255, 255);
    CHSV pongHue_paddle = pongHue_ball;
    pongHue_paddle.hue += 128;

    if (pong_celebrate) pongHue_ball.hue += millis();

    // draw the dots
    leds[getScreenIndex(255, pong_paddle_left_y)] = pongHue_paddle;
    leds[getScreenIndex(255, pong_paddle_left_y-16)] = pongHue_paddle;
    leds[getScreenIndex(255, pong_paddle_left_y+16)] = pongHue_paddle;
    leds[getScreenIndex(0, pong_paddle_right_y)] = pongHue_paddle;
    leds[getScreenIndex(0, pong_paddle_right_y-16)] = pongHue_paddle;
    leds[getScreenIndex(0, pong_paddle_right_y+16)] = pongHue_paddle;
    ballIndex = getScreenIndex(ballX, ballY);
    leds[ballIndex] = pongHue_ball;
    fastledshow();
  }
}

int pong_predict_y(int x, int y, int angle)
{
  while (x >= 16 && x <= 256-16)
  {
    if ((y + cos(degToRad(angle)) * 16) + .5 < 0)
    {
      if (angle > 90 && angle < 270)
      {
        if (angle > 180) angle = 360 - (angle - 180);
        else angle = 90 - (angle - 90);
      }
      else
      {
        if (angle < 90) angle = 90 + (90 - angle);
        else angle = 180 + (360 - angle);
      }
    }
    else if ((y + cos(degToRad(angle)) * 16) + .5 > 256)
    {
      if (angle > 90 && angle < 270)
      {
        if (angle > 180) angle = 360 - (angle - 180);
        else angle = 90 - (angle - 90);
      }
      else
      {
        if (angle < 90) angle = 90 + (90 - angle);
        else angle = 180 + (360 - angle);
      }
    }
    x = x + sin(degToRad(angle)) * 16 + .5;
    y = y + cos(degToRad(angle)) * 16 + .5;            
  }
  return y;
}

int secondsIntoHour()
{
  return (currentMinute * 60) + currentSecond;
}

int minutesIntoDay()
{
  return (currentHour * 60) + currentMinute;
}

void drawDigits()
{
  clockDigit_1();
  clockDigit_2();
  clockDigit_3();
  clockDigit_4();
  if (!clockSet) clockSetBlink = !clockSetBlink;
  closeMyFile();
}

void storeSecondHandColor()
{
  getSecondHandIndex();
  offsetX = 0;
  offsetY = 176;
  bmpDraw(clockFace, 0, 0);
  secondHandColor = leds[getIndex(secondHandX, secondHandY)];
}

void setClockFace(byte face)
{
  strcpy_P(clockFace, PSTR("/00system/digits_"));
  char tickTock[2];
  itoa(face, tickTock, 10);
  strcat(clockFace, tickTock);
  strcat(clockFace, ".bmp");
  if (sd.exists(clockFace)) EEPROM.update(5, face);
}

void clockDigit_1()
{
  char numChar[3];
  itoa(currentHour, numChar, 10);
  byte singleDigit = numChar[0] - '0';
  offsetX = -1;
  if (currentHour >= 10)
  {
    offsetY = singleDigit * 16;
  }
  else offsetY = 160;
  if (!clockSet && !clockDigitSet && !clockSetBlink)
  {
    offsetY = 160;
  }
  bmpDraw(clockFace, 0, 0);
}

void clockDigit_2()
{
  char numChar[3];
  itoa(currentHour, numChar, 10);
  byte singleDigit;
  if (currentHour >= 10)
  {
    singleDigit = numChar[1] - '0';
  }
  else singleDigit = numChar[0] - '0';
  offsetX = 0;
  offsetY = singleDigit * 16;
  if (!clockSet && !clockDigitSet && !clockSetBlink)
  {
    offsetY = 160;
  }
  bmpDraw(clockFace, 3, 0);
}

void clockDigit_3()
{
  char numChar[3];
  itoa(currentMinute, numChar, 10);
  byte singleDigit;
  if (currentMinute >= 10)
  {
    singleDigit = numChar[0] - '0';
  }
  else singleDigit = 0;
  offsetY = singleDigit * 16;
  if (!clockSet && clockDigitSet && !clockSetBlink)
  {
    offsetY = 160;
  }
  bmpDraw(clockFace, 8, 0);
}

void clockDigit_4()
{
  char numChar[3];
  itoa(currentMinute, numChar, 10);
  byte singleDigit;
  if (currentMinute >= 10)
  {
    singleDigit = numChar[1] - '0';
  }
  else singleDigit = numChar[0] - '0';
  offsetY = singleDigit * 16;
  if (!clockSet && clockDigitSet && !clockSetBlink)
  {
    offsetY = 160;
  }
  bmpDraw(clockFace, 12, 0);
}

void getSecondHandIndex()
{
  if (currentSecond < 16)
  {
    secondHandX = currentSecond;
    secondHandY = 0;
  }
  else if (currentSecond >= 16 && currentSecond < 30)
  {
    secondHandX = 15;
    secondHandY = (currentSecond - 15);
  }
  else if (currentSecond >= 30 && currentSecond < 46)
  {
    secondHandX = (15 - (currentSecond - 30));
    secondHandY = 15;
  }
  else if (currentSecond >= 46)
  {
    secondHandX = 0;
    secondHandY = (15 - (currentSecond - 45));
  }
}

void secondHand()
{
  getSecondHandIndex();
  leds[getIndex(secondHandX, secondHandY)] = secondHandColor;
}

// .INI file support

void printErrorMessage(uint8_t e, bool eol = true)
{
  switch (e) {
    case IniFile::errorNoError:
      Serial.print(F("no error"));
      break;
    case IniFile::errorFileNotFound:
      Serial.print(F("fnf"));
      break;
    case IniFile::errorFileNotOpen:
      Serial.print(F("fno"));
      break;
    case IniFile::errorBufferTooSmall:
      Serial.print(F("bts"));
      break;
    case IniFile::errorSeekError:
      Serial.print(F("se"));
      break;
    case IniFile::errorSectionNotFound:
      Serial.print(F("snf"));
      break;
    case IniFile::errorKeyNotFound:
      Serial.print(F("knf"));
      break;
    case IniFile::errorEndOfFile:
      Serial.print(F("eof"));
      break;
    case IniFile::errorUnknownError:
      Serial.print(F("unknown"));
      break;
    default:
      Serial.print(F("unknown error value"));
      break;
  }
  if (eol)
    Serial.println();
}

bool readIniFile()
{
  bool iniExists = false;
  const size_t bufferLen = 50;
  char buffer[bufferLen];
  char configFile[11];
  strcpy_P(configFile, PSTR("config.ini"));
  const char *filename = configFile;
  IniFile ini(filename);
  if (!ini.open()) {
    Serial.print(filename);
    Serial.println(F(" does not exist"));
    // Cannot do anything else
  }
  else
  {
    Serial.println(F("Ini file exists"));
    iniExists = true;
  }

  // Check the file is valid. This can be used to warn if any lines
  // are longer than the buffer.
  if (!ini.validate(buffer, bufferLen)) {
    Serial.print(F("ini file "));
    Serial.print(ini.getFilename());
    Serial.print(F(" not valid: "));
    printErrorMessage(ini.getError());
    // Cannot do anything else
  }
  char section[10];
  strcpy_P(section, PSTR("animation"));
  char entry[11];
  strcpy_P(entry, PSTR("hold"));

  // Fetch a value from a key which is present
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    Serial.print(F("hold value: "));
    Serial.println(buffer);
    holdTime = atol(buffer);
  }
  else {
    printErrorMessage(ini.getError());
    holdTime = 200;
  }

  strcpy_P(entry, PSTR("loop"));

  // Fetch a boolean value
  bool loopCheck;
  bool found = ini.getValue(section, entry, buffer, bufferLen, loopCheck);
  if (found) {
    Serial.print(F("animation loop value: "));
    // Print value, converting boolean to a string
    Serial.println(loopCheck ? F("TRUE") : F("FALSE"));
    folderLoop = loopCheck;
  }
  else {
    printErrorMessage(ini.getError());
    folderLoop = true;
  }

  strcpy_P(entry, PSTR("finish"));

  // Fetch a boolean value
  bool finishCheck;
  bool found4 = ini.getValue(section, entry, buffer, bufferLen, finishCheck);
  if (found4) {
    Serial.print(F("finish value: "));
    // Print value, converting boolean to a string
    Serial.println(finishCheck ? F("TRUE") : F("FALSE"));
    finishBeforeProgressing = finishCheck;
  }
  else {
    printErrorMessage(ini.getError());
    finishBeforeProgressing = false;
  }

  // fade in length in ms
  strcpy_P(entry, PSTR("fadein"));

  // Fetch a value from a key which is present
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    Serial.print(F("fadein value: "));
    Serial.println(buffer);
    fadeLength = atoi(buffer);
  }
  else {
    printErrorMessage(ini.getError());
    fadeLength = fadeLengthGlobal;
  }

  strcpy_P(section, PSTR("translate"));
  strcpy_P(entry, PSTR("moveX"));

  // Fetch a value from a key which is present
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    Serial.print(F("moveX value: "));
    Serial.println(buffer);
    offsetSpeedX = atoi(buffer);
  }
  else {
    printErrorMessage(ini.getError());
    offsetSpeedX = 0;
  }

  strcpy_P(entry, PSTR("moveY"));

  // Fetch a value from a key which is present
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    Serial.print(F("moveY value: "));
    Serial.println(buffer);
    offsetSpeedY = atoi(buffer);
  }
  else {
    printErrorMessage(ini.getError());
    offsetSpeedY = 0;
  }

  strcpy_P(entry, PSTR("loop"));

  // Fetch a boolean value
  bool loopCheck2;
  bool found2 = ini.getValue(section, entry, buffer, bufferLen, loopCheck2);
  if (found2) {
    Serial.print(F("translate loop value: "));
    // Print value, converting boolean to a string
    Serial.println(loopCheck2 ? F("TRUE") : F("FALSE"));
    moveLoop = loopCheck2;
  }
  else {
    printErrorMessage(ini.getError());
    moveLoop = false;
  }

  strcpy_P(entry, PSTR("panoff"));

  // Fetch a boolean value
  bool loopCheck3;
  bool found3 = ini.getValue(section, entry, buffer, bufferLen, loopCheck3);
  if (found3) {
    Serial.print(F("panoff value: "));
    // Print value, converting boolean to a string
    Serial.println(loopCheck3 ? F("TRUE") : F("FALSE"));
    panoff = loopCheck3;
  }
  else {
    printErrorMessage(ini.getError());
    panoff = true;
  }

  strcpy_P(entry, PSTR("nextFolder"));

  // Fetch a value from a key which is present
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    Serial.print(F("nextFolder value: "));
    Serial.println(buffer);
    memcpy(nextFolder, buffer, 8);
  }
  else {
    printErrorMessage(ini.getError());
    nextFolder[0] = '\0';
  }

  if (ini.isOpen()) ini.close();
  return iniExists;
}

void readClockIni()
{
  clockIniRead = true;
  const size_t bufferLen = 50;
  char buffer[bufferLen];
  char configFile[11];
  strcpy_P(configFile, PSTR("clock.ini"));
  const char *filename = configFile;
  IniFile ini(filename);
  sd.chdir("/00system");
  if (!ini.open()) {
    Serial.print(filename);
    Serial.println(F(" does not exist"));
    // Cannot do anything else
  }
  else
  {
    Serial.println(F("Ini file exists"));
  }

  // Check the file is valid. This can be used to warn if any lines
  // are longer than the buffer.
  if (!ini.validate(buffer, bufferLen)) {
    Serial.print(F("ini file "));
    Serial.print(ini.getFilename());
    Serial.print(F(" not valid: "));
    printErrorMessage(ini.getError());
    // Cannot do anything else
  }
  char section[10];
  strcpy_P(section, PSTR("clock"));
  char entry[11];
  strcpy_P(entry, PSTR("hour12"));

  // Fetch a boolean value
  // 12/24 hour mode
  bool loopCheck;
  bool found = ini.getValue(section, entry, buffer, bufferLen, loopCheck);
  if (found) {
    Serial.print(F("hour12 value: "));
    // Print value, converting boolean to a string
    Serial.println(loopCheck ? F("TRUE") : F("FALSE"));
    hour12 = loopCheck;
  }
  else {
    printErrorMessage(ini.getError());
    hour12 = true;
  }

  strcpy_P(entry, PSTR("second"));

  // Fetch a boolean value
  // show second hand
  found = ini.getValue(section, entry, buffer, bufferLen, loopCheck);
  if (found) {
    Serial.print(F("second value: "));
    // Print value, converting boolean to a string
    Serial.println(loopCheck ? F("TRUE") : F("FALSE"));
    enableSecondHand = loopCheck;
  }
  else {
    printErrorMessage(ini.getError());
    enableSecondHand = true;
  }

  strcpy_P(entry, PSTR("offset"));

  // Fetch a value from a key which is present
  // second hand position offset
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    Serial.print(F("offset value: "));
    Serial.println(buffer);
    secondOffset = atoi(buffer);
  }
  else {
    printErrorMessage(ini.getError());
    secondOffset = 0;
  }

  strcpy_P(entry, PSTR("anim"));

  // Fetch a value from a key which is present
  // clock animation length in seconds
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    Serial.print(F("anim value: "));
    Serial.println(buffer);
    clockAnimationLength = atoi(buffer);
  }
  else {
    printErrorMessage(ini.getError());
    clockAnimationLength = 5;
  }

  strcpy_P(entry, PSTR("adjust"));

  // Fetch a value from a key which is present
  // daily midnight offset in seconds
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    Serial.print(F("adjust value: "));
    Serial.println(buffer);
    clockAdjust = atoi(buffer);
  }
  else {
    printErrorMessage(ini.getError());
    clockAdjust = 0;
  }

  if (ini.isOpen()) ini.close();
  sd.chdir("/");
}

void APANativeBrightnessCheck()
{
  const size_t bufferLen = 50;
  char buffer[bufferLen];
  char configFile[11];
  strcpy_P(configFile, PSTR("wifi.ini"));
  const char *filename = configFile;
  IniFile ini(filename);
  sd.chdir("/00system/wifi");
  if (!ini.open()) {
    Serial.print(filename);
    Serial.println(F(" does not exist"));
    // Cannot do anything else
  }
  else
  {
    Serial.println(F("Ini file exists"));
  }

  // Check the file is valid. This can be used to warn if any lines
  // are longer than the buffer.
  if (!ini.validate(buffer, bufferLen)) {
    Serial.print(F("ini file "));
    Serial.print(ini.getFilename());
    Serial.print(F(" not valid: "));
    printErrorMessage(ini.getError());
    // Cannot do anything else
  }
  char section[8];
  strcpy_P(section, PSTR("network"));
  char entry[10];
  strcpy_P(entry, PSTR("apabright"));

  // Fetch a boolean value
  // Native APA Brightness?
  bool apaCheck;
  bool apaFound = ini.getValue(section, entry, buffer, bufferLen, apaCheck);
  if (apaFound) {
    Serial.print(F("apabright: "));
    // Print value, converting boolean to a string
    Serial.println(apaCheck ? F("TRUE") : F("FALSE"));
    APANativeBrightness = apaCheck;
    if (APANativeBrightness) stripSetBrightness();
  }
  else {
    printErrorMessage(ini.getError());
  }

  // fade in length in ms
  strcpy_P(entry, PSTR("fadein"));

  // Fetch a value from a key which is present
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    Serial.print(F("fadein value: "));
    Serial.println(buffer);
    fadeLengthGlobal = atoi(buffer);
  }
  else {
    printErrorMessage(ini.getError());
    fadeLengthGlobal = 0;
  }
}

void clearEEPROM()
{
  Serial.println("Clearing EEPROM...");
  EEPROM.clear();
  Serial.println("Done!");
}

void networkConnect()
{

  // are we recovering from a powered down state?
  uint8_t powerState;
  EEPROM.get(201, powerState);

  // read network.ini
  bool iniExists = false;
  bool useIniWifi = false;
  bool newCredentials = false;
  bool useStaticIP = false;
  bool newStaticIP = false;
  const size_t bufferLen = 65;
  char buffer[bufferLen];
  char networkSSID[33];
  char networkSSIDEEPROM[33];
  EEPROM.get(80, networkSSIDEEPROM); // retrieve stored WIFI SSID starting from EEPROM address 80
  char networkPass[65];
  char networkPassEEPROM[65];
  EEPROM.get(10, networkPassEEPROM); // retrieve stored WIFI pass starting from EEPROM address 10
  int authValue = -1;
  int cipherValue = -1;
  int authEEPROM = -2;
  int cipherEEPROM = -2;
  EEPROM.get(120, authEEPROM); // retrieve stored WIFI pass starting from EEPROM address 10
  EEPROM.get(124, cipherEEPROM); // retrieve stored WIFI pass starting from EEPROM address 10
  char configFile[12];
  IPAddress myAddress;
  IPAddress netmask;
  IPAddress gateway;
  IPAddress dns;
  strcpy_P(configFile, PSTR("wifi.ini"));
  const char *filename = configFile;
  IniFile ini(filename);
  sd.chdir("/00system/wifi");
  if (!ini.open()) {
    Serial.print(filename);
    Serial.println(F(" does not exist"));
    // Cannot do anything else
  }
  else
  {
    Serial.print(filename);
    Serial.println(F(" file exists"));
    iniExists = true;
    wifiFolderFound = true;
  }

  // Check the file is valid. This can be used to warn if any lines
  // are longer than the buffer.
  if (!ini.validate(buffer, bufferLen)) {
    Serial.print(F("ini file "));
    Serial.print(ini.getFilename());
    Serial.print(F(" not valid: "));
    printErrorMessage(ini.getError());
    // Cannot do anything else
  }
  char section[8];
  strcpy_P(section, PSTR("network"));
  char entry[7];
  strcpy_P(entry, PSTR("wifi"));

  // Fetch a boolean value
  // enable WIFI?
  bool wifiCheck;
  bool wifiFound = ini.getValue(section, entry, buffer, bufferLen, wifiCheck);
  if (wifiFound) {
    Serial.print(F("Enable WIFI: "));
    // Print value, converting boolean to a string
    Serial.println(wifiCheck ? F("TRUE") : F("FALSE"));
    enableWifi = wifiCheck;
  }
  else {
    printErrorMessage(ini.getError());
    enableWifi = false;
  }

  strcpy_P(entry, PSTR("inifile"));

  // Fetch a boolean value
  // use inifile credentials?
  bool useIniCheck;
  bool useIniFound = ini.getValue(section, entry, buffer, bufferLen, useIniCheck);
  if (useIniFound) {
    Serial.print(F("IniFile: "));
    // Print value, converting boolean to a string
    Serial.println(useIniCheck ? F("TRUE") : F("FALSE"));
    useIniWifi = useIniCheck;
  }
  else {
    printErrorMessage(ini.getError());
    useIniWifi = false;
  }

  strcpy_P(entry, PSTR("ssid"));

  // Fetch a value from a key which is present
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    strcpy(networkSSID, buffer);
    Serial.print(F("ssid: "));
    Serial.println(networkSSID);
    for (byte i=0; i<64; i++)
    {
      if (networkSSID[i] != networkSSIDEEPROM[i] && useIniWifi)
      {
        Serial.println("New SSID");
        newCredentials = true;
        EEPROM.put(80, networkSSID);
        break;
      }
      if (networkSSID[i] == '\0') break;
    }
  }
  else {
    printErrorMessage(ini.getError());
  }

  strcpy_P(entry, PSTR("pass"));

  // Fetch a value from a key which is present
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    strcpy(networkPass, buffer);
    Serial.print(F("password: "));
    Serial.println(networkPass);
    for (byte i=0; i<32; i++)
    {
      if (networkPass[i] != networkPassEEPROM[i] && useIniWifi)
      {
        Serial.println("New Password");
        newCredentials = true;
        EEPROM.put(10, networkPass);
        break;
      }
      if (networkPass[i] == '\0') break;
    }
  }
  else {
    printErrorMessage(ini.getError());
  }

  strcpy_P(entry, PSTR("auth"));

  // Fetch a value from a key which is present
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    authValue = atoi(buffer);
    Serial.print(F("security: "));
    Serial.println(authValue);
  }
  else {
    printErrorMessage(ini.getError());
  }
  // save to EEPROM
  if (authValue != authEEPROM && useIniWifi)
  {
    Serial.println("New WIFI security");
    EEPROM.put(120, authValue);
    newCredentials = true;
  }

  strcpy_P(entry, PSTR("cipher"));

  // Fetch a value from a key which is present
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    cipherValue = atoi(buffer);
    Serial.print(F("cipher: "));
    Serial.println(cipherValue);
    // don't confuse my studid code
    if (cipherValue == 0) cipherValue = -1;
  }
  else {
    printErrorMessage(ini.getError());
  }
  // save to EEPROM
  if (cipherValue != cipherEEPROM && useIniWifi)
  {
    Serial.println("New WIFI cipher");
    EEPROM.put(124, cipherValue);
    newCredentials = true;
  }

  strcpy_P(entry, PSTR("static"));

  // Fetch a boolean value
  // use static IP?
  bool boolCheck;
  bool found = ini.getValue(section, entry, buffer, bufferLen, boolCheck);
  if (found) {
    Serial.print(F("use static ip: "));
    // Print value, converting boolean to a string
    Serial.println(boolCheck ? F("TRUE") : F("FALSE"));
    useStaticIP = boolCheck;
  }
  else {
    printErrorMessage(ini.getError());
    useStaticIP = false;
  }

  strcpy_P(entry, PSTR("ip"));
  // Fetch a value from a key which is present
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    if (strlen(buffer) > 0)
    {
      uint8_t i0 = atoi(strtok(buffer, "."));
      uint8_t i1 = atoi(strtok(NULL, "."));
      uint8_t i2 = atoi(strtok(NULL, "."));
      uint8_t i3 = atoi(strtok(NULL, "."));
      uint8_t server[] = {i0, i1, i2, i3};
      myAddress = server;

      // check against EEPROM
      if (i0 != EEPROM.read(130) || i1 != EEPROM.read(131) || i2 != EEPROM.read(132) || i3 != EEPROM.read(133))
      {
        if (useIniWifi)
        {
          EEPROM.update(130, i0);
          EEPROM.update(131, i1);
          EEPROM.update(132, i2);
          EEPROM.update(133, i3);
          newStaticIP = true;
        }
      }

      Serial.print("static ip: ");
      Serial.print(i0);
      Serial.print(".");
      Serial.print(i1);
      Serial.print(".");
      Serial.print(i2);
      Serial.print(".");
      Serial.println(i3);
    }
  }
  else {
    printErrorMessage(ini.getError());
  }

  strcpy_P(entry, PSTR("netmask"));
  // Fetch a value from a key which is present
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    if (strlen(buffer) > 0)
    {
      uint8_t i0 = atoi(strtok(buffer, "."));
      uint8_t i1 = atoi(strtok(NULL, "."));
      uint8_t i2 = atoi(strtok(NULL, "."));
      uint8_t i3 = atoi(strtok(NULL, "."));
      uint8_t server[] = {i0, i1, i2, i3};
      netmask = server;

      // check against EEPROM
      if (i0 != EEPROM.read(134) || i1 != EEPROM.read(135) || i2 != EEPROM.read(136) || i3 != EEPROM.read(137))
      {
        if (useIniWifi)
        {
          EEPROM.update(134, i0);
          EEPROM.update(135, i1);
          EEPROM.update(136, i2);
          EEPROM.update(137, i3);
          newStaticIP = true;
        }
      }

      Serial.print("netmask: ");
      Serial.print(i0);
      Serial.print(".");
      Serial.print(i1);
      Serial.print(".");
      Serial.print(i2);
      Serial.print(".");
      Serial.println(i3);
    }
  }
  else {
    printErrorMessage(ini.getError());
  }

  strcpy_P(entry, PSTR("gateway"));
  // Fetch a value from a key which is present
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    if (strlen(buffer) > 0)
    {
      uint8_t i0 = atoi(strtok(buffer, "."));
      uint8_t i1 = atoi(strtok(NULL, "."));
      uint8_t i2 = atoi(strtok(NULL, "."));
      uint8_t i3 = atoi(strtok(NULL, "."));
      uint8_t server[] = {i0, i1, i2, i3};
      gateway = server;

      // check against EEPROM
      if (i0 != EEPROM.read(138) || i1 != EEPROM.read(139) || i2 != EEPROM.read(140) || i3 != EEPROM.read(141))
      {
        if (useIniWifi)
        {
          EEPROM.update(138, i0);
          EEPROM.update(139, i1);
          EEPROM.update(140, i2);
          EEPROM.update(141, i3);
          newStaticIP = true;
        }
      }

      Serial.print("gateway: ");
      Serial.print(i0);
      Serial.print(".");
      Serial.print(i1);
      Serial.print(".");
      Serial.print(i2);
      Serial.print(".");
      Serial.println(i3);
    }
  }
  else {
    printErrorMessage(ini.getError());
  }

  strcpy_P(entry, PSTR("dns"));
  // Fetch a value from a key which is present
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    if (strlen(buffer) > 0)
    {
      uint8_t i0 = atoi(strtok(buffer, "."));
      uint8_t i1 = atoi(strtok(NULL, "."));
      uint8_t i2 = atoi(strtok(NULL, "."));
      uint8_t i3 = atoi(strtok(NULL, "."));
      uint8_t server[] = {i0, i1, i2, i3};
      dns = server;

      // check against EEPROM
      if (i0 != EEPROM.read(142) || i1 != EEPROM.read(143) || i2 != EEPROM.read(144) || i3 != EEPROM.read(145))
      {
        if (useIniWifi)
        {
          EEPROM.update(142, i0);
          EEPROM.update(143, i1);
          EEPROM.update(144, i2);
          EEPROM.update(145, i3);
          newStaticIP = true;
        }
      }

      Serial.print("dns: ");
      Serial.print(i0);
      Serial.print(".");
      Serial.print(i1);
      Serial.print(".");
      Serial.print(i2);
      Serial.print(".");
      Serial.println(i3);
    }
  }
  else {
    printErrorMessage(ini.getError());
  }

  if (ini.isOpen()) ini.close();
  sd.chdir("/");

  if (enableWifi)
  {
    // use ini file credentials
    if (useIniWifi)
    {
      // Static IP or DHCP?
      if (useStaticIP)
      {
        Serial.println("Using Static IP Address");
        if (newStaticIP)
        {
          WiFi.setStaticIP(myAddress, netmask, gateway, dns);
        }
        WiFi.useStaticIP();
      }
      else
      {
        Serial.println("Using DHCP");
        WiFi.useDynamicIP();
      }

      if (Particle.connected() == false && iniExists) {
        // don't draw graphic if recovering from power off state
        if (powerState != 128) bmpDraw("/00system/wifi/white.bmp", 0, 0);
        Serial.println("WiFi On...");
        WiFi.on();
        if (newCredentials)
        {
          Serial.println("Clearing Credentials...");
          WiFi.clearCredentials();

          // set Credentials
          Serial.println("Setting New Credentials...");
          if (authValue > -1 && cipherValue > -1)
          {
            WiFi.setCredentials(networkSSID, networkPass, authValue, cipherValue);
          }
          else if (authValue > -1)
          {
            WiFi.setCredentials(networkSSID, networkPass, authValue);
          }
          else WiFi.setCredentials(networkSSID, networkPass);
        }
        else Serial.println("Using existing credentials...");

        WiFi.connect(WIFI_CONNECT_SKIP_LISTEN);
        if (WiFi.hasCredentials())
        {
          Serial.println("We have Credentials! Connecting to Cloudz...");
          Particle.connect();
          // don't draw graphic if recovering from power off state
          if (powerState != 128) bmpDraw("/00system/wifi/green.bmp", 0, 0);
          long connectTimeout = millis() + 30000;
          long progressMeter = millis() + 500;
          while(!Particle.connected())
          {
            Particle.process();
            // timneout if no connection is established
            // system will continue trying in the background
            if (millis() > connectTimeout)
            {
              bmpDraw("/00system/wifi/red.bmp", 0, 0);
              delay(2000);
              break;
            }
            if (millis() > progressMeter)
            {
              Serial.print(".");
              progressMeter += 500;
            }
          }
          Serial.println("Connected to:");
          Serial.println(WiFi.localIP());
          Serial.println(WiFi.subnetMask());
          Serial.println(WiFi.gatewayIP());
          Serial.println(WiFi.SSID());
          randomSeed(Time.now());
        }
        else
        {
          // If INI credentials match EEPROM but stored credentials are missing...
          Serial.println("Credential loss detected. Restoring from INI.");
          WiFi.disconnect();
          Serial.println("Clearing Credentials...");
          WiFi.clearCredentials();

          // set Credentials
          Serial.println("Setting New Credentials...");
          if (authValue > -1 && cipherValue > -1)
          {
            WiFi.setCredentials(networkSSID, networkPass, authValue, cipherValue);
          }
          else if (authValue > -1)
          {
            WiFi.setCredentials(networkSSID, networkPass, authValue);
          }
          else WiFi.setCredentials(networkSSID, networkPass);

          WiFi.connect(WIFI_CONNECT_SKIP_LISTEN);

          // let's try this again
          if (WiFi.hasCredentials())
          {
            Serial.println("We have Credentials! Connecting to Cloud...");
            Particle.connect();
            // don't draw graphic if recovering from power off state
            if (powerState != 128) bmpDraw("/00system/wifi/green.bmp", 0, 0);
            long connectTimeout = millis() + 30000;
            long progressMeter = millis() + 500;
            while(!Particle.connected())
            {
              Particle.process();
              // timneout if no connection is established
              // system will continue trying in the background
              if (millis() > connectTimeout)
              {
                bmpDraw("/00system/wifi/red.bmp", 0, 0);
                delay(2000);
                break;
              }
              if (millis() > progressMeter)
              {
                Serial.print(".");
                progressMeter += 500;
              }
            }
            Serial.println("Connected to:");
            Serial.println(WiFi.localIP());
            Serial.println(WiFi.subnetMask());
            Serial.println(WiFi.gatewayIP());
            Serial.println(WiFi.SSID());
            randomSeed(Time.now());
          }
          else Serial.println("We have NO Credentials.");
        }
      }
    }
    // use stored credentials
    else
    {
      if (Particle.connected() == false && iniExists) {
        // don't draw graphic if recovering from power off state
        if (powerState != 128) bmpDraw("/00system/wifi/white.bmp", 0, 0);
        Serial.println("WiFi On...");
        WiFi.on();
        Serial.println("Using stored credentials...");
        WiFi.connect(WIFI_CONNECT_SKIP_LISTEN);
        if (WiFi.hasCredentials())
        {
          Serial.println("We have Credentials! Connecting to Cloud...");
          Particle.connect();
          // don't draw graphic if recovering from power off state
          if (powerState != 128) bmpDraw("/00system/wifi/green.bmp", 0, 0);
          long connectTimeout = millis() + 30000;
          long progressMeter = millis() + 500;
          while(!Particle.connected())
          {
            Particle.process();
            // timneout if no connection is established
            // system will continue trying in the background
            if (millis() > connectTimeout)
            {
              bmpDraw("/00system/wifi/red.bmp", 0, 0);
              delay(2000);
              break;
            }
            if (millis() > progressMeter)
            {
              Serial.print(".");
              progressMeter += 500;
            }
          }
          Serial.println("Connected to:");
          Serial.println(WiFi.localIP());
          Serial.println(WiFi.subnetMask());
          Serial.println(WiFi.gatewayIP());
          Serial.println(WiFi.SSID());
          randomSeed(Time.now());
        }
        else Serial.println("We have NO Credentials.");
      }
    }
  }
  Serial.println("---");
  Serial.print("deviceID: ");
  String myID = System.deviceID();
  Serial.println(myID);
  Serial.printlnf("System version: %s", System.version().c_str());
  uint32_t freemem = System.freeMemory();
  Serial.print("free memory: ");
  Serial.println(freemem);
}

// Auto Brightness Control
void readABC()
{
  const size_t bufferLen = 50;
  char buffer[bufferLen];
  char configFile[11];
  strcpy_P(configFile, PSTR("clock.ini"));
  const char *filename = configFile;
  IniFile ini(filename);
  sd.chdir("/00system");
  if (!ini.open()) {
    Serial.print(filename);
    Serial.println(F(" does not exist"));
    // Cannot do anything else
  }
  else
  {
    Serial.println(F("Ini file exists"));
  }

  // Check the file is valid. This can be used to warn if any lines
  // are longer than the buffer.
  if (!ini.validate(buffer, bufferLen)) {
    Serial.print(F("ini file "));
    Serial.print(ini.getFilename());
    Serial.print(F(" not valid: "));
    printErrorMessage(ini.getError());
    // Cannot do anything else
  }
  char section[10];
  strcpy_P(section, PSTR("abc"));
  char entry[11];
  strcpy_P(entry, PSTR("abc"));

  // Fetch a boolean value
  // Use ABC?
  bool boolcheck;
  bool found = ini.getValue(section, entry, buffer, bufferLen, boolcheck);
  if (found) {
    Serial.print(F("abc value: "));
    // Print value, converting boolean to a string
    Serial.println(boolcheck ? F("TRUE") : F("FALSE"));
    abc = boolcheck;
  }
  else {
    printErrorMessage(ini.getError());
    abc = false;
  }

  //  check INI for 1440 time of day entries (takes too long)
  //  int minuteInDay = 0;
  //
  //  for (int h=0; h<24; h++)
  //  {
  //    for (int m=0; m<60; m++)
  //    {
  //      // construct time of day character array
  //      char timeOfDay[6];
  //      char minuteChar[3];
  //      itoa(h, timeOfDay, 10);
  //      strcat(timeOfDay, ":");
  //      itoa(m, minuteChar, 10);
  //      if (strlen(minuteChar) == 1) strcat(timeOfDay, "0");
  //      strcat(timeOfDay, minuteChar);
  //
  //      // Fetch a value from a key which is present
  //      // abc brightness level 0
  //      buffer[0] = '\0';
  //      if (ini.getValue(section, timeOfDay, buffer, bufferLen)) {
  //        if (strlen(buffer) > 0)
  //        {
  //          Abc abc0 = {minuteInDay, atoi(buffer)};
  //          Serial.print(timeOfDay);
  //          Serial.print(F(" brightness: "));
  //          Serial.println(atoi(buffer));
  //        }
  //      }
  //      minuteInDay++;
  //    }
  //  }

  strcpy_P(entry, PSTR("abc0"));
  // Fetch a value from a key which is present
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    if (strlen(buffer) > 0)
    {
      int h = atoi(strtok(buffer, ":"));
      int m = atoi(strtok(NULL, ":,"));
      abc0.m = minuteCounter(h, m);
      abc0.b = atoi(strtok(NULL, " ,"));
    }
    Serial.print("abc0 value: ");
    Serial.print(abc0.m);
    Serial.print(", ");
    Serial.println(abc0.b);
  }
  else {
    printErrorMessage(ini.getError());
  }

  strcpy_P(entry, PSTR("abc1"));
  // Fetch a value from a key which is present
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    if (strlen(buffer) > 0)
    {
      int h = atoi(strtok(buffer, ":"));
      int m = atoi(strtok(NULL, ":,"));
      abc1.m = minuteCounter(h, m);
      abc1.b = atoi(strtok(NULL, " ,"));
    }
    Serial.print("abc1 value: ");
    Serial.print(abc1.m);
    Serial.print(", ");
    Serial.println(abc1.b);
  }
  else {
    printErrorMessage(ini.getError());
  }

  strcpy_P(entry, PSTR("abc2"));
  // Fetch a value from a key which is present
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    if (strlen(buffer) > 0)
    {
      int h = atoi(strtok(buffer, ":"));
      int m = atoi(strtok(NULL, ":,"));
      abc2.m = minuteCounter(h, m);
      abc2.b = atoi(strtok(NULL, " ,"));
    }
    Serial.print("abc2 value: ");
    Serial.print(abc2.m);
    Serial.print(", ");
    Serial.println(abc2.b);
  }
  else {
    printErrorMessage(ini.getError());
  }

  strcpy_P(entry, PSTR("abc3"));
  // Fetch a value from a key which is present
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    if (strlen(buffer) > 0)
    {
      int h = atoi(strtok(buffer, ":"));
      int m = atoi(strtok(NULL, ":,"));
      abc3.m = minuteCounter(h, m);
      abc3.b = atoi(strtok(NULL, " ,"));
    }
    Serial.print("abc3 value: ");
    Serial.print(abc3.m);
    Serial.print(", ");
    Serial.println(abc3.b);
  }
  else {
    printErrorMessage(ini.getError());
  }

  strcpy_P(entry, PSTR("abc4"));
  // Fetch a value from a key which is present
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    if (strlen(buffer) > 0)
    {
      int h = atoi(strtok(buffer, ":"));
      int m = atoi(strtok(NULL, ":,"));
      abc4.m = minuteCounter(h, m);
      abc4.b = atoi(strtok(NULL, " ,"));
    }
    Serial.print("abc4 value: ");
    Serial.print(abc4.m);
    Serial.print(", ");
    Serial.println(abc4.b);
  }
  else {
    printErrorMessage(ini.getError());
  }

  strcpy_P(entry, PSTR("abc5"));
  // Fetch a value from a key which is present
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    if (strlen(buffer) > 0)
    {
      int h = atoi(strtok(buffer, ":"));
      int m = atoi(strtok(NULL, ":,"));
      abc5.m = minuteCounter(h, m);
      abc5.b = atoi(strtok(NULL, " ,"));
    }
    Serial.print("abc5 value: ");
    Serial.print(abc5.m);
    Serial.print(", ");
    Serial.println(abc5.b);
  }
  else {
    printErrorMessage(ini.getError());
  }

  strcpy_P(entry, PSTR("abc6"));
  // Fetch a value from a key which is present
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    if (strlen(buffer) > 0)
    {
      int h = atoi(strtok(buffer, ":"));
      int m = atoi(strtok(NULL, ":,"));
      abc6.m = minuteCounter(h, m);
      abc6.b = atoi(strtok(NULL, " ,"));
    }
    Serial.print("abc6 value: ");
    Serial.print(abc6.m);
    Serial.print(", ");
    Serial.println(abc6.b);
  }
  else {
    printErrorMessage(ini.getError());
  }

  strcpy_P(entry, PSTR("abc7"));
  // Fetch a value from a key which is present
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    if (strlen(buffer) > 0)
    {
      int h = atoi(strtok(buffer, ":"));
      int m = atoi(strtok(NULL, ":,"));
      abc7.m = minuteCounter(h, m);
      abc7.b = atoi(strtok(NULL, " ,"));
    }
    Serial.print("abc7 value: ");
    Serial.print(abc7.m);
    Serial.print(", ");
    Serial.println(abc7.b);
  }
  else {
    printErrorMessage(ini.getError());
  }

  strcpy_P(entry, PSTR("abc8"));
  // Fetch a value from a key which is present
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    if (strlen(buffer) > 0)
    {
      int h = atoi(strtok(buffer, ":"));
      int m = atoi(strtok(NULL, ":,"));
      abc8.m = minuteCounter(h, m);
      abc8.b = atoi(strtok(NULL, " ,"));
    }
    Serial.print("abc8 value: ");
    Serial.print(abc8.m);
    Serial.print(", ");
    Serial.println(abc8.b);
  }
  else {
    printErrorMessage(ini.getError());
  }

  strcpy_P(entry, PSTR("abc9"));
  // Fetch a value from a key which is present
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    if (strlen(buffer) > 0)
    {
      int h = atoi(strtok(buffer, ":"));
      int m = atoi(strtok(NULL, ":,"));
      abc9.m = minuteCounter(h, m);
      abc9.b = atoi(strtok(NULL, " ,"));
    }
    Serial.print("abc9 value: ");
    Serial.print(abc9.m);
    Serial.print(", ");
    Serial.println(abc9.b);
  }
  else {
    printErrorMessage(ini.getError());
  }

  // Fetch a value from a key which is present
  if (ini.getValue(section, "colorCorrection", buffer, bufferLen)) {
    if (strlen(buffer) > 0)
    {
      colorCorrection = strtoul(buffer, NULL, 16);
    }
    Serial.print("Color Correction: ");
    Serial.println(buffer);
  }
  else {
    printErrorMessage(ini.getError());
    colorCorrection = 0xFFFFFF;
  }

  // Fetch a value from a key which is present
  if (ini.getValue(section, "colorTemperature", buffer, bufferLen)) {
    if (strlen(buffer) > 0)
    {
      colorTemperature = strtoul(buffer, NULL, 16);
    }
    Serial.print("Color Temperature: ");
    Serial.println(buffer);
  }
  else {
    printErrorMessage(ini.getError());
    colorTemperature = 0xFFFFFF;
  }

  if (ini.isOpen()) ini.close();
  sd.chdir("/");
}

void readRemoteIni()
{
  const size_t bufferLen = 50;
  char buffer[bufferLen];
  char configFile[11];
  strcpy_P(configFile, PSTR("remote.ini"));
  const char *filename = configFile;
  IniFile ini(filename);
  sd.chdir("/00system");
  if (!ini.open()) {
    Serial.print(filename);
    Serial.println(F(" does not exist"));
    // Cannot do anything else
  }
  else
  {
    Serial.println(F("Ini file exists"));
  }

  // Check the file is valid. This can be used to warn if any lines
  // are longer than the buffer.
  if (!ini.validate(buffer, bufferLen)) {
    Serial.print(F("ini file "));
    Serial.print(ini.getFilename());
    Serial.print(F(" not valid: "));
    printErrorMessage(ini.getError());
    // Cannot do anything else
  }
  char section[7];
  strcpy_P(section, PSTR("remote"));
  char entry[6];
  strcpy_P(entry, PSTR("menu"));

  // Fetch a value from a key which is present
  // second hand position offset
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    Serial.print(F("IR Menu code: "));
    Serial.println(buffer);
    remoteCodeMenu = strtoull(buffer, NULL, 10);
  }
  else {
    printErrorMessage(ini.getError());
    remoteCodeMenu = 2155864095;
  }

  strcpy_P(entry, PSTR("next"));

  // Fetch a value from a key which is present
  // clock animation length in seconds
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    Serial.print(F("IR Next code: "));
    Serial.println(buffer);
    remoteCodeNext = strtoull(buffer, NULL, 10);
  }
  else {
    printErrorMessage(ini.getError());
    remoteCodeNext = 2155831455;
  }

  strcpy_P(entry, PSTR("power"));

  // Fetch a value from a key which is present
  // daily midnight offset in seconds
  if (ini.getValue(section, entry, buffer, bufferLen)) {
    Serial.print(F("IR Power code: "));
    Serial.println(buffer);
    remoteCodePower = strtoull(buffer, NULL, 10);
  }
  else {
    printErrorMessage(ini.getError());
    remoteCodePower = 2155819215;
  }

  if (ini.isOpen()) ini.close();
  sd.chdir("/");
}

int minuteCounter(byte h, byte m)
{
  int minutes = 0;
  minutes = h * 60;
  minutes += m;
  return minutes;
}

int realRandom(int max)
{
  if (0 == max) {
    return 0;
  }
  return HAL_RNG_GetRandomNumber() % max;
}

int realRandom(int min, int max)
{
  if (min >= max) {
    return min;
  }
  return realRandom(max - min) + min;
}

// breakout code

void drawPaddle()
{
  leds[paddleIndex] = CRGB(200, 200, 200);
  leds[paddleIndex + 1] = CRGB(200, 200, 200);
  leds[paddleIndex + 2] = CRGB(200, 200, 200);
  fastledshow();
}

void breakoutLoop()
{
  // if (holdTime > 0) holdTime--; // paddle
  // if (fileIndex > 0) fileIndex--; // ball

  long currentTime = millis();

  if (buttonEnabled == false)
  {
    if (millis() > buttonTime + 50) buttonEnabled = true;
  }

  // menu button
  if ((digitalRead(buttonNextPin) == LOW || irCommand == 'N' || irNextRepeat == true) && currentTime > movePaddle && paddleIndex < 237 && gameInitialized == true && buttonEnabled == true)
  {
    paddleIndex++;
    leds[paddleIndex - 1] = CRGB(0, 0, 0);
    drawPaddle();
    // paddle speed
    movePaddle = currentTime + delayPaddle;
    if (ballMoving == false)
    {
      delayPaddle = 50, // ms delay for movement
      delayBall = 150, // ms delay for ball
      ballMoving = true;
      ballAngle = random8(190, 225);
    }
  }

  // next button
  else if ((digitalRead(buttonMenuPin) == LOW || irCommand == 'M' || irMenuRepeat == true) && currentTime > movePaddle && paddleIndex > 224 && buttonEnabled == true)
  {
    paddleIndex--;
    leds[paddleIndex + 3] = CRGB(0, 0, 0);
    drawPaddle();
    // paddle speed
    movePaddle = currentTime + delayPaddle;
    if (ballMoving == false)
    {
      delayPaddle = 50, // ms delay for movement
      delayBall = 150, // ms delay for ball
      ballMoving = true;
      ballAngle = random8(135, 170);
    }
  }

  else if ((digitalRead(buttonNextPin) == HIGH || digitalRead(buttonMenuPin) == HIGH || irCommand == 'M' || irCommand == 'N') && gameInitialized == false) gameInitialized = true;

  // ball logic
  if (ballMoving == true && currentTime > moveBall)
  {
    moveBall = currentTime + delayBall;
    leds[ballIndex] = CRGB(0, 0, 0);

    // did the player lose?
    if (ballIndex >= 239)
    {
      ballMoving = false;
      breakout = false;
      Serial.println("Lose!!!");
      for (int c = 250; c >= 0; c = c - 10)
      {
        for (int i = 0; i < NUM_LEDS; i++)
        {
          byte r = 0;
          byte g = 0;
          byte b = 0;
          if (random8(0, 2) == 1)
          {
            r = c;
          }
          if (random8(0, 2) == 1)
          {
            g = c;
          }
          if (random8(0, 2) == 1)
          {
            b = c;
          }
          leds[i] = CRGB(r, g, b);
        }
        fastledshow();
        delay(50);
      }
    }

    // ball still in play
    else
    {
      if (ballAngle < 180)
      {
        if ((ballX + sin(degToRad(ballAngle)) * 16) + .5 > 256) swapXdirection();
      }
      else
      {
        if ((ballX + sin(degToRad(ballAngle)) * 16) + .5 < 0) swapXdirection();
      }
      ballX = ballX + sin(degToRad(ballAngle)) * 16 + .5;

      if (ballAngle > 90 && ballAngle < 270)
      {
        if ((ballY + cos(degToRad(ballAngle)) * 16) + .5 < 0) swapYdirection();
      }
      else
      {
        if ((ballY + cos(degToRad(ballAngle)) * 16) + .5 > 256) swapYdirection();
      }
      ballY = ballY + cos(degToRad(ballAngle)) * 16 + .5;
      ballIndex = getScreenIndex(ballX, ballY);

      // paddle hit?
      if (ballIndex == paddleIndex or ballIndex == paddleIndex + 1 or ballIndex == paddleIndex + 2)
      {
        // move the ball back in time one step
        ballX = ballX + ((sin(degToRad(ballAngle)) * 16 + .5) * -1);
        ballY = ballY + ((cos(degToRad(ballAngle)) * 16 + .5) * -1);
        swapYdirection();
        if (ballIndex == paddleIndex)
        {
          ballAngle = random8(115, 170);
        }
        else if (ballIndex == paddleIndex + 2)
        {
          ballAngle = random8(190, 245);
        }
        ballIndex = getScreenIndex(ballX, ballY);
        leds[paddleIndex] = CRGB(200, 200, 200);
        leds[paddleIndex + 1] = CRGB(200, 200, 200);
        leds[paddleIndex + 2] = CRGB(200, 200, 200);
      }

      // brick hit?
      if (leds[ballIndex].r > 0 || leds[ballIndex].g > 0 || leds[ballIndex].b > 0)
      {
        // speed up and change direction
        delayBall -= 1;

        swapYdirection();
        if (winCheck())
        {
          Serial.println("Win!!!");
          for (byte flashes=0; flashes < 30; flashes++)
          {
            leds[ballIndex] = CRGB(random8(), random8(), random8());
            fastledshow();
            delay(50);
          }
          ballMoving = false;
          chdirFirework();
          char bmpFile[7]; // 2-digit number + .bmp + null byte
          for (byte fileIndex = 0; fileIndex < 37; fileIndex++)
          {
            itoa(fileIndex, bmpFile, 10);
            strcat(bmpFile, ".bmp");
            bmpDraw(bmpFile, 0, 0);
            delay(90);
          }
          breakout = false;
        }
      }

      // check for preceeding win
      if (breakout == true)
      {
        leds[ballIndex] = CRGB(175, 255, 15);
        fastledshow();
      }
    }
  }
}

void chdirFirework()
{
  char tmp[20];
  strcpy_P(tmp, PSTR("/00system/firework"));
  sd.chdir(tmp);
}

boolean winCheck()
{
  byte numberOfLitPixels = 0;
  for (byte i = 0; i < 255; i++)
  {
    if (leds[i].r > 0 || leds[i].b > 0 || leds[i].g > 0)
    {
      numberOfLitPixels++;
    }
  }
  if (numberOfLitPixels <= 4)
  {
    // why is this delay needed?? Function always returns true without it.
    delay(1);
    return true;
  }
  return 0;
}

byte getScreenIndex(byte x, byte y)
{
  byte screenX = x / 16;
  byte screenY = y / 16;
  byte index;
  index = screenY * 16;
  if (screenY == 0)
  {
    index = 15 - screenX;
  }
  else if (screenY % 2 != 0)
  {
    index = (screenY * 16) + screenX;
  }
  else
  {
    index = (screenY * 16 + 15) - screenX;
  }
  return index;
}

void swapYdirection()
{
  if (ballAngle > 90 && ballAngle < 270)
  {
    if (ballAngle > 180)
    {
      ballAngle = 360 - (ballAngle - 180);
    }
    else
    {
      ballAngle = 90 - (ballAngle - 90);
    }
  }
  else
  {
    if (ballAngle < 90)
    {
      ballAngle = 90 + (90 - ballAngle);
    }
    else
    {
      ballAngle = 180 + (360 - ballAngle);
    }
  }
}

void swapXdirection()
{
  if (ballAngle < 180)
  {
    if (ballAngle < 90)
    {
      ballAngle = 270 + (90 - ballAngle);
    }
    else ballAngle = 270 - (ballAngle - 90);
  }
  else
  {
    if (ballAngle > 270)
    {
      ballAngle = 360 - ballAngle;
    }
    else ballAngle = 180 - (ballAngle - 180);
  }
}

float degToRad(float deg)
{
  float result;
  result = deg * 3.14159265359 / 180;
  return result;
}

// These read 16- and 32-bit types from the SD card file.
// BMP data is stored little-endian, Arduino is little-endian too.
// May need to reverse subscript order if porting elsewhere.

uint16_t read16(SdFile& f) {
  readPhase = 16;
  uint16_t result;
  f.read(&result, 2);
  return result;
  readPhase = 0;
}

uint32_t read32(SdFile& f) {
  readPhase = 32;
  uint32_t result;
  f.read(&result, 4);
  return result;
  readPhase = 0;
}

// clock debug function
void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}

// WebServer stuff

// Modified LS function from SDFAT lib to print to webserver
void listFiles(WebServer &server, uint8_t flags, uint8_t indent) {
  char indexDirectory[33] = "/00system/wifi/";
//  sd.vwd()->getName(indexDirectory, 32);
  char indexFile[33];
  uint32_t indexPosition = 0;
  if (myFile.isOpen())
  {
    myFile.getName(indexFile, 32); // store current file
    indexPosition = myFile.curPosition();
  }
  sd.chdir("/");
  closeMyFile();
  int file = 0;
  while (myFile.openNext(sd.vwd(), O_READ)) {
    if (!myFile.isHidden() || (flags & LS_A)) {
      // get folder name
      char folder[33];
      myFile.getName(folder, 32);
      if (myFile.isDir() && folder[0] == 48 && folder[1] == 48) {}
      else
      {
        // print slash if folder
        if (myFile.isDir()) {
          if (file) server.print(",");
          server.print('/');
          server.print(folder);
        }
        else
        {
          if (file) server.print(",");
          server.print(folder);
        }
      }
    }
    myFile.close();
    file++;
  }
  sd.chdir(indexDirectory);
  if (indexFile[0] != '\0')
  {
    Serial.print("Re-opening: ");
    if (indexDirectory[0] != '/') Serial.print("/");
    Serial.print(indexDirectory);
    if (indexDirectory[0] != '/') Serial.print("/");
    Serial.print(indexFile);
    Serial.print(" at ");
    Serial.println(indexPosition);
    myFile.open(indexFile, O_READ);
    myFile.seekSet(indexPosition);
  }
}

// playCmd has no effect in clock mode
void playCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  if (type == WebServer::POST)
  {
    bool repeat;
    char name[33], value[8];
    do
    {
      /* readPOSTparam returns false when there are no more parameters
       * to read from the input.  We pass in buffers for it to store
       * the name and value strings along with the length of those
       * buffers. */
      repeat = server.readPOSTparam(name, 32, value, 8);

      char folderWithSlash[32];
      strcat(folderWithSlash, "/");
      strcat(folderWithSlash, name);

      Serial.println("-=-=-=-=-");
      Serial.print("PLAY FOLDER: ");
      Serial.println(folderWithSlash);
      if (sd.exists(folderWithSlash))
      {
        strcpy_P(nextFolder, name);
        if (displayMode == 0) irCommand = 'N';
        server.httpSuccess("application/json");
        server.println("{\"status\": \"success\", \"message\": \"Folder selected.\"}");
      }
      else
      {
        server.httpSuccess("application/json");
        server.println("{\"status\": \"error\", \"message\": \"Folder does not exist.\"}");
      }
    } while (repeat);
  }
  else server.httpNoContent();
}

// control Game Frame settings directly
void setCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  // power button
  String tail(url_tail);
  if (tail == "power")
  {
    irCommand = 'P';
  }

  // menu button
  else if (tail == "menu")
  {
    irCommand = 'M';
  }

  // next button
  else if (tail == "next")
  {
    irCommand = 'N';
  }

  // brightness
  else if (url_tail[0] == 'b')
  {
    if (brightness == 0 && (atoi(&url_tail[1]) > 0 && webServerActive)) bmpDraw("/00system/wifi/wifi.bmp", 0, 0);
    brightness = atoi(&url_tail[1]);
    stripSetBrightness();
    saveSettingsToEEPROM();
    if (displayMode == 3) drawChart(chartValues);
    else fastledshow();
  }

  // playMode
  else if (url_tail[0] == 'p')
  {
    playMode = atoi(&url_tail[1]);
    if (playMode > 2 || playMode < 0) playMode = 0;
    saveSettingsToEEPROM();
  }

  // cycleTime
  else if (url_tail[0] == 'c')
  {
    cycleTimeSetting = atoi(&url_tail[1]);
    if (cycleTimeSetting > 8 || cycleTimeSetting < 1) cycleTimeSetting = 1;
    setCycleTime();
    saveSettingsToEEPROM();
  }

  // displayMode
  else if (url_tail[0] == 'm')
  {
    displayMode = atoi(&url_tail[1]);
    if (displayMode > displayModeMax || displayMode < 0) displayMode = 0;
    saveSettingsToEEPROM();
    if (displayMode == 1)
    {
      initClock();
    }
    else
    {
      if (clockShown || clockAnimationActive)
      {
        closeMyFile();
        clockAnimationActive = false;
        clockShown = false;
        abortImage = true;
        nextFolder[0] = '\0';
      }
    }
  }

  // clock face
  else if (url_tail[0] == 'f')
  {
    int face = atoi(&url_tail[1]);
    if (face > 0 && face < 6)
    {
      setClockFace(face);
      if (clockShown && !enableSecondHand)
      {
        drawDigits();
        fastledshow();
      }
    }
  }

  // time zone
  else if (url_tail[0] == 'z')
  {
    Serial.println("New Time Zone!");
    int floatLength = strlen(url_tail);
    char gmtOffset[7];
    gmtOffset[0] = url_tail[1];
    for (int i=2; i <= floatLength; i++)
    {
      gmtOffset[i-1] = url_tail[i];
    }
    timeZone = atof(gmtOffset);
    setTimeZone();
    saveTimeZone();
    Serial.print("New timeZone = ");
    Serial.println(timeZone);
  }

  // priority toggle
  else if (url_tail[0] == 'w')
  {
    toggleWebServerPriority();
  }

  // delete root files
  else if (url_tail[0] == 'd')
  {
    sd.vwd()->getName(currentDirectory, 24);
    sd.chdir("/");
    sd.vwd()->rewind();

    while (myFile.openNext(sd.vwd(), O_READ))
    {
      myFile.printName(&Serial);
      char fileNameString[33];
      myFile.getName(fileNameString, 32);
      if (!myFile.isDir())
      {
        myFile.close();
        sd.remove(fileNameString);
      }
      else myFile.close();
    }
    sd.chdir(currentDirectory);
  }
}

void commandCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  if (type == WebServer::POST)
  {
    bool repeat;
    char name[16], value[33];
    do
    {
      /* readPOSTparam returns false when there are no more parameters
       * to read from the input.  We pass in buffers for it to store
       * the name and value strings along with the length of those
       * buffers. */
      repeat = server.readPOSTparam(name, 16, value, 33);

      if (repeat)
      {
        Serial.println("-=-=-=-=-");
        Serial.print("POST NAME: ");
        Serial.println(name);
        Serial.print("POST VALUE: ");
        Serial.println(value);
        /* this is a standard string comparison function.  It returns 0
         * when there's an exact match.  We're looking for a parameter
         * named "next" here. */
        if (strcasecmp(name, "next") == 0)
        {
          irCommand = 'N';
          server.httpNoContent();
        }
        else if (strcasecmp(name, "menu") == 0)
        {
          irCommand = 'M';
          server.httpNoContent();
        }
        else if (strcasecmp(name, "power") == 0)
        {
          irCommand = 'P';
          server.httpNoContent();
        }
        else if (strcasecmp(name, "brightness") == 0)
        {
          cloudBrightness(value);
        }
        // skip backward in nested movie
        else if (strcasecmp(name, "chapter-") == 0)
        {
          if (offsetSpeedY == 16)
          {
            chainIndex += -2;
            if (chainIndex < 0) chainIndex = 0;
            offsetY = imageHeight + 1;
          }
          server.httpNoContent();
        }
        // skip forward in nested movie
        else if (strcasecmp(name, "chapter+") == 0)
        {
          if (offsetSpeedY == 16)
          {
            offsetY = imageHeight + 1;
          }
          server.httpNoContent();
        }
        // play an animation
        else if (strcasecmp(name, "play") == 0)
        {
          char folderWithSlash[34];
          strcpy_P(folderWithSlash, PSTR("/"));
          strcat(folderWithSlash, value);

          Serial.println("-=-=-=-=-");
          Serial.print("PLAY FOLDER: ");
          Serial.println(folderWithSlash);
          if (sd.exists(folderWithSlash))
          {
            strcpy_P(nextFolder, value);
            if (displayMode == 0) irCommand = 'N';
            server.httpSuccess("application/json");
            server.println("{\"status\": \"success\", \"message\": \"Folder selected.\"}");
          }
          else
          {
            server.httpSuccess("application/json");
            server.println("{\"status\": \"error\", \"message\": \"Folder does not exist.\"}");
          }
        }

        // make folder
        else if (strcasecmp(name, "mkdir") == 0)
        {
          storeGalleryState();
          strcpy(uploadTargetFolder, value);

          // make new folder
          server.httpSuccess("application/json");
          if (!sd.exists(value))
          {
            if (sd.mkdir(value))
            {
              Serial.println("Directory created successfully!");
              server.println("{\"status\": \"success\", \"message\": \"Folder created.\"}");
            }
            else
            {
              Serial.println("Directory creation FAILED.");
              server.println("{\"status\": \"error\", \"message\": \"Failed to create folder.\"}");
            }
          }
          else
          {
            Serial.println("Directory already exists.");
            server.println("{\"status\": \"error\", \"message\": \"Error: Folder already exists.\"}");
          }

          resumeGalleryState();
        }

        // remove folder
        else if (strcasecmp(name, "rmdir") == 0)
        {
          storeGalleryState();

          // make new folder
          server.httpSuccess("application/json");
          if (sd.exists(value))
          {
            sd.chdir(value);
            sd.vwd()->rmRfStar();
            Serial.println("Directory destroyed.");
            server.println("{\"status\": \"success\", \"message\": \"Folder removed.\"}");
          }
          else
          {
            Serial.println("Directory does not exist.");
            server.println("{\"status\": \"error\", \"message\": \"Error: Folder does not exist.\"}");
          }

          resumeGalleryState();
        }

        // change time zone
        else if (strcasecmp(name, "timeZone") == 0)
        {
          timeZone = atof(value);
          setTimeZone();
          saveTimeZone();
          server.httpNoContent();
        }

        // stock chart
        else if (strcasecmp(name, "chart") == 0)
        {
          char *c_type;
          char *c_symbol;
          c_type = strtok(value, ",");
          c_symbol = strtok(NULL, " ");
          strcpy(chartSymbol, c_symbol);
          if (strcasecmp(c_type, "stock") == 0) chartStock = true;
          else chartStock = false;
          displayMode = 3;
          chartRefresh = secondCounter + 60;
          refreshChart();
          EEPROM.put(147, chartStock);
          EEPROM.put(148, chartSymbol);
          server.httpNoContent();
        }

        // retrieve current settings as json
          else if (strcasecmp(name, "getValues") == 0)
        {
          Serial.println("Sending current variable values.");
          server.httpSuccess("application/json");
          server.print("{\"nowPlaying\": \"");
          server.print("/");
          sd.vwd()->getName(currentDirectory, 24);
          if (currentDirectory[0] != '/') server.print(currentDirectory);
          server.print("\", \"brightness\": \"");
          server.print(brightness);
          server.print("\", \"playMode\": \"");
          server.print(playMode);
          server.print("\", \"cycleTime\": \"");
          server.print(cycleTimeSetting);
          server.print("\", \"displayMode\": \"");
          server.print(displayMode);
          server.print("\", \"clockFace\": \"");
          byte clockFace = EEPROM.read(5);
          server.print(clockFace);
          server.print("\", \"timeZone\": \"");
          if (timeZone >= -12 || timeZone <= 13)
          {
            server.print(timeZone);
          }
          else server.print('0');
          server.println("\"}");
        }
      }
    } while (repeat);

    // after procesing the POST data, tell the web browser to reload
    // the page using a GET method.
    // server.httpSeeOther(PREFIX);
    return;
  }
  else server.httpNoContent();
}

void indexCmd(WebServer &server, WebServer::ConnectionType type, char *, bool)
{

  /* this line sends the standard "we're all OK" headers back to the
     browser */
  server.httpSuccess();

  /* For a HEAD request, we stop after outputting headers. */
  if (type == WebServer::HEAD)
    return;

  else if (type == WebServer::GET)
  {
    // store current animation info
    storeGalleryState();

    sd.chdir("/00system/wifi");
    readHTML(server, "index.html");

    resumeGalleryState();
  }
}

void helpCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{

  storeGalleryState();
  strcpy_P(uploadTargetFolder, "/00system/wifi/"); // upload files to wifi for replacing index.html


  /* this line sends the standard "we're all OK" headers back to the
     browser */
  server.httpSuccess();

  /* if we're handling a GET or POST, we can output our data here.
     For a HEAD request, we just stop after outputting headers. */
  if (type == WebServer::HEAD)
    return;

  /* this defines some HTML text in read-only memory aka PROGMEM.
   * This is needed to avoid having the string copied to our limited
   * amount of RAM. */
  P(Page_start) = "<html><head>\n<title>Game Frame</title>\n";
  server.printP(Page_start);
  server.println("<script src='https://ajax.googleapis.com/ajax/libs/jquery/2.1.3/jquery.min.js'></script>\n");
  server.println("<link rel='icon' type='image/png' href='http://ledseq.com/gameFrameFavicon.png'>\n");

  server.println("</head><body>\n");
  server.println("<script>");
  server.print("function funky(cmd){ $.ajax('http://");
  server.print(WiFi.localIP());
  server.println("/' + cmd); }");
  server.println("</script>\n");

  server.println("<script>");
  server.println("function setSelectedIndex(s, valsearch)");
  server.println("{");
  server.println("for (i = 0; i< document.getElementById(s).options.length; i++)");
  server.println("{ ");
  server.println("if (document.getElementById(s).options[i].value == valsearch)");
  server.println("{");
  server.println("document.getElementById(s).options[i].selected = true;");
  server.println("break;");
  server.println("}");
  server.println("}");
  server.println("return;");
  server.println("}");
  server.println("</script>");

  server.println("<script>");
  server.println("$(window).keydown(function(e) {");
  server.println("switch (e.keyCode) {");
  server.println("case 37: // left arrow key");
  server.println("funky(\"menu\");");
  server.println("return;");
  server.println("case 39: // right arrow key");
  server.println("funky(\"next\");");
  server.println("return;");
  server.println("case 80: // p key");
  server.println("funky(\"power\");");
  server.println("return;");
  server.println("}");
  server.println("});");
  server.println("</script>\n");

  server.printP("<H2><p>Game Frame Online</p></H2>\n");

  server.println("<table><tr>");
  server.println("<td><form><input type='button' value='POWER' onclick='funky(\"set?power\")'></form></td>\n");
  server.println("<td><form><input type='button' value='MENU' onclick='funky(\"set?menu\")'></form></td>\n");
  server.println("<td><form><input type='button' value='NEXT' onclick='funky(\"set?next\")'></form></td>\n");
  server.println("<td><form><input type='button' value='DELETE ROOT FILES' onclick='funky(\"set?d\")'></form></td>\n");
  server.println("</tr></table>");

  server.println("<table><tr><td>Brightness</td><td>Playback</td><td>Frequency</td><td>Mode</td><td>Clock Face</td></tr>");
  server.println("<tr><td>");
  server.println("<form method='post'>");
  server.println("<select id='brightness' onchange='funky(\"set?b\" + this.value)'>");
  server.println("<option value='7'>7</option>");
  server.println("<option value='6'>6</option>");
  server.println("<option value='5'>5</option>");
  server.println("<option value='4'>4</option>");
  server.println("<option value='3'>3</option>");
  server.println("<option value='2'>2</option>");
  server.println("<option value='1'>1</option>");
  server.println("<option value='0'>0</option>");
  server.println("</select>");
  server.println("</form>");
  server.println("</td>");

  server.println("<script>");
  server.print("setSelectedIndex('brightness',"); server.print(brightness); server.println(");");
  server.println("</script>");

  server.println("<td>");
  server.println("<form method='post'>");
  server.println("<select id='playMode' onchange='funky(\"set?p\" + this.value)'>");
  server.println("<option value='0'>Sequential</option>");
  server.println("<option value='1'>Shuffle</option>");
  server.println("<option value='2'>Shuffle (No Animation)</option>");
  server.println("</select>");
  server.println("</form>");
  server.println("</td>");

  server.println("<script>");
  server.print("setSelectedIndex('playMode',"); server.print(playMode); server.println(");");
  server.println("</script>");

  server.println("<td>");
  server.println("<form method='post'>");
  server.println("<select id='cycleTime' onchange='funky(\"set?c\" + this.value)'>");
  server.println("<option value='1'>10 Seconds</option>");
  server.println("<option value='2'>30 Seconds</option>");
  server.println("<option value='3'>1 Minute</option>");
  server.println("<option value='4'>5 Minutes</option>");
  server.println("<option value='5'>15 Minutes</option>");
  server.println("<option value='6'>30 Minutes</option>");
  server.println("<option value='7'>1 Hour</option>");
  server.println("<option value='8'>Infinite</option>");
  server.println("</select>");
  server.println("</form>");
  server.println("</td>");

  server.println("<script>");
  server.print("setSelectedIndex('cycleTime',"); server.print(cycleTimeSetting); server.println(");");
  server.println("</script>");

  server.println("<td>");
  server.println("<form method='post'>");
  server.println("<select id='displayMode' onchange='funky(\"set?m\" + this.value)'>");
  server.println("<option value='0'>Gallery</option>");
  server.println("<option value='1'>Clock</option>");
  server.println("<option value='2'>Effects</option>");
  server.println("<option value='3'>Chart</option>");
  server.println("</select>");
  server.println("</form>");
  server.println("</td>");

  server.println("<script>");
  server.print("setSelectedIndex('displayMode',"); server.print(displayMode); server.println(");");
  server.println("</script>");

  server.println("<td>");
  server.println("<form method='post'>");
  server.println("<select id='clockFace' onchange='funky(\"set?f\" + this.value)'>");
  server.println("<option value='1'>Color</option>");
  server.println("<option value='2'>Muted</option>");
  server.println("<option value='3'>Shadow</option>");
  server.println("<option value='4'>Binary 1</option>");
  server.println("<option value='5'>Binary 2</option>");
  server.println("</select>");
  server.println("</form>");
  server.println("</td></tr></table>");

  server.println("<script>");
  server.print("setSelectedIndex('clockFace',"); server.print(EEPROM.read(5)); server.println(");");
  server.println("</script>");

  server.println("<p>");
  server.println("SSID: ");
  server.println(WiFi.SSID());
  server.println("<br>");
  server.println("Local IP: ");
  server.println(WiFi.localIP());
  server.println("<br>");
  server.println("Subnet Mask: ");
  server.println(WiFi.subnetMask());
  server.println("<br>");
  server.println("Gateway IP: ");
  server.println(WiFi.gatewayIP());
  server.println("<br>");
  server.println("---");
  server.println("<br>");
  server.println("Game Frame Firmware: ");
  server.println(firmwareVersion);
  server.println("<br>");
  server.println("deviceID: ");
  String myID = System.deviceID();
  server.println(myID);
  server.println("<br>");
  server.printlnf("System version: %s", System.version().c_str());
  server.println("<br>");
  server.print("free memory: ");
  uint32_t freemem = System.freeMemory();
  server.println(freemem);
  server.println("");

  // readHTML(server, "/00system/wifi/index.htm");

  server.println("<hr>");
  server.println("<p><input type='button' value='Restore Index.html' onclick='funky(\"set?index\")'></form> (Restore index.html file used for the main control panel.)\n");
  server.println("<hr>");
  server.println("<p>Replace index.html:</p>");
  server.println("<p><form method='post' enctype='multipart/form-data' id='uploader' action='/upload' method='POST'>");
  server.println("<input type='file' name='my_file[]' multiple>");
  server.println("<input type='submit' value='Upload'>");
  server.println("</form>");
  server.println("<hr>");
  server.println("<p><a href='/00system/wifi/manager.html'>Update Firmware</a>");

  // listFiles(server, 0, 1);

  resumeGalleryState();
}

void readHTML(WebServer &server, const String& htmlfile)
{
  Serial.print("Opening HTML file....");
  if (myFile.open(htmlfile, O_READ))
  {
    Serial.println("Success!");
    while(myFile.available()) {
      // support native code with hooks
      if (myFile.peek() == '|') // native code begin flag
      {
        myFile.read(); // consume the flag character
        // don't mistake || operator for hook
        if (myFile.peek() == '|')
        {
          myFile.read();
          server.print("||");
        }
        else // hook!
        {
          char hook[16];
          for (int i=0; i<15; i++)
          {
            if (myFile.peek() == '|') // native code end flag
            {
              myFile.read(); // consume the flag character
              hook[i] = '\0'; // add escape char to close the string
              break; // go to execute hook
            }
            hook[i] = myFile.read(); // read hook
          }
          // execute hook
          String command(hook);
          if (command == "ip")
          {
            server.print(WiFi.localIP());
          }
          else if (command == "nowPlaying")
          {
            server.print("/");
            if (currentDirectory[0] != '/') server.print(currentDirectory);
          }
          else if (command == "ssid")
          {
            server.print(WiFi.SSID());
          }
          else if (command == "subnet")
          {
            server.print(WiFi.subnetMask());
          }
          else if (command == "gateway")
          {
            server.print(WiFi.gatewayIP());
          }
          else if (command == "deviceID")
          {
//            server.print("220037001447343338312345"); // fake code for broadcast purposes
            String myID = System.deviceID();
            server.print(myID);
          }
          else if (command == "version")
          {
            String ver = System.version().c_str();
            server.print(ver);
          }
          else if (command == "freemem")
          {
            uint32_t freemem = System.freeMemory();
            server.print(freemem);
          }
          else if (command == "brightness")
          {
            server.print(brightness);
          }
          else if (command == "playMode")
          {
            server.print(playMode);
          }
          else if (command == "cycleTime")
          {
            server.print(cycleTimeSetting);
          }
          else if (command == "displayMode")
          {
            server.print(displayMode);
          }
          else if (command == "clockFace")
          {
            byte clockFace = EEPROM.read(5);
            server.print(clockFace);
          }
          else if (command == "listFiles")
          {
            listFiles(server, 0, 1);
          }
          else if (command == "timeZone")
          {
            if (timeZone >= -12 || timeZone <= 13)
            {
              server.print(timeZone);
            }
            else server.print('0');
          }
          else if (command == "firmware")
          {
            server.print(firmwareVersion);
          }
          else if (command == "chart")
          {
            server.print(chartSymbol);
          }
          else if (command == "stockcoinsymbol")
          {
            server.print(chartSymbol);
          }
          else if (command == "stockcoin")
          {
            if (chartStock) server.print("stock");
            else server.print("coin");
          }
          else
          {
            Serial.print("UNKNOWN: ");
            Serial.print(command);
          }
        }
      }
      // write HTML file to browser
      else server.write(myFile.read()); // send web page to client
    }
    Serial.println("Finished serving HTML file.");
    myFile.close();
  }
  else Serial.println("Failed!");
}

//=============================================================================================================================
// utilities for mime types, in order to provide correct file type info to web browser
char* mime_types=
  "HTM*text/html|"
  "TXT*text/plain|"
  "INI*text/plain|"
  "CSS*text/css|"
  "XML*text/xml|"
  "GIF*image/gif|"
  "JPG*image/jpeg|"
  "PNG*image/png|"
  "BMP*image/bmp|"
  "ICO*image/vnd.microsoft.icon|"
  "MP3*audio/mpeg|";

  static const uint16_t text_html_content_type = 4;  // index of standard HTM type.
// =============================================================================// =============================================================================

uint16_t get_mime_type_from_filename(const char* filename) {
  uint16_t r = text_html_content_type;
  if (!filename) {
    return r;
  }
  char* ext = strrchr(filename, '.');
  if (ext) {     // We found an extension. Skip past the '.'
    ext++;
    char ch;
    int i = 0;
    while (i < strlen(mime_types)) {
      // Compare the extension
      char* p = ext;
      ch = mime_types[i];
      while (*p && ch != '*' && toupper(*p) == ch) {
        p++; i++;
        ch = mime_types[i];
      }
      if (!*p && ch == '*') {   // We reached the end of the extension while checking
        r = ++i;   // equality with a MIME type: we have a match. Increment i
        break;      // to reach past the '*' char, and assign it to `mime_type'.
      } else {  // Skip past the the '|' character indicating the end of a MIME type.
        while (mime_types[i++] != '|');
      }
    }
  }
  //  free(ext);
  return r;
}

void failureCmd(WebServer &server, WebServer::ConnectionType type, char *url_tail, bool tail_complete)
{
  /* if we're handling a GET or POST, we can output our data here.
     For a HEAD request, we just stop after outputting headers.
     Also, we're only serving files here so url_tail is required. */
  if (type == WebServer::HEAD || strlen(url_tail) == 0)
    return;

  storeGalleryState();

  // this checks if the requested url is perhaps a file that needs to be served
  SdFile filetosend;
  char* filename=url_tail;  //
  Serial.print("requesting file: ");
  Serial.println(filename);
  if (!filetosend.open(filename, O_READ))
  {
    server.httpFail();
    server.println("<html><body><h1>Requested Page not found.</h1></body></html>");
    return;
  }
  else if (filetosend.isFile()) {  // if not, file is not found, corrupt or else.
    Serial.print("Serving file: ");
    Serial.println(filename);
    uint16_t mimetype=get_mime_type_from_filename(filename);
    //send_error_code(server,mimetype, 200);
    server.println("HTTP/1.0 200 OK");
    server.print("Content-Type: ");
    // next part writes the content type (mime types)
    char ch;
    int i = mimetype;
    while ((ch = mime_types[i++]) != '|') {
     server.print(ch);
    }
    server.println("\n");
    // finished writing mimetypes,
    // use HTML-specific script if necessary
    if (mimetype == 4)
    {
      readHTML(server, filename);
      filetosend.close();
      resumeGalleryState();
      return;
    }
    // not HTML; write the file
    int16_t n;
    uint8_t buf[1000];//
    while ((n = filetosend.read(buf, sizeof(buf))) > 0) {
         server.write(buf,n);
    }
    Serial.println("File sent.");
    filetosend.close();
  }
  else Serial.println("file is corrupt or not found.");

  resumeGalleryState();
}

void uploadCmd(WebServer & server, WebServer::ConnectionType type, char * url_tail, bool tail_complete)
{
  storeGalleryState();
  sd.vwd()->rewind();

  Serial.println("uploadCmd Called...");
  int numUploads = 0;
  long nbytes;
  boolean success = false;
  if (type == WebServer::POSTMULTI) // this is what is expected: a multipart form
  {
    Serial.println("POSTMULTI detected...");
    if (uploadTargetFolder[0] != '\0') sd.chdir(uploadTargetFolder);
    bool repeat;
    int act = -1;
    char ch;
    while ((act = server.readNextFormPart(act)) != -4)
    { // if readnextformpart returns -4, either an error occurred or end of forms was reached.
      numUploads++;
      Serial.print(numUploads);
      Serial.print(" : ");
      Serial.print("m_filename: ");
      Serial.println(server.m_filename);
      if (strlen(server.m_filename) > 0)
      { // filename is not empty: a file has been submitted.  filename is read in readnextformpart.
        int buffer[5]; // a simple FIFO buffer is used to make sure the last two bytes are not written to file.   A better way could(should) be used.  for now, this works
        int b_read = 0;
        int b_write = 0;
        int diff = 0; // buffer read/write indexes, and the number of bytes the read buffer is behind
        SdFile filetoload;
        filetoload.clearWriteError();
        if (filetoload.open(server.m_filename, O_CREAT | O_WRITE | O_TRUNC))
        { // returns true if file was opened succesfully
          nbytes = 0; // number of bytes read
          success = true;
          while ((act = server.readNextFormPart(-3)) >= 0)
          { // as long as a valid byte is read. (-3) as input means it is returning reading bytes  and exiting if a boundary was found.
            nbytes++;
            buffer[b_write] = act; // put data in buffer
            b_write = (b_write + 1) % 5; //updat buffer index
            diff++;
            if (diff > 2)
            { // start writing after  a 2-byte delay
              nbytes++;
              ch = buffer[b_read];
              filetoload.write(buffer[b_read]); //write data to files
              if (filetoload.getWriteError())
              {
                break;
              }
              b_read = (b_read + 1) % 5;
              diff--;
            }
          }
          // free(buffer);
          // this previous section needs some cleaning up.  writing to a much larger buffer and writing the buffer in a single step to the file would probably be much faster.
          if (filetoload.getWriteError())
          {
            success = false;
            Serial.println("write error!!");
          }
          Serial.println("Finished writing!");
          filetoload.sync();
          filetoload.close();
        }
        else
        {
          Serial.println("Failed to open file!!");
        }
      }
    }
  } // end if POSTMULTI
  server.httpSuccess("application/json");
  if (success)
  {
    server.print("{\"status\": \"success\", \"numUploads\": \"");
    server.print(numUploads);
    server.print("\", \"numBytes\": \"");
    server.print(nbytes);
    server.println("\"}");
  }
  else
  {
    server.println("{\"status\": \"error\", \"message\": \"Files failed to upload.\"}");
  }
  resumeGalleryState();
}

void WebServerLaunch()
{
  /* setup our default command that will be run when the user accesses
   * the root page on the server */
  webserver.setDefaultCommand(&indexCmd);

  /* run the same command if you try to load /index.html, a common
   * default page name */
   webserver.addCommand("help", &helpCmd);
   webserver.addCommand("command", &commandCmd);
   webserver.addCommand("play", &playCmd);
   webserver.addCommand("set", &setCmd);
   webserver.addCommand("upload", &uploadCmd);
   webserver.setFailureCommand(&failureCmd); // this servers files from the SD card, else error.

  /* start the webserver */
  webserver.begin();
}

void processWebServer()
{
  if (Particle.connected())
  {
    char buff[64];
    int len = 64;

    /* process incoming connections one at a time forever */
    webserver.processConnection(buff, &len);

  }
}

void storeGalleryState()
{
  // store current animation info
  Serial.println("Storing gallery state...");
  galleryStateStored = true;

  if (displayMode == 1) // clock mode
  {
    closeMyFile();
    clockAnimationActive = false;
    clockShown = false;
    abortImage = true;
    chainIndex = -1;
    nextFolder[0] = '\0';
  }

  if (chainIndex > -1)
  {
    char chainChar[6];
    strcpy(currentDirectory, chainRootFolder);
    strcat(currentDirectory, "/");
    itoa(chainIndex-1, chainChar, 10);
    strcat(currentDirectory, chainChar);
    chainIndexBuffer = chainIndex;
    chainIndex = -1;
  }
  else
  {
    sd.vwd()->getName(currentDirectory, 24);
  }
  resumeFile = false;
  currentFile[0] = '\0';
  currentFilePosition = 0; // stores current position when using web server
  if (myFile.isOpen())
  {
    resumeFile = true;
    myFile.getName(currentFile, 13); // store current file
    Serial.print("File is open. Saving position of: ");
    Serial.print(currentDirectory);
    Serial.print("/");
    Serial.println(currentFile);
    currentFilePosition = myFile.curPosition();
    closeMyFile();
  }
  sd.chdir("/");
  singleGraphicBuffer = singleGraphic;
  secondCounterBuffer = secondCounter;
  fileIndexBuffer = fileIndex;
  fileIndex = 0;
  offsetBufferX = offsetX;
  offsetBufferY = offsetY;
  offsetX = 0;
  offsetY = 0;
  imageWidthBuffer = imageWidth;
  imageHeightBuffer = imageHeight;
  if (framePowered && alertPhase == 0 && brightness > 0)
  {
    bmpDraw("/00system/wifi/wifi.bmp", 0, 0);
    fastledshow();
    closeMyFile();
  }
}

void resumeGalleryState()
{
  if (displayMode == 1)
  {
    Serial.println("Re-initializing Clock...");
    galleryStateStored = false;
    initClock();
    return;
  }
  else if (displayMode == 2)
  {
    Serial.println("Returning to FX...");
    galleryStateStored = false;
    chainIndex = chainIndexBuffer;
    singleGraphic = singleGraphicBuffer;
    secondCounter = secondCounterBuffer;
    fileIndex = fileIndexBuffer;
    offsetX = offsetBufferX;
    offsetY = offsetBufferY;
    imageWidth = imageWidthBuffer;
    imageHeight = imageHeightBuffer;
    clearStripBuffer();
    return;
  }
  else if (displayMode == 3)
  {
    Serial.println("Returning to Chart...");
    galleryStateStored = false;
    setCycleTime();
    secondCounter = secondCounterBuffer;
    offsetX = offsetBufferX;
    offsetY = offsetBufferY;
    imageWidth = imageWidthBuffer;
    imageHeight = imageHeightBuffer;
    drawChart(chartValues);
    return;
  }
  sd.chdir("/");
  Serial.print("Returning to: ");
  Serial.println(currentDirectory);
  if (!sd.chdir(currentDirectory))
  {
    Serial.println("Failed to chdir! Advancing to next image.");
    resumeFile = false;
    nextImage();
    return;
  }
  setCycleTime();
  chainIndex = chainIndexBuffer;
  singleGraphic = singleGraphicBuffer;
  secondCounter = secondCounterBuffer;
  fileIndex = fileIndexBuffer;
  offsetX = offsetBufferX;
  offsetY = offsetBufferY;
  imageWidth = imageWidthBuffer;
  imageHeight = imageHeightBuffer;
  if (framePowered)
  {
    if (chainIndex > -1)
    {
      char rootFolderConfig[20];
      strcpy(rootFolderConfig, "/");
      strcat(rootFolderConfig, chainRootFolder);
      strcat(rootFolderConfig, "/config.ini");
      if (sd.exists(rootFolderConfig))
      {
        sd.chdir("/");
        sd.chdir(chainRootFolder);
        Serial.print(F("Opening Root File: "));
        Serial.print(chainRootFolder);
        Serial.println(F("/config.ini"));
        readIniFile();
        sd.chdir("/");
        sd.chdir(currentDirectory);
      }
      else
      {
        Serial.print(F("Opening File: "));
        Serial.print(currentDirectory);
        Serial.println(F("/config.ini"));
        readIniFile();
      }
    }
    else
    {
      Serial.print(F("Opening File: "));
      Serial.print(currentDirectory);
      Serial.println(F("/config.ini"));
      readIniFile();
    }
    if (resumeFile)
    {
      Serial.print("Re-opening: ");
      if (currentDirectory[0] != '/') Serial.print("/");
      Serial.print(currentDirectory);
      if (currentDirectory[0] != '/') Serial.print("/");
      Serial.print(currentFile);
      Serial.print(" at ");
      Serial.println(currentFilePosition);
      myFile.open(currentFile, O_READ);
      myFile.seekSet(currentFilePosition);
    }
    swapTime = millis() + holdTime;
    drawFrame();
  }
  galleryStateStored = false;
}

void toggleWebServerPriority()
{
  webServerActive = !webServerActive;
  if (webServerActive) storeGalleryState();
  webServerDisplayManager();
  if (webServerActive) resumeGalleryState();
}

void webServerDisplayManager()
{
  if (webServerActive)
  {
    offsetBufferX = offsetX;
    offsetBufferY = offsetY;
    offsetX = 0;
    offsetY = 0;
    if (framePowered) bmpDraw("/00system/wifi/wifi.bmp", 0, 0);
    closeMyFile();
    /*
    // desaturate screen if animations paused
    for (int i = 0; i < 256; i++)
    {
      uint8_t luma = leds[i].getLuma();
      leds[i] = CRGB(luma, luma, luma);
    }
    */
    fastledshow();
  }
  // exit web priority
  else
  {
    offsetX = offsetBufferX;
    offsetY = offsetBufferY;
    if (displayMode == 1) initClock();
    else if (displayMode == 0) drawFrame();
  }
}


// scroll the local IP address across screen
void scrollAddress()
{
  clockShown = true; // allows us to control when FastLED.show() is called
  singleGraphic = true; // allows us to control when BMP file is closed
  IPAddress myAddr = WiFi.localIP();
  int x = 0;
  int scroll = 16; // cursor position starts off screen
  long scrollTick = 0;
  // each draw pass of the scroll
  while (x >= -3)
  {
    // allow the user to skip with NEXT button
    irReceiver();
    if (irCommand == 'N')
    {
      irCommand = 'Z';
      break;
    }
    if (millis() > scrollTick)
    {
      scrollTick = millis() + 75; // ip address scroll speed
      clearStripBuffer();
      int xDrawShift = 0;
      // each octet
      for (byte o=0; o<4; o++)
      {
        byte octet = myAddr[o];
        char a[4];
        itoa(octet, a, 10);
        byte numDigits;
        if (octet >= 100) numDigits=3;
        else if (octet >= 10) numDigits=2;
        else numDigits=1;
        // each character
        for (byte i=0; i<numDigits; i++)
        {
          offsetY = ((a[i] - '0') * 16); // read number from digits BMP
          x = xDrawShift + scroll; // write at the correct column
          bmpDraw("/00system/digits_2.bmp", x, 0);
          xDrawShift += 4; // shift cursor to next character
        }
        xDrawShift += 2; // leave space between octets
      }
      fastledshow();
      scroll--; // scroll left
    }
  }
  closeMyFile();
}

// Cloud

// One function to rule them all
int cloudCommand(String c)
{
  char cmd[512];
  strcpy(cmd, c);
  char *command;
  char *parameter;
  command = strtok(cmd, " ");
  command[0] = tolower(command[0]);
  parameter = strtok(NULL, " ");
  parameter[0] = tolower(parameter[0]);
  Serial.println("Cloud Command!");
  Serial.print("Command: ");
  Serial.println(command);
  Serial.print("Parameter: ");
  Serial.println(parameter);

  if (strcasecmp(command, "next") == 0)
  {
    cloudNext(parameter);
  }
  else if (strcasecmp(command, "brightness") == 0 || strcasecmp(command, "b") == 0)
  {
    cloudBrightness(parameter);
  }
  else if (strcasecmp(command, "power") == 0)
  {
    cloudPower(parameter);
  }
  else if (strcasecmp(command, "play") == 0)
  {
    cloudPlayFolder(parameter);
  }
  else if (strcasecmp(command, "alert") == 0)
  {
    cloudAlert(parameter);
  }
  else if (strcasecmp(command, "score") == 0)
  {
    cloudScore(parameter);
  }
  else if (strcasecmp(command, "color") == 0)
  {
    cloudColor(parameter);
  }
  else if (strcasecmp(command, "webserver") == 0)
  {
    Serial.println("Relaunching web server.");
    webserver.begin(); // test for relaunching web server after Wi-Fi dropout
  }
  // change clock face
  else if (strcasecmp(command, "clockface") == 0)
  {
    Serial.println("Changing clock face.");
    uint8_t face = atoi(parameter);
    setClockFace(face);
    if (clockShown && !enableSecondHand)
    {
      drawDigits();
      fastledshow();
    }
  }
  // pong clock
  else if (strcasecmp(command, "pongclock") == 0)
  {
    cloudPongClock(parameter);
  }
  // change time zone
  else if (strcasecmp(command, "timezone") == 0)
  {
    Serial.println("Changing timezone.");
    timeZone = atof(parameter);
    setTimeZone();
    saveTimeZone();
  }
  // plasma
  else if (strcasecmp(command, "plasma") == 0)
  {
    Serial.println("Plasma.");
    displayMode = 2;
  }
  // change playback mode
  else if (strcasecmp(command, "playback") == 0)
  {
    Serial.println("Changing playback mode.");
    uint8_t newPlayback = playMode;

    if (isDigit(parameter[0])) newPlayback = atoi(parameter);

    // accept "on" or "off" text for Alexa compatibility. Controls shuffle mode.
    else
    {
      if (strcasecmp(parameter, "on") == 0) newPlayback = 1;
      else if (strcasecmp(parameter, "off") == 0) newPlayback = 0;
    }

    if (newPlayback < 3)
    {
      playMode = newPlayback;
      saveSettingsToEEPROM();
    }
  }
  // change display mode
  else if (strcasecmp(command, "display") == 0)
  {
    Serial.println("Display mode change requested.");
    uint8_t oldMode = displayMode;
    int16_t newMode = displayMode;

    // explicit display mode number provided
    if (isDigit(parameter[0])) newMode = atoi(parameter);

    // text received, must convert
    else if (strcasecmp(parameter, "gallery") == 0
      || strcasecmp(parameter, "graphics") == 0
      || strcasecmp(parameter, "pictures") == 0
      || strcasecmp(parameter, "animation") == 0
      || strcasecmp(parameter, "animations") ==0)
    {
      newMode = 0;
    }
    else if (strcasecmp(parameter, "clock") == 0 || strcasecmp(parameter, "time") == 0)
    {
      newMode = 1;
    }
    else if (strcasecmp(parameter, "effect") == 0 || strcasecmp(parameter, "effects") == 0)
    {
      newMode = 2;
    }

    // support incrementing and decrementing
    else if (parameter[0] == '+')
    {
      newMode = displayMode + 1;
      if (newMode > displayModeMax) newMode = 0;
    }
    else if (parameter[0] == '-')
    {
      newMode = displayMode - 1;
      if (newMode < 0) newMode = displayModeMax;
    }

    if (newMode <= displayModeMax && newMode != oldMode)
    {
      displayMode = newMode;
      saveSettingsToEEPROM();
      if (displayMode == 1)
      {
        initClock();
      }
      else if (displayMode == 3)
      {
        drawChart(chartValues);
        refreshChart();
      }
      else
      {
        if (clockShown || clockAnimationActive)
        {
          closeMyFile();
          clockAnimationActive = false;
          clockShown = false;
          abortImage = true;
          nextFolder[0] = '\0';
        }
      }
    }
  }
  // change cycle time
  else if (strcasecmp(command, "cycle") == 0)
  {
    Serial.println("Changing cycle interval.");
    uint8_t newSetting;

    if (isDigit(parameter[0])) newSetting = atoi(parameter);

    // alexa compatibility
    else
    {
      if (strcasecmp(parameter, "PT10S") == 0) newSetting = 1;
      else if (strcasecmp(parameter, "PT30S") == 0) newSetting = 2;
      else if (strcasecmp(parameter, "PT1M") == 0) newSetting = 3;
      else if (strcasecmp(parameter, "PT5M") == 0) newSetting = 4;
      else if (strcasecmp(parameter, "PT15M") == 0) newSetting = 5;
      else if (strcasecmp(parameter, "PT30M") == 0) newSetting = 6;
      else if (strcasecmp(parameter, "PT1H") == 0) newSetting = 7;
      // workaround for AMAZON.DURATION not supporting infinity (use 1 year instead)
      else if (strcasecmp(parameter, "P1Y") == 0) newSetting = 8;
      else if (strcasecmp(parameter, "infinity") == 0) newSetting = 8;
    }

    if (newSetting <= 8 && newSetting > 0)
    {
      cycleTimeSetting = newSetting;
      setCycleTime();
      saveSettingsToEEPROM();
    }
  }
  // auto cycle display mode
  else if (strcasecmp(command, "cycleModes") == 0)
  {
    if (isDigit(parameter[0]))
    {
      long l = atol(parameter);
      if (l > 0)
      {
        displayModeCycleLength = l;
        displayModeCycle = true;
      }
      else displayModeCycle = false;
    }
    else displayModeCycle = false;
  }
  // reboot
  else if (strcasecmp(command, "reboot") == 0)
  {
    Serial.println("Rebooting.");
    systemRecover();
  }
  // choose effect
  else if (strcasecmp(command, "effect") == 0)
  {
    Serial.println("Changing effect.");
    int requestedEffect = atoi(parameter);
    // valid effect?
    if (requestedEffect >= 0 && requestedEffect < numEffects)
    {
      currentEffect = requestedEffect;
      if (displayMode == 2)
      {
        clearStripBuffer();
        BasicVariablesSetup();
        secondCounter = 0;
        baseTime = millis();
      }
    }
  }

  // stock chart
  // example: Command stock AAPL
  else if (strcasecmp(command, "stock") == 0)
  {
    chartStock = true;
    displayMode = 3;
    chartRefresh = secondCounter + 60;
    strcpy(chartSymbol, parameter);
    EEPROM.put(147, chartStock);
    EEPROM.put(148, chartSymbol);
    refreshChart();
  }

  // crypto chart
  // example: Command coin BTC
  else if (strcasecmp(command, "coin") == 0)
  {
    chartStock = false;
    displayMode = 3;
    chartRefresh = secondCounter + 60;
    strcpy(chartSymbol, parameter);
    EEPROM.put(147, chartStock);
    EEPROM.put(148, chartSymbol);
    refreshChart();
  }

  // generic chart
  // expects a 17-float array
  // array should be in reverse chronological order
  // i.e. first entry will render on the right-most column
  // example: Command chart 1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17
  else if (strcasecmp(command, "chart") == 0)
  {
    chartStock = false;
    displayMode = 3;
    chartSymbol[0] = '\0';
    chartValues[0]=atof(strtok(parameter, ","));
    for (int i=1; i<17; i++)
    {
      chartValues[i]=atof(strtok(NULL, ","));
    }
    drawChart(chartValues);
  }
  return 0;
}

int cloudNext(String c)
{
  Serial.println("Next Requested.");
  if (framePowered && brightness > 0)
  {
    if (displayMode == 0)
    {
      // exit chaining if necessary
      if (chainIndex > -1)
      {
        chainIndex = -1;
        chainRootFolder[0] = '\0';
        sd.chdir("/");
      }
      nextImage();
      drawFrame();
    }
  }
}

int cloudBrightness(String c)
{
  Serial.print("Brightness Requested: ");
  Serial.println(c);
  if (framePowered)
  {
    if (strchr(c, '%'))
    {
      Serial.println("Percentage detected.");
      char percentageString[5];
      c.toCharArray(percentageString, 5);
      int percentage = atoi(strtok(percentageString, "%%"));
      int fullBright = 0;

      if (APANativeBrightness) fullBright = 255;
      else fullBright = brightnessMultiplier * 7;

      float multiplier = percentage / 100.0f;
      float newBright = fullBright * multiplier;
      if (APANativeBrightness && newBright < 4) newBright = 4;
      if (newBright >= 0 && newBright <= fullBright) FastLED.setBrightness(newBright);

      if (APANativeBrightness)
      {
        newBright = 7.0f * multiplier;
        brightness = round(newBright);
        if (brightness < 1) brightness = 1;
        ledController.setAPA102Brightness(brightness);
      }
    }
    // Do NOT use this.
    /*else if (strchr(c, '!'))
    {
      Serial.println("Absolute detected.");
      char absoluteString[5];
      c.toCharArray(absoluteString, 5);
      uint8_t newBright = atoi(strtok(absoluteString, "!"));
      if (newBright < 32 && APANativeBrightness)
      {
        FastLED.setBrightness(255);
        ledController.setAPA102Brightness(newBright);
      }
      else if (!APANativeBrightness)
      {
        FastLED.setBrightness(newBright);
      }
    }*/
    else
    {
      brightness = atoi(c);
      stripSetBrightness();
      saveSettingsToEEPROM();
    }
    if (displayMode == 3) drawChart(chartValues);
    else fastledshow();
  }
  return -1;
}

int cloudPower(String c)
{
  if (c == "on")
  {
    Serial.println("Power On Requested.");
    if (brightness == 0)
    {
      brightness = 1;
      stripSetBrightness();
      if (displayMode == 0) drawFrame(); // refrech the screen
      else initClock();
    }
    else if (!framePowered)
    {
      EEPROM.update(201, 255); // store power state
      initGameFrame();
      framePowered = true;
    }
  }
  else if (c == "off")
  {
    Serial.println("Power Off Requested.");
    if (framePowered)
    {
      framePowerDown();
      framePowered = false;
    }
  }
  else
  {
    Serial.println("Power Toggle Requested.");
    irCommand = 'P';
    powerControl();
  }
}

int cloudPongClock(String c)
{
  if (c == "on")
  {
    Serial.println("PongClock Requested.");

    if (displayMode != 1)
    {
      displayMode = 1;
      initClock();
    }
    pong_reset = true;
    pongclock = true;
    pong_showtime = 0;
  }
  else if (c == "off")
  {
    Serial.println("PongClock Off Requested.");
    pongclock = false;
  }
  else
  {
    pongclock = !pongclock;
    if (pongclock)
    {
      if (displayMode != 1)
      {
        displayMode = 1;
        initClock();
      }
      pong_reset = true;
      pongclock = true;
      pong_showtime = 0;
    }
  }
}

int cloudPlayFolder(String c)
{
  Serial.print("Play requested: ");
  Serial.println(c);
  if (framePowered && brightness > 0)
  {
    if (displayMode == 0)
    {
      // verify chaining folders disabled
      chainIndex = -1;
      strcpy_P(nextFolder, c);
      nextImage();
      drawFrame();
    }
    else if (displayMode == 1)
    {
      // verify chaining folders disabled
      chainIndex = -1;
      strcpy_P(nextFolder, c);
    }
  }
  return 0;
}

int cloudAlert(String c)
{
  Serial.print("Alert! Folder requested: ");
  Serial.println(c);
  if (framePowered && brightness > 0 && alertPhase != 1)
  {
    if (c[0] == '/') strcpy(nextFolder, c);
    else
    {
      strcpy(nextFolder, "/");
      strcat(nextFolder, c);
    }

    // flash
    for (int i=0; i<3; i++)
    {
      // color
      for (int i = 0; i < 256; i++)
      {
        leds[i] = CRGB(255, 255, 0);
      }
      fastledshow();
      delay(50);
      clearStripBuffer();
      fastledshow();
      delay(75);
    }

    delay(100);

    if (sd.exists(nextFolder))
    {
      if (displayMode == 0 || displayMode == 2 || displayMode == 3)
      {
        // only store gallery state once in case of multiple alerts
        if (alertPhase == 0 || alertPhase == 2)
        {
          alertPhase = 1;
          storeGalleryState();
        }
        nextImage();
        drawFrame();
      }
      else if (displayMode == 1)
      {
        alertPhase = 1;
        offsetX = 0;
        offsetY = 0;
        secondCounter = 0;
        currentSecond = Time.second();
        clockAnimationActive = true;
        clockShown = false;
        closeMyFile();
        abortImage = true;
      }
    }
    else
    {
      Serial.println("Alert folder not found.");
      nextFolder[0] = '\0';
    }
  }
  else
  {
    if (alertPhase > 0) Serial.println("Sorry, an alert is already in progress.");
    else Serial.println("Screen is off. Ignoring request.");
    return alertPhase;
  }
  return 0;
}

int cloudColor(String c)
{
  Serial.print("Color requested: ");
  Serial.println(c);
  if (framePowered && brightness > 0)
  {
    // verify chaining folders disabled
    chainIndex = -1;
    byte r, g, b;

    if (isDigit(c[0]) || c[0] == '#')
    {
      // Get rid of '#' and convert it to integer
      int number = 0;
      if (c[0] == '#') number = (int) strtol( &c[1], NULL, 16);
      else number = (int) strtol( &c[0], NULL, 16);

      // Split them up into r, g, b values
      r = number >> 16;
      g = number >> 8 & 0xFF;
      b = number & 0xFF;
    }

    // random color
    else if (c.toLowerCase() == "random")
    {
      r = random8();
      g = random8();
      b = random8();
    }

    // off; eventually want to make this return to previous animation
    else if (c.toLowerCase() == "off")
    {
      r = 0;
      g = 0;
      b = 0;
    }

    // convert color text to RGB values
    else colorNameToRGB(c, &r, &g, &b);

    if (displayMode == 0)
    {
      baseTime = millis();
    }
    else if (displayMode == 1)
    {
      currentSecond = Time.second();
      clockAnimationActive = true;
  //    clockShown = false;
  //    closeMyFile();
  //    abortImage = true;
    }
    holdTime = -1;
    secondCounter = 0;

    // apply contrast
    r = dim8_jer(r);
    g = dim8_jer(g);
    b = dim8_jer(b);

    // rainbow unicorns exist
    if (c.toLowerCase() == "rainbow")
    {
      uint8_t rainbowPixel = 0;
      uint8_t rainbowHue = 0;
      // each row
      for (uint8_t r=0; r<16; r++)
      {
        // each column
        for (uint8_t c=0; c<16; c++)
        {
          leds[rainbowPixel++] = CHSV(rainbowHue, 255, 255);
        }
        rainbowHue += 16;
      }
      fastledshow();
    }

    else
    {
      // send the color data
      for (int i = 0; i < 256; i++)
      {
        leds[i] = CRGB(r, g, b);
      }
      fastledshow();
    }
  }
  return 0;
}

int cloudScore(String c)
{
  Serial.print("Score requested: ");
  Serial.println(c);
  if (framePowered && brightness > 0)
  {
    // verify chaining folders disabled
    chainIndex = -1;
    scoreBoard = true;
    char scoreBuf[32];

    byte r0 = 0;
    byte g0 = 0;
    byte b0 = 0;
    byte r1 = 0;
    byte g1 = 0;
    byte b1 = 0;
    int c0 = 0;
    int c1 = 0;

    strcpy(scoreString, c);
    strcpy(scoreBuf, scoreString);
    uint8_t s0 = atoi(strtok(scoreBuf, "- "));
    uint8_t s1 = atoi(strtok(NULL, "-# "));
    char *color0 = strtok(NULL, "-# ");
    char *color1 = strtok(NULL, "-# ");
    Serial.println(color0);
    Serial.println(color1);
    if (strlen(color0) > 0)
    {
      // Get rid of '#' and convert color to integer
      if (color0[0] == '#') c0 = (int) strtol( &color0[1], NULL, 16);
      else c0 = (int) strtol( &color0[0], NULL, 16);
      if (color1[0] == '#') c1 = (int) strtol( &color1[1], NULL, 16);
      else c1 = (int) strtol( &color1[0], NULL, 16);
      r0 = c0 >> 16;
      g0 = c0 >> 8 & 0xFF;
      b0 = c0 & 0xFF;
      r1 = c1 >> 16;
      g1 = c1 >> 8 & 0xFF;
      b1 = c1 & 0xFF;
    }
    else
    {
      // default colors
      b0 = 255;
      r1 = 255;
    }

    clockShown = true; // hack for score flashing screen

    // score 0
    char numChar[3];
    itoa(s0, numChar, 10);
    byte singleDigit = numChar[0] - '0';
    offsetX = -1;
    if (s0 >= 10)
    {
      offsetY = singleDigit * 16;
    }
    else offsetY = 160;
    bmpDraw(clockFace, 0, 0);
    // second digit
    if (s0 >= 10)
    {
      singleDigit = numChar[1] - '0';
    }
    else singleDigit = numChar[0] - '0';
    offsetX = 0;
    offsetY = singleDigit * 16;
    bmpDraw(clockFace, 3, 0);

    // score 1
    numChar[0] = '\0';
    numChar[1] = '\0';
    numChar[2] = '\0';
    itoa(s1, numChar, 10);
    singleDigit = numChar[0] - '0';
    if (s1 >= 10)
    {
      offsetY = singleDigit * 16;
    }
    else offsetY = 160;
    bmpDraw(clockFace, 8, 0);
    // second digit
    if (s1 >= 10)
    {
      singleDigit = numChar[1] - '0';
    }
    else singleDigit = numChar[0] - '0';
    offsetY = singleDigit * 16;
    bmpDraw(clockFace, 12, 0);

    closeMyFile();
    clockShown = false; // turn off hack

    // colorize
    // left
    for (int x=0; x<7; x++)
    {
      for (int y=0; y<16; y++)
      {
        if (leds[getIndex(x, y)] != CRGB(0,0,0)) leds[getIndex(x, y)] = CRGB(r0, g0, b0);
      }
    }
    // right
    for (int x=7; x<16; x++)
    {
      for (int y=0; y<16; y++)
      {
        if (leds[getIndex(x, y)] != CRGB(0,0,0)) leds[getIndex(x, y)] = CRGB(r1, g1, b1);
      }
    }
    FastLED.show();

    if (displayMode == 0)
    {
      baseTime = millis();
    }
    else if (displayMode == 1)
    {
      currentSecond = Time.second();
      clockAnimationActive = true;
  //    clockShown = false;
  //    closeMyFile();
  //    abortImage = true;
    }
    holdTime = -1;
    secondCounter = 0;
  }
  return 0;
}

void showCoindeskBTC()
{
  Serial.println("#HODL");
  float bitcoin[16];

  // historical data
  // Request path and body can be set at runtime or at setup.
  request.hostname = "api.coindesk.com";
  request.port = 80;
  request.path = "/v1/bpi/historical/close.json";

  // Get request
  http.get(request, response, headers);

  char json[2048];
  String jsonString = String(response.body);
  strcpy(json, jsonString);
  strtok(json, ":,}");
  // skip to the last 15 weeks
  for (int i=0; i<33; i++)
  {
    strtok(NULL, ":,}");
  }
  for (int i=0; i<15; i++)
  {
    bitcoin[i]=atoi(strtok(NULL, ":,}"));
    strtok(NULL, ":,}");
  }

  // current price
  request.path = "/v1/bpi/currentprice/USD.json";

  // Get request
  http.get(request, response, headers);

  jsonString = String(response.body);
  strcpy(json, jsonString);

  strtok(json, ":}");
  // skip to the goods
  for (int i=0; i<17; i++)
  {
    strtok(NULL, ":}");
  }
  bitcoin[15]=atoi(strtok(NULL, ":}"));

  /*// check your answers
  Serial.println("---");
  for (int i=0; i<16; i++)
  {
    Serial.print(bitcoin[i]);
    Serial.print(", ");
  }
  Serial.println("---");*/

  drawChart(bitcoin);
}

void getChart(char chartFunction[], char chartSymbol[], char chartInterval[])
{
  Serial.print("Get Chart: ");
  Serial.print(chartFunction);
  Serial.print(", ");
  Serial.print(chartSymbol);
  Serial.print(", ");
  Serial.println(chartInterval);
  chartValues[0] = '\0';

  chartSymbol = strupr(chartSymbol);

  // Request path and body can be set at runtime or at setup.
  request.hostname = "ledseq.com";
  request.port = 80;
  // assemble path
  char requestPath[120];
  strcpy(requestPath, "/gf-stocks.php?chartFunction=");
  strcat(requestPath, chartFunction);
  strcat(requestPath, "&chartSymbol=");
  strcat(requestPath, chartSymbol);
  strcat(requestPath, "&chartInterval=");
  strcat(requestPath, chartInterval);
  request.path = requestPath;

  // Get request
  http.get(request, response, headers);

  char json[2048];
  String jsonString = String(response.body);
  strcpy(json, jsonString);

  // make backup
  String dailyStringBackup = jsonString;

  // error detection
  if (jsonString.startsWith("ERROR"))
  {
    flashBox(255, 0, 0);
    Serial.println(response.body);
  }

  // intraday
  else if (strcmp(chartFunction, "TIME_SERIES_INTRADAY") == 0 || strcmp(chartFunction, "DIGITAL_CURRENCY_INTRADAY") == 0)
  {
    Serial.println("Intraday Chart");
    chartValues[0]=atof(strtok(json, ","));
    for (int i=1; i<17; i++)
    {
      chartValues[i]=atof(strtok(NULL, ","));
    }
  }

  // daily
  else
  {
    Serial.println("Daily Chart");
    Serial.println(requestPath);
    Serial.println(response.body);
    chartValues[1]=atof(strtok(json, ","));
    for (int i=2; i<17; i++)
    {
      chartValues[i]=atof(strtok(NULL, ","));
    }

    Serial.print("Append Latest?...");
    // stock daily append current
    if (strcmp(chartFunction, "TIME_SERIES_DAILY") == 0)
    {
      Serial.println("Stock Daily");
      strcpy(requestPath, "/gf-stocks.php?chartFunction=TIME_SERIES_INTRADAY&chartInterval=1&chartSymbol=");
      strcat(requestPath, chartSymbol);
      request.path = requestPath;

      // Get request
      http.get(request, response, headers);

      Serial.println(requestPath);
      Serial.println(response.body);

      char jsonIntraday[2048];
      jsonString = String(response.body);
      strcpy(jsonIntraday, jsonString);

      chartValues[0]=atof(strtok(jsonIntraday, ","));

      // trading day over?
      if (chartValues[0] == chartValues[1])
      {
        // latest value already reflected in Daily data
        Serial.println("Trading day over.");
        strcpy(json, dailyStringBackup);
        chartValues[0]=atof(strtok(json, ","));
        for (int i=1; i<17; i++)
        {
          chartValues[i]=atof(strtok(NULL, ","));
        }
      }
    }

    // crypto daily append current
    else if (strcmp(chartFunction, "DIGITAL_CURRENCY_DAILY") == 0)
    {
      Serial.println("Crypto Daily");
      strcpy(requestPath, "/gf-stocks.php?chartFunction=DIGITAL_CURRENCY_PRICE&chartSymbol=");
      strcat(requestPath, chartSymbol);
      request.path = requestPath;

      // Get request
      http.get(request, response, headers);

      char jsonIntraday[2048];
      jsonString = String(response.body);
      strcpy(jsonIntraday, jsonString);

      chartValues[0]=atof(jsonIntraday);
    }
  }

  // check your answers
  Serial.println("---");
  for (int i=0; i<17; i++)
  {
    Serial.print(chartValues[i]);
    Serial.print(", ");
  }
  Serial.println("");
  Serial.println("---");

  drawChart(chartValues);
}

void fill_with_gradient(CRGB top, CRGB bottom)
{
  int ledIndex = 0;
  for (int row=0; row<16; row++)
  {
    uint8_t amountOfBlend = map(row, 0, 16, 0, 255);
    CRGB rowColor = blend(top, bottom, amountOfBlend);
    for (int col=0; col<16; col++)
    {
      leds[ledIndex] = rowColor;
      ledIndex++;
    }
  }
}

void drawChart(float arry[])
{
  // expects a 17-float array
  // array should be in reverse chronological order
  // i.e. first entry will render on the right-most column

  float arrayMin = 2147483647;
  float arrayMax = 0;

  for (int i=0; i<16; i++)
  {
    arrayMin = min(arrayMin, arry[i]);
  }
  for (int i=0; i<16; i++)
  {
    arrayMax = max(arrayMax, arry[i]);
  }

  clearStripBuffer();
  int arraySize = 0;
  arraySize = sizeof(chartValues)/sizeof(chartValues[0]);
  Serial.println(arraySize);
  for (int i=0; i<arraySize; i++)
  {
    Serial.println(chartValues[i]);
  }

  // area render
  if (chartStyle == 0)
  {
    // just use the first 16 entries
    for (int i=0; i<16; i++)
    {
      // map $USD to 0-15 (screen height)
      uint8_t y = map(arry[i], arrayMin, arrayMax, 0.0f, 15.0f);
      // color the peaks
      leds[getIndex(15-i, 15-y)] = CRGB(255, 215, 0);
      // fill beneath
      for (int fill=y-1; fill>=0; --fill)
      {
        leds[getIndex(15-i, 15-fill)] = CRGB(32, 48, 32);
      }
    }
    fastledshow();
  }

  // candle stick render (no wicks!)
  else if (chartStyle == 1)
  {
    if (APANativeBrightness) fill_with_gradient(CRGB(0,0,8), CRGB(0,0,64));
    else fill_with_gradient(CRGB(0,0,32), CRGB(0,0,64));
    // 17th entry is used to color first candlestick
    for (int i=0; i<16; i++)
    {
      // map $USD to 0-15 (screen height)
      uint8_t y = map(arry[i], arrayMin, arrayMax, 0.0f, 15.0f);
      uint8_t pY = map(arry[i+1], arrayMin, arrayMax, 0.0f, 15.0f); // previous column

      // bull
      if (arry[i] >= arry[i+1])
      {
        for (int candle = pY; candle <= y; candle++)
        {
          leds[getIndex(15-i, 15-candle)] = CRGB(0, 255, 0);
          // white dot
          if (candle==y && i==0) leds[getIndex(15, 15-y)] = CRGB(128, 255, 128);
        }
      }
      // bear
      else
      {
        for (int candle = pY; candle >= y; candle--)
        {
          // avoid draw errors for first column
          if (15-candle >= 0) leds[getIndex(15-i, 15-candle)] = CRGB(255, 0, 0);
          // white dot
          if (candle==y && i==0) leds[getIndex(15, 15-y)] = CRGB(255, 128, 128);
        }
      }
    }
    fastledshow();
  }
}

void refreshChart()
{
  if (chartSymbol[0] != '\0')
  {
    char chartFunction[26];
    // daily or intraday chart?
    if (cycleTimeSetting == 8)
    {
      if (chartStock) strcpy(chartFunction, "TIME_SERIES_DAILY");
      else strcpy(chartFunction, "DIGITAL_CURRENCY_DAILY");
    }
    else
    {
      if (chartStock) strcpy(chartFunction, "TIME_SERIES_INTRADAY");
      else strcpy(chartFunction, "DIGITAL_CURRENCY_INTRADAY"); // DIGITAL_CURRENCY_INTRADAY
    }

    char timeInterval[3];
    if (cycleTimeSetting <= 3) strcpy(timeInterval, "1");
    else if (cycleTimeSetting == 4) strcpy(timeInterval, "5");
    else if (cycleTimeSetting == 5) strcpy(timeInterval, "15");
    else if (cycleTimeSetting == 6) strcpy(timeInterval, "30");
    else strcpy(timeInterval, "60");
    getChart(chartFunction, chartSymbol, timeInterval);
  }
}

void refreshChartLatest()
{
  if (chartSymbol[0] != '\0')
  {
    // Request path and body can be set at runtime or at setup.
    char requestPath[120];
    String jsonString;
    Serial.print("Latest ");
    Serial.print(chartSymbol);
    Serial.print(" Price: ");

    // stock
    if (chartStock)
    {
      strcpy(requestPath, "/gf-stocks.php?chartFunction=TIME_SERIES_INTRADAY&chartInterval=1&chartSymbol=");
      strcat(requestPath, chartSymbol);
      request.path = requestPath;

      // Get request
      http.get(request, response, headers);

      char jsonIntraday[2048];
      jsonString = String(response.body);
      strcpy(jsonIntraday, jsonString);

      chartValues[0]=atof(strtok(jsonIntraday, ","));
    }

    // crypto
    else
    {
      strcpy(requestPath, "/gf-stocks.php?chartFunction=DIGITAL_CURRENCY_PRICE&chartSymbol=");
      strcat(requestPath, chartSymbol);
      request.path = requestPath;

      // Get request
      http.get(request, response, headers);

      char jsonIntraday[2048];
      jsonString = String(response.body);
      strcpy(jsonIntraday, jsonString);

      chartValues[0]=atof(jsonIntraday);
    }

    Serial.print(chartValues[0]);
    Serial.print(" ");
    Serial.println(Time.timeStr());
  }
}

void fastledshow()
{
  if (fadeStartTime + fadeLength > millis())
  {
    memmove( leds_buf, leds, NUM_LEDS * sizeof( CRGB) );
    uint8_t fadeAmount = map(millis(), fadeStartTime, fadeStartTime + fadeLength, 255, 0);
    for (int i = 0; i < NUM_LEDS; i++)
    {
      leds[i].fadeToBlackBy(fadeAmount);
    }
  }
  FastLED.show();
}

void fadeControl()
{
  if (fadeStartTime + fadeLength > millis())
  {
    memmove( leds, leds_buf, NUM_LEDS * sizeof( CRGB) );
    uint8_t fadeAmount = map(millis(), fadeStartTime, fadeStartTime + fadeLength, 255, 0);
    for (int i = 0; i < NUM_LEDS; i++)
    {
      leds[i].fadeToBlackBy(fadeAmount);
    }
    FastLED.show();
  }
}

// UDP
void checkUDP()
{
  if (Udp.parsePacket() > 0) {

    char sBuffer[64];
    int sBufferIndex = 0;
    sBuffer[0] = '\0';

    // store in buffer
    while (sBufferIndex < 63 && Udp.available())
    {
      sBuffer[sBufferIndex] = Udp.read();
      sBufferIndex++;
    }
    sBuffer[sBufferIndex] = '\0';
    Serial.print("UDP: ");
    Serial.println(sBuffer);

    cloudCommand(sBuffer);

    // Store sender ip and port
    ipAddress = Udp.remoteIP();
    port = Udp.remotePort();

    if (port > -1)
    {
      // Echo back data to sender
      Udp.beginPacket(ipAddress, port);
      Udp.write("ok: ");
      Udp.write(sBuffer);
      Udp.endPacket();
    }
  }
}
