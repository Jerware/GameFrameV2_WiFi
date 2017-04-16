// APA102 Global Brightness Support

template <uint8_t DATA_PIN, uint8_t CLOCK_PIN, EOrder RGB_ORDER = BGR, uint8_t SPI_SPEED = DATA_RATE_MHZ(24)>
class APA102Controller_WithBrightness : public CLEDController {
  typedef SPIOutput<DATA_PIN, CLOCK_PIN, SPI_SPEED> SPI;
  SPI mSPI;
  uint8_t bBaseValue;

  void startBoundary() { mSPI.writeWord(0); mSPI.writeWord(0); }
  void endBoundary(int nLeds) { int nBytes = (nLeds/32); do { mSPI.writeWord(0xFF00); mSPI.writeWord(0x0000); } while(nBytes--); }

  inline void writeLed(uint8_t b0, uint8_t b1, uint8_t b2) __attribute__((always_inline)) {
    mSPI.writeByte(bBaseValue); mSPI.writeByte(b0); mSPI.writeByte(b1); mSPI.writeByte(b2);
  }

public:
  APA102Controller_WithBrightness() {bBaseValue = 0xFF;}

  virtual void init() {
    mSPI.init();
  }

  void setAPA102Brightness(uint8_t br) {
    bBaseValue = 0xE0 | (br & 0x1F);
  }

  uint8_t getAPA102Brightness() {
    return (bBaseValue & 0x1F);
  }
  virtual void clearLeds(int nLeds) {
    showColor(CRGB(0,0,0), nLeds, CRGB(0,0,0));
  }

protected:

  virtual void showColor(const struct CRGB & data, int nLeds, CRGB scale) {
    PixelController<RGB_ORDER> pixels(data, nLeds, scale, getDither());

    mSPI.select();

    startBoundary();
    for(int i = 0; i < nLeds; i++) {
      uint8_t b = pixels.loadAndScale0();
      mSPI.writeWord(bBaseValue << 8 | b);
      uint16_t w = pixels.loadAndScale1() << 8;
      w |= pixels.loadAndScale2();
      mSPI.writeWord(w);
      pixels.stepDithering();
    }
    endBoundary(nLeds);

    mSPI.waitFully();
    mSPI.release();
  }

  virtual void show(const struct CRGB *data, int nLeds, CRGB scale) {
    PixelController<RGB_ORDER> pixels(data, nLeds, scale, getDither());

    mSPI.select();

    startBoundary();
    for(int i = 0; i < nLeds; i++) {
      uint16_t b = bBaseValue << 8 | (uint16_t)pixels.loadAndScale0();
      mSPI.writeWord(b);
      uint16_t w = pixels.loadAndScale1() << 8;
      w |= pixels.loadAndScale2();
      mSPI.writeWord(w);
      pixels.advanceData();
      pixels.stepDithering();
    }
    endBoundary(nLeds);
    mSPI.waitFully();
    mSPI.release();
  }

#ifdef SUPPORT_ARGB
  virtual void show(const struct CRGB *data, int nLeds, CRGB scale) {
    PixelController<RGB_ORDER> pixels(data, nLeds,, scale, getDither());

    mSPI.select();

    startBoundary();
    for(int i = 0; i < nLeds; i++) {
      mSPI.writeByte(0xFF);
      uint8_t b = pixels.loadAndScale0(); mSPI.writeByte(b);
      b = pixels.loadAndScale1(); mSPI.writeByte(b);
      b = pixels.loadAndScale2(); mSPI.writeByte(b);
      pixels.advanceData();
      pixels.stepDithering();
    }
    endBoundary(nLeds);
    mSPI.waitFully();
    mSPI.release();
  }
#endif
};

// END APA102 Global Brightness Support
