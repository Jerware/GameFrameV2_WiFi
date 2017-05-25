// Game Frane Progromatic Effects
// This implementation is rough and careless RAM wise.
// The effects should probably be converted to classes.

// prototypes
byte getIndex(byte x, byte y);
void clearStripBuffer();
void runEffects();
void nextEffect();
void plasmaPrime();
void plasmaSlow();
void randomizePlasma();
void fxSwirl();
void FillNoise(byte layer);
void MirroredNoise();
void RedClouds();
void Lavalamp1();
void Lavalamp2();
void Lavalamp3();
void Lavalamp4();
void Lavalamp5();
void Constrained1();
void RelativeMotion1();
void Water();
void WaterJer();
void Bubbles1();
void TripleMotion();
void Caleido1();
void Caleido2();
void Caleido3();
void Caleido4();
void Caleido5();
void Caleido6();
void Caleido7();
void RandomAnimation();
void CrossNoise();
void CrossNoise2();
void BasicVariablesSetup();
uint16_t beatsin(accum88 beats_per_minute, uint16_t lowest = 0, uint16_t highest = 65535, byte phase = 0);
void FillNoise(byte layer);
void PaletteRed();
void PaletteCustom();
void Palette16();
void SetupRandomPalette();
void SetupRandomPalette2();
void SetupRandomPalette3();
void SetupRandomPalette4();
void Caleidoscope1();
void Caleidoscope2();
void Caleidoscope3();
void Caleidoscope4();
void Caleidoscope5();
void ShowLayer(byte layer, byte colorrepeat);
void MergeMethod1(byte colorrepeat);
void MergeMethod2(byte colorrepeat);
void MergeMethod3(byte colorrepeat);
void MergeMethod4(byte colorrepeat);
void ConstrainedMapping(byte layer, byte lower_limit, byte upper_limit, byte colorrepeat);
void ShowLayerBright(byte layer, byte colorrepeat);
void FilterAll();
void CrossMapping(byte colorrepeat, byte limit);
void CrossMapping(byte colorrepeat, byte limit);
void fxFire();
CRGB ColorFromCurrentPalette(uint8_t index = 0, uint8_t brightness = 255, TBlendType blendType = LINEARBLEND);
void MoveFractionalNoiseX(byte amt = 16);
void Spiral(int x,int y, int r, byte dimm);
void fxSpiral();
int XY(int x, int y);
void Line(int x0, int y0, int x1, int y1, byte color);
void whatIsTheMatrix();

#define kMatrixWidth  16
#define kMatrixHeight 16
#define NUM_LEDS (kMatrixWidth * kMatrixHeight)
extern uint8_t playMode;
extern int currentEffect;
extern int secondCounter;
extern unsigned long baseTime;
extern CRGB leds[NUM_LEDS];
extern int realRandom(int max);

uint16_t numEffects = 13; // number of effects

// // unused effects
// Lavalamp2();
// Lavalamp4();
// Lavalamp5();
// Constrained1();
// Bubbles1();
// TripleMotion();
// Caleido1();
// Caleido2();
// Caleido4();
// Caleido5();
// Caleido6();
// Caleido7();
// CrossNoise();
// RandomAnimation();


void runEffects()
{
  if (currentEffect == 0) plasmaPrime();
  else if (currentEffect == 1) plasmaSlow();
  else if (currentEffect == 2) fxSwirl();
  else if (currentEffect == 3) MirroredNoise();
  else if (currentEffect == 4) Lavalamp1();
  else if (currentEffect == 5) Lavalamp3();
  else if (currentEffect == 6) RelativeMotion1();
  else if (currentEffect == 7) Water();
  else if (currentEffect == 8) Caleido3();
  else if (currentEffect == 9) CrossNoise2();
  else if (currentEffect == 10) fxFire();
  else if (currentEffect == 11) fxSpiral();
  else if (currentEffect == 12) whatIsTheMatrix();
}

void nextEffect()
{
  clearStripBuffer();
  BasicVariablesSetup();
  if (playMode == 0) currentEffect++;
  else
  {
    uint8_t lastEffect = currentEffect;
    while (currentEffect == lastEffect)
    {
      currentEffect = random8(numEffects);
    }
  }
  if (currentEffect > numEffects - 1) currentEffect = 0;
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

// swirl pattern by Mark Kriegsman
// https://gist.github.com/kriegsman/5adca44e14ad025e6d3b

void fxSwirl()
{
  // Apply some blurring to whatever's already on the matrix
  // Note that we never actually clear the matrix, we just constantly
  // blur it repeatedly.  Since the blurring is 'lossy', there's
  // an automatic trend toward black -- by design.
  uint8_t blurAmount = beatsin8(2,10,255);
  blurRows(leds, 16, 16, blurAmount);
  // blurColumns
  uint8_t keep = 255 - blurAmount;
  uint8_t seep = blurAmount >> 1;
  for( uint8_t col = 0; col < 16; col++) {
      CRGB carryover = CRGB::Black;
      for( uint8_t i = 0; i < 16; i++) {
          CRGB cur = leds[getIndex(col,i)];
          CRGB part = cur;
          part.nscale8( seep);
          cur.nscale8( keep);
          cur += carryover;
          if( i) leds[getIndex(col,i-1)] += part;
          leds[getIndex(col,i)] = cur;
          carryover = part;
      }
  }

  // Use two out-of-sync sine waves
  uint8_t  i = beatsin8( 27, 3, 16-3);
  uint8_t  j = beatsin8( 41, 3, 16-3);
  // Also calculate some reflections
  uint8_t ni = (16-1)-i;
  uint8_t nj = (16-1)-j;

  // The color of each point shifts over time, each at a different speed.
  uint16_t ms = millis();
  leds[getIndex( i, j)] += CHSV( ms / 11, 200, 255);
  leds[getIndex( j, i)] += CHSV( ms / 13, 200, 255);
  leds[getIndex(ni,nj)] += CHSV( ms / 17, 200, 255);
  leds[getIndex(nj,ni)] += CHSV( ms / 29, 200, 255);
  leds[getIndex( i,nj)] += CHSV( ms / 37, 200, 255);
  leds[getIndex(ni, j)] += CHSV( ms / 41, 200, 255);

  FastLED.show();
}

// Stefan Petrick Routines
// https://github.com/StefanPetrick/FunkyNoise

// used in FillNoise for central zooming
byte CentreX =  (kMatrixWidth / 2) - 1;
byte CentreY = (kMatrixHeight / 2) - 1;

// a place to store the color palette
CRGBPalette16 currentPalette;

// can be used for palette rotation
// "colorshift"
byte colorshift;

// The coordinates for 3 16-bit noise spaces.
#define NUM_LAYERS 3

uint32_t x[NUM_LAYERS];
uint32_t y[NUM_LAYERS];
uint32_t z[NUM_LAYERS];
uint32_t scale_x[NUM_LAYERS];
uint32_t scale_y[NUM_LAYERS];

// used for the random based animations
int16_t dx;
int16_t dy;
int16_t dz;
int16_t dsx;
int16_t dsy;

// a 3dimensional array used to store the calculated
// values of the different noise planes
uint8_t noise[NUM_LAYERS][kMatrixWidth][kMatrixHeight];

// used for the color histogramm
uint16_t values[256];

uint8_t noisesmoothing;

byte red_level;
byte green_level;
byte blue_level;
byte pgm;
byte spd;

// routines

void MirroredNoise() {

  // move within the noise space
  x[0] += 100;
  z[0] += 100;
  scale_x[0] = 4000;
  scale_y[0] = 4000;

  // calculate the noise array
  FillNoise(0);

  currentPalette = RainbowStripeColors_p;
  noisesmoothing = 10;

  for(int i = 0; i < kMatrixWidth; i++) {
    for(int j = 0; j < kMatrixHeight; j++) {

      // map the noise values down
      uint16_t index = ( noise[0][i][j] + noise[0][kMatrixWidth - 1 - i][j] ) / 2;
      uint16_t   bri = 255;
      // assign a color from the HSV space
      CRGB color = ColorFromPalette( currentPalette, index, bri);

      leds[getIndex(i,j)] = color;
    }
  }
  FastLED.show();
}

// meh needs work
void RedClouds() {

  // clear the screenbuffer
  clearStripBuffer();

  PaletteRed();
  colorshift = 240;

  // move within the noise space
  x[0] = beatsin16(1)*10;
  y[0] += 2000;
  z[0] += 100;
  scale_x[0] = 6000;
  scale_x[0] = 6000;

  // calculate the noise array
  FillNoise(0);

  for(int i = 0; i < kMatrixWidth; i++) {
    for(int j = 0; j < kMatrixHeight; j++) {

      // map the noise values down to a byte range
      uint16_t index = noise[0][i][j];
      uint16_t   bri = 255;
      // assign a color depending on the actual palette
      CRGB color = ColorFromPalette( currentPalette, index + colorshift, bri);

      // draw only the part lower than the threshold
      if (index < 128) {
        leds[getIndex(i,j)] = color;
      }
    }
  }
  FastLED.show();
}

// Lavalamp1
// works good with the RedBlack palette

void Lavalamp1() {

  PaletteRed();
  colorshift = 0;

  x[0] = beatsin16(3, 200, 64000);
  y[0] += 100;
  z[0] = 7000;
  scale_x[0] = 6000;
  scale_y[0] = 8000;
  FillNoise(0);

  x[1] = beatsin16(2, 200, 64000);
  y[1] += 130;
  z[1] = 7000;
  scale_x[1] = 6000;
  scale_y[1] = 8000;
  FillNoise(1);

  x[2] = beatsin16(4, 200, 6400);
  y[2] += 1000;
  z[2] = 3000;
  scale_x[2] = 7000;
  scale_y[2] = 8000;
  FillNoise(2);

  noisesmoothing = 200;

  MergeMethod1(2);
  //Show3Layers();

  FastLED.show();
}


// with a scrolling palette

void Lavalamp2() {

  currentPalette = PartyColors_p;

  noisesmoothing = 200;

  x[0] = beatsin16(3, 200, 64000);
  y[0] = beatsin16(4, 200, 64000);
  z[0] = 7000;
  scale_x[0] = beatsin16(2, 6000, 8000);
  scale_y[0] = beatsin16(1, 4000, 12000);
  FillNoise(0);

  x[1] = beatsin16(5, 200, 64000);
  y[1] = beatsin16(6, 200, 64000);
  z[1] = 6000;
  scale_x[1] = 6000;
  scale_y[1] = 8000;
  FillNoise(1);

  x[2] = beatsin16(4, 200, 6400);
  y[2] += 1000;
  z[2] = 3000;
  scale_x[2] = 7000;
  scale_y[2] = 8000;
  FillNoise(2);

  colorshift++;

  MergeMethod1(2);
  //Show3Layers();

  FastLED.show();
}


// a very slow one

void Lavalamp3() {

  noisesmoothing = 40;
  currentPalette = ForestColors_p;

  y[0] += 100;
  z[0] = 7000;
  scale_x[0] = 6000;
  scale_y[0] = 6000;
  FillNoise(0);

  y[1] += 200;
  z[1] = 3000;
  scale_x[1] = 7000;
  scale_y[1] = 8000;
  FillNoise(1);

  y[2] += 250;
  z[2] = 6000;
  scale_x[2] = 20000;
  scale_y[2] = 8000;
  FillNoise(2);

  MergeMethod1(1);
  //Show3Layers();

  FastLED.show();
}


// the palette can also be defined within the animation

void Lavalamp4() {

  currentPalette = CRGBPalette16(
  CHSV(   0, 255, 0    ),
  CHSV(   0, 255, 255  ),
  CHSV(   0, 255, 0    ),
  CHSV( 160, 255, 255  ));

  noisesmoothing = 150;

  y[0] += 100;
  z[0] = 7000;
  scale_x[0] = 6000;
  scale_y[0] = 6000;
  FillNoise(0);

  y[1] += 200;
  z[1] = 3000;
  scale_x[1] = 7000;
  scale_y[1] = 8000;
  FillNoise(1);

  y[2] += 250;
  z[2] = 6000;
  scale_x[2] = 20000;
  scale_y[2] = 8000;
  FillNoise(2);

  MergeMethod1(2);
  //Show3Layers();

  FastLED.show();
}


// lets play with the scaling of 2 layers

void Lavalamp5() {

  currentPalette = CRGBPalette16(
  CHSV(   0, 255, 0   ),
  CHSV(   0, 200, 255 ),
  CHSV(  63, 150, 255 ),
  CHSV( 160, 255, 0   ));

  noisesmoothing = 50;

  y[0] += 1000;
  z[0] = 7000;
  scale_x[0] = beatsin16(3, 1000, 20000);
  scale_y[0] = 6000;
  FillNoise(0);

  y[1] += 2000;
  z[1] = 3000;
  scale_x[1] = beatsin16(4, 1000, 20000);
  scale_y[1] = 8000;
  FillNoise(1);

  y[2] += 3000;
  z[2] = 6000;
  scale_x[2] = beatsin16(5, 1000, 20000);
  scale_y[2] = 8000;
  FillNoise(2);

  MergeMethod2(3);
  //Show3Layers();

  FastLED.show();
}

// 2 layers of constrained noise using differnt palettes for color mapping

void Constrained1() {

  noisesmoothing = 100;
  colorshift = 0;

  x[0] += 2000;
  scale_x[0] = 6000;
  scale_y[0] = 6000;
  FillNoise(0);

  x[1] -= 2000;
  scale_x[1] = 6000;
  scale_y[1] = 6000;
  FillNoise(1);

  clearStripBuffer();

  // define a palette used for layer 0
  currentPalette = CRGBPalette16(
  CHSV(   0, 255, 0   ),
  CHSV(   0, 255, 0   ),
  CHSV(   0, 255, 255 ),
  CHSV( 160, 255, 0   ));

  // map layer 0 (red) for noise values between 100 and 200
  ConstrainedMapping( 0, 100, 200, 1);

  // palette for the second layer
  currentPalette = CRGBPalette16(
  CHSV( 0, 255, 0   ),
  CHSV( 0, 255, 0 ),
  CHSV( 160, 255, 255 ),
  CHSV( 160, 255, 0   ));

  // map layer 1 (blue) for noise values between 100 and 200
  ConstrainedMapping( 1, 100, 200, 1);

  FastLED.show();
}

// move 2 layers relative to each other

void RelativeMotion1() {

  currentPalette = CRGBPalette16(
  CHSV(  0, 255, 0   ),
  CHSV( 80, 255, 255 ),
  CHSV( 60, 255, 255 ),
  CHSV(  0, 255, 0   ));

  colorshift = beatsin8(10);
  noisesmoothing = 100;

  x[0] = 5 * beatsin16(2, 15000, 40000);
  y[0] = 5 * beatsin16(3, 15000, 40000);
  z[0] += 100;
  scale_x[0] = 6000 + beatsin16(30, 0, 4000);
  scale_y[0] = 8000 + beatsin16(27, 0, 4000);
  FillNoise(0);

  x[1] = x[0] + (5 * beatsin16(30, 0, 10000)) - 25000;
  y[1] = y[0] + (5 * beatsin16(40, 0, 10000)) - 25000;
  z[1] += 100;
  scale_x[1] = 6000 + beatsin16(30, 0, 3000);
  scale_y[1] = 8000 + beatsin16(27, 0, 3000);
  FillNoise(1);

  MergeMethod3(1);

  FastLED.show();
}


// first approach of a water simulation
// uses a beatsin function with phase shift

void Water() {

  currentPalette = OceanColors_p;
  EVERY_N_MILLIS(50) {colorshift++;}
  noisesmoothing = 200;

  // 2 sinewaves shiftet by 63 (90 degrees)
  // results in a circular motion
  x[0] = 10 * beatsin16(5, 0, 10000, 0, 0);
  y[0] = 10 * beatsin16(5, 0, 10000, 0, 63);
  z[0] += 10;
  scale_x[0] = 6000;
  scale_y[0] = 8000;
  FillNoise(0);

  x[1] = x[0] + (10 * beatsin16(30, 0, 10000, 0, 0)) - 50000;
  y[1] = y[0] + (10 * beatsin16(30, 0, 10000, 0, 63)) - 50000;
  z[1] += 10;
  scale_x[1] = 6000;
  scale_y[1] = 8000;
  FillNoise(1);

  MergeMethod3(3);

  FastLED.show();
}

void WaterJer() {

  currentPalette = OceanColors_p;
  EVERY_N_MILLIS(50) {colorshift++;}
  noisesmoothing = 200;

  // 2 sinewaves shiftet by 63 (90 degrees)
  // results in a circular motion
  x[0] = 10 * beatsin16(0, 0, 10000, 0, 0);
  y[0] = 10 * beatsin16(0, 0, 10000, 0, 63);
  z[0] += 100;
  scale_x[0] = 6000;
  scale_y[0] = 8000;
  FillNoise(0);

  x[1] = x[0] + (10 * beatsin16(0, 0, 10000, 0, 0)) - 50000;
  y[1] = y[0] + (10 * beatsin16(0, 0, 10000, 0, 63)) - 50000;
  z[1] += 100;
  scale_x[1] = 6000;
  scale_y[1] = 8000;
  FillNoise(1);

  MergeMethod3(3);

  FastLED.show();
}

// outlined bubbles by constrained mapping + palette

void Bubbles1() {

  noisesmoothing = 200;
  PaletteRed();
  colorshift = 0;

  x[0] = beatsin16(7);
  y[0] += 2000;
  z[0] = 7000;
  scale_x[0] = 6000;
  scale_y[0] = 6000;
  FillNoise(0);

  x[1] = beatsin16(8);
  y[1] += 3000;
  z[1] = 10000;
  scale_x[1] = 6000;
  scale_y[1] = 6000;
  FillNoise(1);

  clearStripBuffer();

  ConstrainedMapping(1, 0, 100, 3);
  ConstrainedMapping(0, 0, 100, 3);

  FastLED.show();
}

// layer2 movving arround a layer1 moving arround a layer0

void TripleMotion() {

  currentPalette = RainbowColors_p;
  colorshift++;
  noisesmoothing = 200;

  x[0] = 10 * beatsin16(10, 0, 10000, 0, 0);
  y[0] = 10 * beatsin16(9, 0, 10000, 0, 63);
  z[0] += 1000;
  scale_x[0] = 6000;
  scale_y[0] = 8000;
  FillNoise(0);

  x[1] = x[0] + (10 * beatsin16(13, 0, 10000, 0, 0));
  y[1] = y[0] + (10 * beatsin16(12, 0, 10000, 0, 63));
  z[1] += 1000;
  scale_x[1] = 6000;
  scale_y[1] = 8000;
  FillNoise(1);

  x[2] = x[1] + (10 * beatsin16(18, 0, 10000, 0, 0));
  y[2] = y[1] + (10 * beatsin16(17, 0, 10000, 0, 63));
  z[2] += 1000;
  scale_x[2] = 6000;
  scale_y[2] = 8000;
  FillNoise(2);

  MergeMethod4(2);
  //Show3Layers();

  FastLED.show();
}

void Caleido1() {

  EVERY_N_SECONDS(4) {

    SetupRandomPalette2();
    dy = random16(4000) - 2000;
    dx = random16(4000) - 2000;
    dz = random16(4000) - 2000;
    scale_x[0] = random16(10000) + 2000;
    scale_y[0] = random16(10000) + 2000;
  }

  y[0] += dy;
  x[0] += dx;
  z[0] += dz;
  FillNoise(0);

  ShowLayer(0, 1);
  Caleidoscope2();

  FastLED.show();
}


void Caleido2() {

  EVERY_N_SECONDS(4) {

    SetupRandomPalette3();
    dy = random16(4000) - 2000;
    dx = random16(4000) - 2000;
    dz = random16(4000) - 2000;
    scale_x[0] = random16(10000) + 2000;
    scale_y[0] = random16(10000) + 2000;
  }

  y[0] += dy;
  x[0] += dx;
  z[0] += dz;
  FillNoise(0);

  ShowLayer(0, 1);
  Caleidoscope1();

  FastLED.show();
}

void Caleido3() {

  // a new parameter set every 15 seconds
  EVERY_N_SECONDS(15) {

    SetupRandomPalette3();
    dy = random16(500) - 250; // random16(2000) - 1000 is pretty fast but works fine, too
    dx = random16(500) - 250;
    dz = random16(500) - 250;
    scale_x[0] = random16(10000) + 2000;
    scale_y[0] = random16(10000) + 2000;
  }

  y[0] += dy;
  x[0] += dx;
  z[0] += dz;

  FillNoise(0);
  ShowLayer(0, 1);

  Caleidoscope3();
  Caleidoscope1();

  FastLED.show();
}

void Caleido4() {

  EVERY_N_SECONDS(45) {

    SetupRandomPalette3();
    /*
    dy = random16(2000) - 1000; // random16(2000) - 1000 is pretty fast but works fine, too
     dx = random16(2000) - 1000;
     dz = random16(2000) - 1000;
     */

    dy = random16(500) - 250;
    dx = random16(500) - 250;
    dz = random16(500) - 250;

    scale_x[0] = random16(10000) + 2000;
    scale_y[0] = random16(10000) + 2000;
  }

  y[0] += dy;
  x[0] += dx;
  z[0] += dz;

  FillNoise(0);
  ShowLayer(0, 1);

  Caleidoscope4();
  Caleidoscope2();

  FastLED.show();
}

void Caleido5() {

  // a new parameter set every 10 seconds
  EVERY_N_SECONDS(10) {

    SetupRandomPalette4();

    dy = random16(1000) - 500; // random16(2000) - 1000 is pretty fast but works fine, too
    dx = random16(1000) - 500;
    dz = random16(500);


    scale_x[0] = random16(7000) + 2000;
    scale_y[0] = random16(7000) + 2000;
  }

  y[0] += dy;
  x[0] += dx;
  z[0] += dz;

  EVERY_N_MILLIS(50) {
    colorshift++;
  }

  FillNoise(0);
  ShowLayer(0, 1);

  Caleidoscope5();
  Caleidoscope4();
  Caleidoscope2();

  FastLED.show();
}


void Caleido6() {

  // a new parameter set every 10 seconds
  EVERY_N_SECONDS(10) {

    SetupRandomPalette4();

    dy = random16(1000) - 500; // random16(2000) - 1000 is pretty fast but works fine, too
    dx = random16(1000) - 500;
    dz = random16(500);


    scale_x[0] = random16(7000) + 2000;
    scale_y[0] = random16(7000) + 2000;
  }

  y[0] += dy;
  x[0] += dx;
  z[0] += dz;

  EVERY_N_MILLIS(50) {
    colorshift++;
  }

  FillNoise(0);
  ShowLayerBright(0, 1);

  //Caleidoscope5();
  Caleidoscope4();
  Caleidoscope1();

  FastLED.show();
}


void Caleido7() {

  EVERY_N_SECONDS(10) {

    SetupRandomPalette4();

    dy = random16(1000) - 500;
    dx = random16(1000) - 500;
    dz = random16(500);


    scale_x[0] = random16(7000) + 2000;
    scale_y[0] = random16(7000) + 2000;
  }

  y[0] += dy;
  x[0] += dx;
  z[0] += dz;

  EVERY_N_MILLIS(50) {
    colorshift++;
  }

  FillNoise(0);
  ShowLayerBright(0, 1);

  Caleidoscope4();
  Caleidoscope1();
  FilterAll();

  FastLED.show();
}

void CrossNoise() {

  currentPalette = RainbowStripeColors_p;
  colorshift = 50;
  noisesmoothing = 20;

  //x[0] = 10 * beatsin(10, 0, 10000, 0);
  y[0] += 100;
  z[0] += 50;
  scale_x[0] = 4000;
  scale_y[0] = 4000;
  FillNoise(0);
  byte border = beatsin8(10, 20, 236);

  CrossMapping(1, border);

  FastLED.show();
}


void CrossNoise2() {

  currentPalette = RainbowStripeColors_p;
  noisesmoothing = 20;

  y[0] += 100;
  z[0] += 50;
  scale_x[0] = beatsin16(3,1000,10000);
  scale_y[0] = beatsin16(2,1000,10000);
  FillNoise(0);

  byte border = beatsin8(8);

  CrossMapping(1, border);

  FastLED.show();
}

void RandomAnimation() {

  noisesmoothing = 100;

  // danger: dirty + crappy code!
  // use EVERY_N_MILLIS instead!
  EVERY_N_SECONDS(10) {
    SetupRandomPalette();
    dy = random(2000) - 1000;
    dx = random(2000) - 1000;
    dz = random(2000) - 1000;
    scale_x[0] = random(8000) + 2000;
    scale_y[0] = random(8000) + 2000;
  }

  y[0] += dy;
  x[0] += dx;
  z[0] += dz;
  FillNoise(0);

  ShowLayer(0, 2);

  FastLED.show();
}



// helper functions
// Initialise the coordinates of the noise space with random
// values for an altering starting point.
// Set the zoom factor to a moderate level.
// Fill the delta values with random stepwidths.

void BasicVariablesSetup() {

  // set to reasonable values to avoid a black out
  colorshift = 0;
  noisesmoothing = 200;

  currentPalette = RainbowStripeColors_p;


  // just any free input pin
  random16_add_entropy(analogRead(18));

  // fill coordinates with random values
  // set zoom levels
  for(int i = 0; i < NUM_LAYERS; i++) {
    x[i] = random16();
    y[i] = random16();
    z[i] = random16();
    scale_x[i] = 6000;
    scale_y[i] = 6000;
  }
  // for the random movement
  dx  = random8();
  dy  = random8();
  dz  = random8();
  dsx = random8();
  dsy = random8();

  // everything for the menu
  spd = 10;
  red_level = 255;
  green_level = 255;
  blue_level = 255;
}

// Fill the x/y array with 16-bit noise values

void FillNoise(byte layer) {

  for(uint8_t i = 0; i < kMatrixWidth; i++) {

    uint32_t ioffset = scale_x[layer] * (i-CentreX);

    for(uint8_t j = 0; j < kMatrixHeight; j++) {

      uint32_t joffset = scale_y[layer] * (j-CentreY);

      byte data = inoise16(x[layer] + ioffset, y[layer] + joffset, z[layer]) >> 8;

      uint8_t olddata = noise[layer][i][j];
      uint8_t newdata = scale8( olddata, noisesmoothing ) + scale8( data, 256 - noisesmoothing );
      data = newdata;


      noise[layer][i][j] = data;
    }
  }
}

// All the caleidoscope functions work directly within the screenbuffer (leds array).
// Draw whatever you like in the area x(0-15) and y (0-15) and then copy it arround.

// rotates the first 16x16 quadrant 3 times onto a 32x32 (+90 degrees rotation for each one)

void Caleidoscope1() {
  for(int x = 0; x < kMatrixWidth / 2 ; x++) {
    for(int y = 0; y < kMatrixHeight / 2; y++) {
      leds[getIndex( kMatrixWidth - 1 - x, y )] = leds[getIndex( y, x )];
      leds[getIndex( kMatrixWidth - 1 - x, kMatrixHeight - 1 - y )] = leds[getIndex( x, y )];
      leds[getIndex( x, kMatrixHeight - 1 - y )] = leds[getIndex( y, x )];
    }
  }
}


// mirror the first 16x16 quadrant 3 times onto a 32x32

void Caleidoscope2() {
  for(int x = 0; x < kMatrixWidth / 2 ; x++) {
    for(int y = 0; y < kMatrixHeight / 2; y++) {
      leds[getIndex( kMatrixWidth - 1 - x, y )] = leds[getIndex( x, y )];
      leds[getIndex( x, kMatrixHeight - 1 - y )] = leds[getIndex( x, y )];
      leds[getIndex( kMatrixWidth - 1 - x, kMatrixHeight - 1 - y )] = leds[getIndex( x, y )];
    }
  }
}


// copy one diagonal triangle into the other one within a 16x16

void Caleidoscope3() {
  for(int x = 0; x <= CentreX ; x++) {
    for(int y = 0; y <= x; y++) {
      leds[getIndex( x, y )] = leds[getIndex( y, x )];
    }
  }
}


// copy one diagonal triangle into the other one within a 16x16 (90 degrees rotated compared to Caleidoscope3)

void Caleidoscope4() {
  for(int x = 0; x <= CentreX ; x++) {
    for(int y = 0; y <= CentreY-x; y++) {
      leds[getIndex( CentreY - y, CentreX - x )] = leds[getIndex( x, y )];
    }
  }
}


// copy one diagonal triangle into the other one within a 8x8

void Caleidoscope5() {
  for(int x = 0; x < kMatrixWidth/4 ; x++) {
    for(int y = 0; y <= x; y++) {
      leds[getIndex( x, y )] = leds[getIndex( y, x )];
    }
  }

  for(int x = kMatrixWidth/4; x < kMatrixWidth/2 ; x++) {
    for(int y = kMatrixHeight/4; y >= 0; y--) {
      leds[getIndex( x, y )] = leds[getIndex( y, x )];
    }
  }
}

/*
 Some color palettes.
 Includes the predifined FastLED palettes and custom ones.

 -----------------------------------------------------------------
 */


// A red-black palette.

void PaletteRed() {
  currentPalette = CRGBPalette16(
  CHSV( 0, 255, 255 ),
  CHSV( 0, 255, 0   ),
  CHSV( 0, 255, 0   ),
  CHSV( 0, 255, 255));
}


void PaletteCustom() {
  currentPalette = CRGBPalette16(
  CHSV( 40, 255, 255),
  CHSV( 40, 255, 255),
  CHSV( 0, 255, 0   ),
  CHSV( 0, 255, 255));
}

// Set here a global color palette.
// All the the predifined FastLED palettes:

void SetupRandomPalette() {
  currentPalette = CRGBPalette16(
  CHSV( random8(), 255, 32 ),
  CHSV( random8(), 255, 255),
  CHSV( random8(), 128, 255),
  CHSV( random8(), 255, 255));
}


void Palette16() {
  currentPalette = CRGBPalette16(
  0x000000, 0xFF0000, 0xFF0000, 0x000000,
  0x000000, 0x00FF00, 0x00FF00, 0x000000,
  0x000000, 0x0000FF, 0x0000FF, 0x000000,
  0x000000, 0xFF0000, 0xFF0000, 0x000000);
}


void SetupRandomPalette2() {
  currentPalette = CRGBPalette16(
  CHSV( random8(), 255, 0 ),
  CHSV( random8(), 255, 0),
  CHSV( random8(), 255, 0),
  CHSV( random8(), 255, 0),

  CHSV( random8(), 255, random8() ),
  CHSV( random8(), random8(), 255),
  CHSV( random8(), 255, 255),
  CHSV( random8(), 255, 255),

  CHSV( random8(), 255, 0 ),
  CHSV( random8(), 255, 255),
  CHSV( random8(), 255, 255),
  CHSV( random8(), random8(), 255),

  CHSV( random8(), 255, 0 ),
  CHSV( random8(), 255, 0),
  CHSV( random8(), 255, 0),
  CHSV( random8(), 255, 0));
}


void SetupRandomPalette3() {
  currentPalette = CRGBPalette16(
  CHSV( random8(), 255, 0 ),
  CHSV( random8(), 255, 0),
  CHSV( random8(), 255, 255),
  CHSV( random8(), 255, 255),

  CHSV( random8(), 255, 0 ),
  CHSV( random8(), 255, 255),
  CHSV( random8(), 255, 255),
  CHSV( random8(), 255, 0),

  CHSV( random8(), 255, 0 ),
  CHSV( random8(), 255, 255),
  CHSV( random8(), 255, 255),
  CHSV( random8(), 255, 0),

  CHSV( random8(), 255, 255 ),
  CHSV( random8(), 255, 255),
  CHSV( random8(), 255, 0),
  CHSV( random8(), 255, 0));
}


void SetupRandomPalette4() {
  currentPalette = CRGBPalette16(
  CHSV( random8(), 255, random8() ),
  CHSV( random8(), 255, random8()),
  CHSV( random8(), 255, 0),
  CHSV( random8(), 255, 255),

  CHSV( random8(), 255, random8() ),
  CHSV( random8(), 255, 255),
  CHSV( random8(), 255, 255),
  CHSV( random8(), random8(), random8()),

  CHSV( random8(), 255, random8() ),
  CHSV( random8(), 255, 255),
  CHSV( random8(), 255, 0),
  CHSV( random8(), 255, random8()),

  CHSV( random8(), 255, 255 ),
  CHSV( random8(), 255, 0),
  CHSV( random8(), 255, 255),
  CHSV( random8(), 255, random8()));
}

void ShowLayer(byte layer, byte colorrepeat) {
  for(uint8_t i = 0; i < kMatrixWidth; i++) {
    for(uint8_t j = 0; j < kMatrixHeight; j++) {

      uint8_t color = noise[layer][i][j];

      uint8_t   bri = color;

      // assign a color depending on the actual palette
      CRGB pixel = ColorFromPalette( currentPalette, colorrepeat * (color + colorshift), bri );

      leds[getIndex(i,j)] = pixel;
    }
  }
}

// overlay layers 0&1&2 for color, layer 2 is brightness

void MergeMethod1(byte colorrepeat) {
  for(uint8_t i = 0; i < kMatrixWidth; i++) {
    for(uint8_t j = 0; j < kMatrixHeight; j++) {

      uint8_t color = ( ( noise[0][i][j] )
        + ( noise[1][i][j] )
        + ( noise[2][i][j] ) )
        / 3;

      // layer 2 gives the brightness
      uint8_t   bri = (noise[2][i][j]);

      // assign a color depending on the actual palette
      CRGB pixel = ColorFromPalette( currentPalette, colorrepeat * (color + colorshift), bri );

      leds[getIndex(i,j)] = pixel;
    }
  }
}

// overlay layers 0&1 for color, layer 2 is brightness

void MergeMethod2(byte colorrepeat) {
  for(uint8_t i = 0; i < kMatrixWidth; i++) {
    for(uint8_t j = 0; j < kMatrixHeight; j++) {

      // map the noise values down to a byte range
      // layer 0 and 2 interfere for the color
      uint8_t color = ( ( noise[0][i][j] )
        + ( noise[1][i][j] ) )
        / 2;

      // layer 2 gives the brightness
      uint8_t   bri = (noise[2][i][j]);

      // assign a color depending on the actual palette
      CRGB pixel = ColorFromPalette( currentPalette, colorrepeat * (color + colorshift), bri );

      leds[getIndex(i,j)] = pixel;
    }
  }
}


// overlay layers 0&1 for color, brightness is layer1

void MergeMethod3(byte colorrepeat) {
  for(uint8_t i = 0; i < kMatrixWidth; i++) {
    for(uint8_t j = 0; j < kMatrixHeight; j++) {

      // map the noise values down to a byte range
      // layer 0 and 2 interfere for the color
      uint8_t color = ( ( noise[0][i][j] )
        + ( noise[1][i][j] ) )
        / 2;

      // layer 1 gives the brightness
      uint8_t   bri = noise[1][i][j];

      // assign a color depending on the actual palette
      CRGB pixel = ColorFromPalette( currentPalette, colorrepeat * (color + colorshift), bri );

      leds[getIndex(i,j)] = pixel;
    }
  }
}


// overlay layers 0&1&2 for color, layer 0 is brightness

void MergeMethod4(byte colorrepeat) {
  for(uint8_t i = 0; i < kMatrixWidth; i++) {
    for(uint8_t j = 0; j < kMatrixHeight; j++) {

      uint8_t color = ( ( noise[0][i][j] )
        + ( noise[1][i][j] )
        + ( noise[2][i][j] ) )
        / 3;

      uint8_t   bri = (noise[0][i][j]);

      // assign a color depending on the actual palette
      CRGB pixel = ColorFromPalette( currentPalette, colorrepeat * (color + colorshift), bri );

      leds[getIndex(i,j)] = pixel;
    }
  }
}

// draw the part between lower and upper limit of one layer

void ConstrainedMapping(byte layer, byte lower_limit, byte upper_limit, byte colorrepeat) {

  for(uint8_t i = 0; i < kMatrixWidth; i++) {
    for(uint8_t j = 0; j < kMatrixHeight; j++) {

      uint8_t data =  noise[layer][i][j] ;

      if ( data >= lower_limit  && data <= upper_limit) {

        CRGB pixel = ColorFromPalette( currentPalette, colorrepeat * (data + colorshift), data );

        leds[getIndex(i,j)] = pixel;
      }
    }
  }
}

// map a layer while ignoring the brightness information and replacing it by maximum

void ShowLayerBright(byte layer, byte colorrepeat) {
  for(uint8_t i = 0; i < kMatrixWidth; i++) {
    for(uint8_t j = 0; j < kMatrixHeight; j++) {

      uint8_t color = noise[layer][i][j];

      uint8_t   bri = 255;

      // assign a color depending on the actual palette
      CRGB pixel = ColorFromPalette( currentPalette, colorrepeat * (color + colorshift), bri );

      leds[getIndex(i,j)] = pixel;
    }
  }
}

// a brightness mask based on layer 0 for the complete screenbuffer

void FilterAll() {
  for(uint8_t i = 0; i < kMatrixWidth; i++) {
    for(uint8_t j = 0; j < kMatrixHeight; j++) {
      leds[getIndex(i,j)] %= noise[0][i][j];
    }
  }
}

// a constrained noise the fills the holes with a mirrored and recolored version of the same noise

void CrossMapping(byte colorrepeat, byte limit) {
  for(uint8_t i = 0; i < kMatrixWidth; i++) {
    for(uint8_t j = 0; j < kMatrixHeight; j++) {

      uint8_t color1 = noise[0][i][j];
      uint8_t color2 = noise[0][j][i];

      CRGB pixel;

      if (color1 > limit) {
        pixel = ColorFromPalette( currentPalette, colorrepeat * (color1 + colorshift), color2 );
      }
      else {
        pixel = ColorFromPalette( currentPalette, colorrepeat * (color2 + colorshift + 128), color1 );
      }
      leds[getIndex(i,j)] = pixel;
    }
  }
}

void DimAll(byte value)
{
  for(int i = 0; i < NUM_LEDS; i++) {
    leds[i].nscale8(value);
  }
}

// Based on Pattern Fire by Jason Coon
// https://github.com/pixelmatix/aurora/blob/d087efe69b539e2c00a026dde2fa205210d4f3b3/PatternFire.h

uint8_t heat[NUM_LEDS];

void fxFire()
{
  // cooling: How much does the air cool as it rises?
  // Less cooling = taller flames.  More cooling = shorter flames.
  // Default 55, suggested range 20-100
  int cooling = 75;

  // sparking: What chance (out of 255) is there that a new spark will be lit?
  // Higher chance = more roaring fire.  Lower chance = more flickery fire.
  // Default 120, suggested range 50-200.
  unsigned int sparking = 75;

    EVERY_N_MILLIS(30)
    {
      for (int x = 0; x < kMatrixWidth; x++) {
        // Step 1.  Cool down every cell a little
        for (int y = 0; y < kMatrixHeight; y++) {
          int xy = getIndex(x, y);
          heat[xy] = qsub8(heat[xy], random8(0, ((cooling * 10) / kMatrixHeight) + 2));
        }

        // Step 2.  Heat from each cell drifts 'up' and diffuses a little
        for (int y = 0; y < kMatrixHeight; y++) {
          heat[getIndex(x, y)] = (heat[getIndex(x, y + 1)] + heat[getIndex(x, y + 2)] + heat[getIndex(x, y + 2)]) / 3;
        }

        // Step 2.  Randomly ignite new 'sparks' of heat
        if (random8() < sparking) {
          // int x = (p[0] + p[1] + p[2]) / 3;

          int xy = getIndex(x, kMatrixHeight - 1);
          heat[xy] = qadd8(heat[xy], random8(160, 255));
        }

        // Step 4.  Map from heat cells to LED colors
        for (int y = 0; y < kMatrixHeight; y++) {
          int xy = getIndex(x, y);
          byte colorIndex = heat[xy];

          // Recommend that you use values 0-240 rather than
          // the usual 0-255, as the last 15 colors will be
          // 'wrapping around' from the hot end to the cold end,
          // which looks wrong.
          colorIndex = scale8(colorIndex, 240);

          // override color 0 to ensure a black background?
          if (colorIndex != 0)
            //                    effects.leds[xy] = CRGB::Black;
            //                else
            leds[xy] = ColorFromPalette(currentPalette, colorIndex);
        }
      }

      FillNoise(0);

      // blur2D
      uint8_t blurAmount = 75;
      blurRows(leds, 16, 16, blurAmount);
      // blurColumns
      uint8_t keep = 255 - blurAmount;
      uint8_t seep = blurAmount >> 1;
      for( uint8_t col = 0; col < 16; col++) {
        CRGB carryover = CRGB::Black;
        for( uint8_t i = 0; i < 16; i++) {
            CRGB cur = leds[getIndex(col,i)];
            CRGB part = cur;
            part.nscale8( seep);
            cur.nscale8( keep);
            cur += carryover;
            if( i) leds[getIndex(col,i-1)] += part;
            leds[getIndex(col,i)] = cur;
            carryover = part;
        }
      }
    }
  FastLED.show();
}

// Spiral by Stefan Petrick

byte count[4];  // counters for the wave functions

void fxSpiral()
{
  EVERY_N_MILLIS(25)
  {
    // first plant the seed into the buffer
    count[0] += 7;
    count[1] += 5;
    count[2] += 3;
    count[3] += 1;

    Line(((sin8(count[0])/17)+(sin8(count[2])/17))/2, ((sin8(count[1])/17)+(sin8(count[3])/17))/2,         // combine 2 wavefunctions for more variety of x
      ((sin8(count[3])/17)+(sin8(count[2])/17))/2, ((sin8(count[1])/17)+(sin8(count[2])/17))/2, count[0]); // and y, color just simple linear ramp

    // rotate leds
    Spiral(7,7,8,100);
  }

  FastLED.show();
}

void Spiral(int x,int y, int r, byte dimm) {
  for(int d = r; d >= 0; d--) {                // from the outside to the inside
    for(int i = x-d; i <= x+d; i++) {
       leds[XY(i,y-d)] += leds[XY(i+1,y-d)];   // lowest row to the right
       leds[XY(i,y-d)].nscale8( dimm );}
    for(int i = y-d; i <= y+d; i++) {
       leds[XY(x+d,i)] += leds[XY(x+d,i+1)];   // right colum up
       leds[XY(x+d,i)].nscale8( dimm );}
    for(int i = x+d; i >= x-d; i--) {
       leds[XY(i,y+d)] += leds[XY(i-1,y+d)];   // upper row to the left
       leds[XY(i,y+d)].nscale8( dimm );}
    for(int i = y+d; i >= y-d; i--) {
       leds[XY(x-d,i)] += leds[XY(x-d,i-1)];   // left colum down
       leds[XY(x-d,i)].nscale8( dimm );}
  }
}

// finds the right index for a S shaped matrix
int XY(int x, int y) {
  if(y > kMatrixHeight) { y = kMatrixHeight; }
  if(y < 0) { y = 0; }
  if(x > kMatrixWidth) { x = kMatrixWidth;}
  if(x < 0) { x = 0; }
  if(x % 2 == 1) {
  return (x * (kMatrixWidth) + (kMatrixHeight - y -1));
  } else {
    // use that line only, if you have all rows beginning at the same side
    return (x * (kMatrixWidth) + y);
  }
}

//  Bresenham line algorythm
void Line(int x0, int y0, int x1, int y1, byte color)
{
  int dx =  abs(x1-x0), sx = x0<x1 ? 1 : -1;
  int dy = -abs(y1-y0), sy = y0<y1 ? 1 : -1;
  int err = dx+dy, e2;
  for(;;){
    leds[XY(x0, y0)] += CHSV(color, 255, 255);
    if (x0==x1 && y0==y1) break;
    e2 = 2*err;
    if (e2 > dy) { err += dy; x0 += sx; }
    if (e2 < dx) { err += dx; y0 += sy; }
  }
}

// Matrix effect by Jeremy Williams

void whatIsTheMatrix()
{
  EVERY_N_MILLIS(75) // falling speed
  {
    // move code downward
    // start with lowest row to allow proper overlapping on each column
    for (int8_t row=kMatrixHeight-1; row>=0; row--)
    {
      for (int8_t col=0; col<kMatrixWidth; col++)
      {
        if (leds[getIndex(col, row)] == CRGB(175,255,175))
        {
          leds[getIndex(col, row)] = CRGB(27,130,39); // create trail
          if (row < kMatrixHeight-1) leds[getIndex(col, row+1)] = CRGB(175,255,175);
        }
      }
    }

    // fade all leds
    for(int i = 0; i < NUM_LEDS; i++) {
      if (leds[i].g != 255) leds[i].nscale8(192); // only fade trail
    }

    // check for empty screen to ensure code spawn
    bool emptyScreen = true;
    for(int i = 0; i < NUM_LEDS; i++) {
      if (leds[i])
      {
        emptyScreen = false;
        break;
      }
    }

    // spawn new falling code
    if (random8(3) == 0 || emptyScreen) // lower number == more frequent spawns
    {
      int8_t spawnX = random8(kMatrixWidth);
      leds[getIndex(spawnX, 0)] = CRGB(175,255,175 );
    }

    FastLED.show();
  }
}
