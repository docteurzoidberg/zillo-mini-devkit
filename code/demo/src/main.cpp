
#define mw 8
#define mh 8
#define NUMMATRIX (mw*mh)
#define LEDPIN 27

#include <Adafruit_GFX.h>
#include <FastLED.h>
#include <FastLED_NeoMatrix.h>
#include <SPI.h>

CRGB matrixleds[NUMMATRIX];

FastLED_NeoMatrix *matrix = new FastLED_NeoMatrix(matrixleds, 8, mh, mw/8, 1,
  NEO_MATRIX_TOP     + NEO_MATRIX_LEFT +
    NEO_MATRIX_COLUMNS + NEO_MATRIX_ZIGZAG +
    NEO_TILE_TOP + NEO_TILE_LEFT +  NEO_TILE_PROGRESSIVE);

const uint16_t colors[] = {
  matrix->Color(255, 0, 0), matrix->Color(0, 255, 0), matrix->Color(0, 0, 255) };

void setup() {
    Serial.begin(115200);
    FastLED.addLeds<NEOPIXEL,LEDPIN>(matrixleds, NUMMATRIX);
    matrix->begin();
    matrix->setTextWrap(false);
    matrix->setBrightness(40);
    matrix->setTextColor(colors[0]);
}

int x    = mw;
int pass = 0;
unsigned long lastshow=0;
int demo=0;
int demo0count=0;
int demo1count=0;

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
    return matrix->Color(255 - WheelPos * 3, 0, WheelPos * 3);
  }
  if(WheelPos < 170) {
    WheelPos -= 85;
    return matrix->Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
  WheelPos -= 170;
  return matrix->Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void loop() {

  //texte qui scroll
  if((demo==0) && (millis()-lastshow>100)){
    matrix->fillScreen(0);
    matrix->setCursor(x, 0);
    matrix->setTextColor(Wheel(((x * 256 / 32) + x) & 255));
    matrix->print(F("Zilloscope"));
    if(--x < -8-10) {
      x = matrix->width();
      if(++pass >= 1) {
        pass = 0;
        demo++;
        lastshow=0;
      }
      //matrix->setTextColor(colors[pass]);
    }
    matrix->show();
    //delay(100);
    lastshow=millis();
  }

  if(demo>1){
    demo=0;
  }
}
