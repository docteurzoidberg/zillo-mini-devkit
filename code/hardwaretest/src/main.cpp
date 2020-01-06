#include <Arduino.h>
#include <FastLED.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

#define MIC_SAMPLES (1024*2)
#define MIC_PIN 34

#define LED_PIN     27
#define COLOR_ORDER GRB
#define CHIPSET     WS2811
#define NUM_LEDS    64

#define BRIGHTNESS  20
#define FRAMES_PER_SECOND 60

// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
////
// This basic one-dimensional 'fire' simulation works roughly as follows:
// There's a underlying array of 'heat' cells, that model the temperature
// at each point along the line.  Every cycle through the simulation,
// four steps are performed:
//  1) All cells cool down a little bit, losing heat to the air
//  2) The heat from each cell drifts 'up' and diffuses a little
//  3) Sometimes randomly new 'sparks' of heat are added at the bottom
//  4) The heat from each cell is rendered as a color into the leds array
//     The heat-to-color mapping uses a black-body radiation approximation.
//
// Temperature is in arbitrary units from 0 (cold black) to 255 (white hot).
//
// This simulation scales it self a bit depending on NUM_LEDS; it should look
// "OK" on anywhere from 20 to 100 LEDs without too much tweaking.
//
// I recommend running this simulation at anywhere from 30-100 frames per second,
// meaning an interframe delay of about 10-35 milliseconds.
//
// Looks best on a high-density LED setup (60+ pixels/meter).
//
//
// There are two main parameters you can play with to control the look and
// feel of your fire: COOLING (used in step 1 above), and SPARKING (used
// in step 3 above).
//
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100
#define COOLING  55

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 120


bool gReverseDirection = false;

CRGB leds[NUM_LEDS];

Adafruit_MPU6050 mpu;

void Fire2012()
{
// Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
    for( int i = 0; i < NUM_LEDS; i++) {
      heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
    }

    // Step 2.  Heat from each cell drifts 'up' and diffuses a little
    for( int k= NUM_LEDS - 1; k >= 2; k--) {
      heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
    }

    // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
    if( random8() < SPARKING ) {
      int y = random8(7);
      heat[y] = qadd8( heat[y], random8(160,255) );
    }

    // Step 4.  Map from heat cells to LED colors
    for( int j = 0; j < NUM_LEDS; j++) {
      CRGB color = HeatColor( heat[j]);
      int pixelnumber;
      if( gReverseDirection ) {
        pixelnumber = (NUM_LEDS-1) - j;
      } else {
        pixelnumber = j;
      }
      leds[pixelnumber] = color;
    }
}

// measure basic properties of the input signal
// determine if analog or digital, determine range and average.
void MeasureAnalog()
{
    uint16_t signalAvg = 0, signalMax = 0, signalMin = 1024, t0 = millis();
    for (int i = 0; i < MIC_SAMPLES; i++)
    {
        adcStart(MIC_PIN);
        while(adcBusy(MIC_PIN)){
          ;
        }
        uint16_t k = adcEnd(MIC_PIN);
        signalMin = min(signalMin, k);
        signalMax = max(signalMax, k);
        signalAvg += k;
    }
    signalAvg /= MIC_SAMPLES;

    // print
    Serial.print("Time: " + String(millis() - t0));
    Serial.print(" Min: " + String(signalMin));
    Serial.print(" Max: " + String(signalMax));
    Serial.print(" Avg: " + String(signalAvg));
    Serial.print(" Span: " + String(signalMax - signalMin));
    Serial.print(", " + String(signalMax - signalAvg));
    Serial.print(", " + String(signalAvg - signalMin));
    Serial.println("");
}

void MeasureMPU() {
  /* Take a new reading */
  mpu.read();

  /* Get new sensor events with the readings */
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  /* Print out the values */
  Serial.print(a.acceleration.x);
  Serial.print(",");
  Serial.print(a.acceleration.y);
  Serial.print(",");
  Serial.print(a.acceleration.z);
  Serial.print(", ");
  Serial.print(g.gyro.x);
  Serial.print(",");
  Serial.print(g.gyro.y);
  Serial.print(",");
  Serial.print(g.gyro.z);
  Serial.println("");
}

void setup() {

  //analogSetAttenuation(ADC_2_5db);
  adcAttachPin(MIC_PIN);
  analogSetPinAttenuation(MIC_PIN, ADC_2_5db);

  Serial.begin(115200);
  while (!Serial) {
     delay(10); // wait for serial port to connect. Needed for native USB port only
  }

  // Try to initialize!
  if (!mpu.begin()) {
    Serial.println("Failed to find MPU6050 chip");
    while (1) {
      delay(10);
    }
  }

  mpu.setAccelerometerRange(MPU6050_RANGE_16_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
  FastLED.setBrightness(BRIGHTNESS);

  Serial.println("");
  delay(100);
}

uint16_t t;

void loop() {

  // put your main code here, to run repeatedly:
  t=millis();
  MeasureAnalog();
  MeasureMPU();
  Fire2012(); // run simulation frame
  t=millis()-t;
  FastLED.show(); // display this frame
  FastLED.delay((1000/ FRAMES_PER_SECOND)-t);
}