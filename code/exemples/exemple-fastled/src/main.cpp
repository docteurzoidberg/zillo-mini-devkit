#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <FastLED.h>

#define LED_PIN     27
#define COLOR_ORDER GRB
#define CHIPSET     WS2812B
#define NUM_LEDS    3

#define BRIGHTNESS  50
#define FRAMES_PER_SECOND 60

CRGB leds[NUM_LEDS];

CRGB color;
void setup() {
  delay(3000); // sanity delay
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness( BRIGHTNESS );
  color.r=255;
  color.g=255;
  color.b=255;
}

void loop()
{
  leds[0] = color;
  FastLED.show(); // display this frame
  FastLED.delay(1000 / FRAMES_PER_SECOND);
}
