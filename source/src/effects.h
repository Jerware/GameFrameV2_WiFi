// Gane Frane Progromatic Effects

// prototypes
byte getIndex(byte x, byte y);
void runEffects();
void nextEffect();
void plasmaPrime();
void plasmaSlow();
void randomizePlasma();
extern CRGB leds[256];
extern int currentEffect;
extern int secondCounter;
extern unsigned long baseTime;

uint16_t maxEffect = 1; // number of effects - 1

void runEffects()
{
  if (currentEffect == 0) plasmaPrime();
  else plasmaSlow();
}

void nextEffect()
{
  currentEffect++;
  if (currentEffect > maxEffect) currentEffect = 0;
  secondCounter = 0;
  baseTime = millis();
}

// effect 1
// plasma by Edmund "Skorn" Horn
long frameCount = 256000;

uint8_t const cos_wave[256] PROGMEM =
{0,0,0,0,1,1,1,2,2,3,4,5,6,6,8,9,10,11,12,14,15,17,18,20,22,23,25,27,29,31,33,35,38,40,42,
45,47,49,52,54,57,60,62,65,68,71,73,76,79,82,85,88,91,94,97,100,103,106,109,113,116,119,
122,125,128,131,135,138,141,144,147,150,153,156,159,162,165,168,171,174,177,180,183,186,
189,191,194,197,199,202,204,207,209,212,214,216,218,221,223,225,227,229,231,232,234,236,
238,239,241,242,243,245,246,247,248,249,250,251,252,252,253,253,254,254,255,255,255,255,
255,255,255,255,254,254,253,253,252,252,251,250,249,248,247,246,245,243,242,241,239,238,
236,234,232,231,229,227,225,223,221,218,216,214,212,209,207,204,202,199,197,194,191,189,
186,183,180,177,174,171,168,165,162,159,156,153,150,147,144,141,138,135,131,128,125,122,
119,116,113,109,106,103,100,97,94,91,88,85,82,79,76,73,71,68,65,62,60,57,54,52,49,47,45,
42,40,38,35,33,31,29,27,25,23,22,20,18,17,15,14,12,11,10,9,8,6,6,5,4,3,2,2,1,1,1,0,0,0,0
};

//Gamma Correction Curve
uint8_t const exp_gamma[256] PROGMEM=
{0,0,0,0,0,0,0,0,0,0,0,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,2,3,3,3,3,3,
4,4,4,4,4,5,5,5,5,5,6,6,6,7,7,7,7,8,8,8,9,9,9,10,10,10,11,11,12,12,12,13,13,14,14,14,15,15,
16,16,17,17,18,18,19,19,20,20,21,21,22,23,23,24,24,25,26,26,27,28,28,29,30,30,31,32,32,33,
34,35,35,36,37,38,39,39,40,41,42,43,44,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,
61,62,63,64,65,66,67,68,70,71,72,73,74,75,77,78,79,80,82,83,84,85,87,89,91,92,93,95,96,98,
99,100,101,102,105,106,108,109,111,112,114,115,117,118,120,121,123,125,126,128,130,131,133,
135,136,138,140,142,143,145,147,149,151,152,154,156,158,160,162,164,165,167,169,171,173,175,
177,179,181,183,185,187,190,192,194,196,198,200,202,204,207,209,211,213,216,218,220,222,225,
227,229,232,234,236,239,241,244,246,249,251,253,254,255
};

inline uint8_t fastCosineCalc( uint16_t preWrapVal)
{
  uint8_t wrapVal = (preWrapVal % 255);
  if (wrapVal<0) wrapVal=255+wrapVal;
  return (pgm_read_byte_near(cos_wave+wrapVal));
}

void plasmaPrime()
{
  frameCount++ ;
  uint16_t t = fastCosineCalc((92 * frameCount)/700);  //time displacement - fiddle with these til it looks good...
  uint16_t t2 = fastCosineCalc((55 * frameCount)/700);
  uint16_t t3 = fastCosineCalc((83 * frameCount)/700);
  for (uint8_t y = 0; y <= 15; y++)
  {
     for (uint8_t x = 0; x <= 15; x++)
     {
        uint8_t r = fastCosineCalc(((x << 3) + (t >> 1) + fastCosineCalc((t2 + (y << 3)))));  //Calculate 3 seperate plasma waves, one for each color channel
        uint8_t g = fastCosineCalc(((y << 3) + t + fastCosineCalc(((t3 >> 2) + (x << 3)))));
        uint8_t b = fastCosineCalc(((y << 3) + t2 + fastCosineCalc((t + x + (g >> 2)))));
        //uncomment the following to enable gamma correction
        r=pgm_read_byte_near(exp_gamma+r);
        g=pgm_read_byte_near(exp_gamma+g);
        b=pgm_read_byte_near(exp_gamma+b);
        leds[getIndex(x, y)] = CRGB(r, g, b);

    }
  }
  FastLED.show();
}

// effect 2
// plasma demo for Adafruit RGBmatrixPanel library.
// Modified by Jeremy Williams for Game Frame
// Demonstrates double-buffered animation our 16x32 RGB LED matrix:
// http://www.adafruit.com/products/420

// Written by Limor Fried/Ladyada & Phil Burgess/PaintYourDragon
// for Adafruit Industries.
// BSD license, all text above must be included in any redistribution.

float radius1  = 65.2/4, radius2  = 92.0/4, radius3  = 163.2/4, radius4  = 176.8/4,
            centerx1 = 64.4/4, centerx2 = 46.4/4, centerx3 =  93.6/4, centerx4 =  16.4/4,
            centery1 = 34.8/4, centery2 = 26.0/4, centery3 =  56.0/4, centery4 = -11.6/4;
float       angle1   =  0.0, angle2   =  0.0, angle3   =   0.0, angle4   =   0.0;
long        hueShift =  0;

void randomizePlasma()
{
  Serial.println("Randomizing.");
  radius1  = random(0, 1000)/10;
  radius2  = random(0, 1000)/10;
  radius3  = random(0, 2000)/10;
  radius4  = random(0, 2000)/10;
  centerx1 = random(0, 1000)/10;
  centerx2 = random(0, 1000)/10;
  centerx3 =  random(0, 1000)/10;
  centerx4 =  random(0, 1000)/10;
  centery1 = random(0, 1000)/10;
  centery2 = random(0, 1000)/10;
  centery3 =  random(0, 1000)/10;
  centery4 = -random(0, 1000)/10;
}

void plasmaSlow() {
  frameCount++;
  float           x1, x2, x3, x4, y1, y2, y3, y4, sx1, sx2, sx3, sx4;
  unsigned char x, y;
  long          value;

  sx1 = (cos(angle1) * radius1 + centerx1);
  sx2 = (cos(angle2) * radius2 + centerx2);
  sx3 = (cos(angle3) * radius3 + centerx3);
  sx4 = (cos(angle4) * radius4 + centerx4);
  y1  = (sin(angle1) * radius1 + centery1);
  y2  = (sin(angle2) * radius2 + centery2);
  y3  = (sin(angle3) * radius3 + centery3);
  y4  = (sin(angle4) * radius4 + centery4);

  for(y=0; y<16; y++) {
    x1 = sx1; x2 = sx2; x3 = sx3; x4 = sx4;
    for(x=0; x<16; x++) {
      value = hueShift
        + sqrt(x1 * x1 + y1 * y1)
        + sqrt(x2 * x2 + y2 * y2)
        + sqrt(x3 * x3 + y3 * y3)
        + sqrt(x4 * x4 + y4 * y4);
        leds[getIndex(x, y)] = CHSV(value * 3, 255, 255);
      x1--; x2--; x3--; x4--;
    }
    y1--; y2--; y3--; y4--;
  }

  angle1 = frameCount * 0.03 / 25;
  angle2 = frameCount * -0.07 / 25;
  angle3 = frameCount * 0.13 / 25;
  angle4 = frameCount * -0.15 / 25;
  hueShift += 0;

  FastLED.show();
}
