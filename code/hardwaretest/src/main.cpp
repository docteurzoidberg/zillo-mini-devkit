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

#define BRIGHTNESS  100
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


/*************************************************
 * Note frequency array
 *************************************************/

unsigned int pt_notes[] = {
      0,    0,    0,    0,    0,    0,    0 ,   0,    0,    0,    0,    0,            //  0- 11  undefined
	   31,   35,   37,   39,   41,   44,   46,   49,   52,   55,   58,   62,            // 12- 23  C1 C#1 D1 D#1 E1 F1 F#1 G1 G#1 A1 A#1 B1
	   65,   69,   73,   78,   82,   87,   93,   98,  104,  110,  117,  123,            // 24- 35  C2 to B2
	  131,  139,  147,  156,  165,  175,  185,  196,  208,  220,  233,  247,            // 36- 47  C3 to B3
	  262,  277,  294,  311,  330,  349,  370,  392,  415,  440,  466,  494,            // 48- 59  C4 to B4 (middle C)
	  523,  554,  587,  622,  659,  698,  740,  784,  831,  880,  932,  988,            // 60- 71  C5 to B5
   1047, 1109, 1175, 1245, 1319, 1397, 1480, 1568, 1661, 1760, 1865, 1976,            // 72- 83  C6 to B6
   2093, 2217, 2349, 2489, 2637, 2794, 2960, 3136, 3322, 3520, 3729, 3951,            // 84- 95  C7 to B7
   4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902             // 96-107  C8 to B8
};



/*
***************************************************************************
  Arduino Melody Player by ericBcreator
  loosely based on the old qbasic PLAY command
  with lyrics and ESP boards support
***************************************************************************
  version 0.95 - last update 20191229 by ericBcreator
***************************************************************************
  Features:
***************************************************************************
  Syntax:
    playMelody(string);

    commands:
    CDEFGABn  notes, n = note length: 1, 2, 4, 8, 16, 32, 64,
              2, 4, 8, 16, 32 dotted (or 3, 6, 12, 24, 48),
              4, 8, 16, 32 triplet 34, 38, 316, 332
              defaults to Ln if n is omitted
    Rn        rest, n = note length. defaults to Ln if n is omitted

    Tn        tempo n = 32-255 (default is 120 BPM, 4/4 time base)
    On        octave n = 1-8 (default is 4)
    Ln        n = default note length (default is 4, quarter note)
    MN        music normal:      7/8 note length duration (default)
    ML        or music legato:   full length duration
    MS        or music staccato: 3/4 note length duration
    MU        mute mode

    <         octave down (range 1-8)
    >         octave up (range 1-8)
    [         transpose down a step (range -12 to 12)
    ]         transpose up a step (range -12 to 12)
    #         sharp (place after note, D#n)
    -         flat (place after note, D-n)
    .         dotted note (note length will be increased with 50%)

    $         reset the default settings. recommended at the start
              of a new song

              note: the Tn, On, Ln, M? and [ ] settings are static
                    so will remain in memory with the next call
                    to playtune. It is recommended to reset these
                    at the beginning of a new melody with the $
                    command
              note: spaces are ignored, use them for readability

	  example the Big Ben tune:
              playMelody("$T120 L4 O5 ECD<GR2 G>DEC R2");
    or:       melody = "$T120 L4 O5 ECD<GR2 G>DEC R2";
              playMelody(melody);

    lyric support:
		          The global String 'amp_lyrics' can be used to display
              lyrics while playing melodies. The code will display words
              and syllables synchronized to the order of the notes, so
              word 1 for note 1, word 2 for note 2, etc.
              Words can be broken into syllables by adding a - (hyphen).
              Words have to be separated with space.
              A double space will be treated as an empty word.
              Lyrics are not played with rests.

	  example:
			        lyrics = "Jin-gle bells Jin-gle bells Jin-gle all the way";
              playMelody("$T240 L4 O4 EEE2 EEE2 EGC.D8 E1");
***************************************************************************
  Parts:
    - Arduino / ESP board (I used a Wemos D1 mini)
    - LED matrices
    - small speaker
    - button or rotary encoder
***************************************************************************
  Todo list:
    - cleanup
	  - check @EB-todo
    - callback functions
    - delay and check loop
    - timing accuracy (check)
    - add double tempo?
	  - amp_noteLengthToMS: use array, only calc after tempo change?
***************************************************************************
  This code is free for personal use, not for commercial purposes.
  Please leave this header intact.

  contact: ericBcreator@gmail.com
***************************************************************************
*/


//#define DEBUG                                         // enable debug messages
//#define DEBUG_PLAY_CMD                                // print play commands
//#define DEBUG_SILENCE

#ifdef DEBUG
  #define DEBUGPRINT(x)   Serial.print(x)
  #define DEBUGPRINTLN(x) Serial.println(x)
#else
  #define DEBUGPRINT(x)
  #define DEBUGPRINTLN(x)
#endif
                                         // ESP pins
#define AMP_SPEAKER_PIN 25               // @EB-setup the pin of the buzzer of speaker

byte amp_lyricsMode = 0;                                // @EB-setup 0: no lyrics, 1: notes, 2: lyrics (or notes if no lyrics are set), 3: lalala ;-)
byte amp_buttonPressed = 0;
String amp_lyrics = "";
String melody = "";

int melodyNum = 0;
bool musicOff = false;
byte loopMode = 0;                                      // @EB-setup 0: loop all songs, 1: loop one song, 2: play one song, don't loop

String fillSpace(int value, int valLength) {
  String tmpStr = String(value);
  while (tmpStr.length() < valLength)
    tmpStr = " " + tmpStr;
  return String(tmpStr);
}

bool delayAndCheckEnc(unsigned int delayTime) {
  unsigned long startTime = millis();
  unsigned long checkTime = millis();
  while ((millis() - startTime) < delayTime) {
    if (millis() - checkTime > 50) {
      checkTime = millis();
    } else
      delay(1);
  }
  return false;
}


//////////////////////////////////////////////////////////////////////
/// amp functions
//////////////////////////////////////////////////////////////////////

bool amp_isNum(byte value) {
  if (value >= 48 && value <= 57)                   // 0 to 9
    return true;
  else
    return false;
}

String amp_noteName(byte note, int sharpFlat) {
  switch (note) {
    case  0:  return "C ";  break;
    case  1:  return (sharpFlat == -1 ? "D-" : "C#");  break;
    case  2:  return "D ";  break;
    case  3:  return (sharpFlat == -1 ? "E-" : "D#");  break;
    case  4:  return "E ";  break;
    case  5:  return "F ";  break;
    case  6:  return (sharpFlat == -1 ? "G-" : "F#");  break;
    case  7:  return "G ";  break;
    case  8:  return (sharpFlat == -1 ? "A-" : "G#");  break;
    case  9:  return "A ";  break;
    case 10:  return (sharpFlat == -1 ? "B-" : "A#");  break;
    case 11:  return "B ";  break;
  }
}

double amp_noteLengthToMS(unsigned int curNoteLength, byte tempo) {
  double timeBase = 60000 / tempo;        // @EB-todo default 4/4

  switch (curNoteLength) {
    case  1:  return (timeBase * 4);                // whole note
    case  2:  return (timeBase * 2);                // half
    case  4:  return (timeBase);                    // quarter
    case  8:  return (timeBase / 2);                // 8th
    case 16:  return (timeBase / 4);                // 16th
    case 32:  return (timeBase / 8);                // 32nd
    case 64:  return (timeBase / 16);               // 64th

    case  3:  return (timeBase * 3);                // dotted half
    case  6:  return (timeBase * 3 / 2);            // dotted 4th
    case 12:  return (timeBase * 3 / 4);            // dotted 8th
    case 24:  return (timeBase * 3 / 8);            // dotted 16th
    case 48:  return (timeBase * 3 / 16);           // dotted 32th

    case 34:  return (timeBase / 1.5);              // triplet quarter
    case 38:  return (timeBase / 3);                // triplet 8th
    case 316: return (timeBase / 6);                // triplet 16th
    case 332: return (timeBase / 12);               // triplet 32th
  }
}

void amp_playNote(byte note, int duration, String curNoteName) {

  #ifndef DEBUG_SILENCE
    if (note >= 0 && note <= 107) {
      if (pt_notes[note]) {
        #ifdef ESP32
          ledcWriteTone(0, pt_notes[note]);
        #else
          tone(AMP_SPEAKER_PIN, pt_notes[note]);
        #endif
      } else {
        #ifdef ESP32
          ledcWriteTone(0, 0);
          ledcWrite(0, LOW);
        #else
          noTone(AMP_SPEAKER_PIN);
        #endif
      }
    }
  #endif

//  delay(duration);

  delayAndCheckEnc(duration);             // @EB-todo
}

void amp_playMelodyEnd() {
  amp_playNote(0, 0, "");
  pinMode(AMP_SPEAKER_PIN, INPUT);                 // make sure the buzzer is silent ;-)
}

void playMelody (String melody) {
  static byte tempo = 120;                          // default to 120 BPM
  static byte octave = 4;                           // default octave
  static unsigned int defaultNoteLength = 4;        // default to quarter note
  static unsigned int noteLengthType = 0;           // 0: normal 7/8, 1: legato 1/1, 2: staccato 3/4, 9: mute (default 0)
  static int transpose = 0;                         // default to 0

  byte notes[] = { 9, 11, 0, 2, 4, 5, 7 };

  byte curFunc = 0;
  byte curChar = 0;
  unsigned int curVal = 0;
  bool skip = false;

  unsigned int curNoteLength = 0;
  int sharpFlat = 0;
  bool dotted = false;
  int octaveOffset = 0;

  int curNote = 0;
  double curNoteLengthMS = 0;
  String curNoteName;

  ledcAttachPin(AMP_SPEAKER_PIN, 0);

  for (unsigned int i = 0; i < melody.length(); i++) {

    curNoteLength = 0;
    curVal = 0;
    skip = true;
    octaveOffset = 0;

    curChar = melody[i];

    if (curChar >= 97 && curChar <= 122) {          // convert lowercase to uppercase
      curChar -= 32;
    }

    switch (curChar) {
      case 32:                                      // skip spaces
        break;

      case '$':                                     // restore default settings
        tempo = 120;
        octave = 4;
        defaultNoteLength = 4;
        noteLengthType = 0;
        transpose = 0;

        #ifdef DEBUG_PLAY_CMD
          DEBUGPRINTLN("Reset settings");
        #endif
        break;

      case '<':                                     // < sign, octave lower
        octave--;
        if (octave < 1) octave = 1;

        #ifdef DEBUG_PLAY_CMD
          DEBUGPRINTLN("Octave set to " + (String) octave);
        #endif
        break;

      case '>':                                     // > sign, octave higher
        octave++;
        if (octave > 8) octave = 8;

        #ifdef DEBUG_PLAY_CMD
          DEBUGPRINTLN("Octave set to " + (String) octave);
        #endif
        break;

      case '[':                                     // [ transpose down
        transpose--;
        if (transpose < -12) transpose = -12;

        #ifdef DEBUG_PLAY_CMD
          DEBUGPRINTLN("Transpose down " + (String) transpose);
        #endif
        break;

      case ']':                                     // ] transpose up
        transpose++;
        if (transpose > 12) transpose = 12;

        #ifdef DEBUG_PLAY_CMD
          DEBUGPRINTLN("Transpose up " + (String) transpose);
        #endif
        break;

      case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'R': case '#': case '-':
                                                    // A B C D E F G notes and R rest and # sharp and - flat
        skip = false;

        if (i + 1 < melody.length()) {
          if (amp_isNum(melody[i + 1])) {
            skip = true;
          }
          if (melody[i + 1] == '#') {               // sharp
            sharpFlat = 1;
            skip = true;
          } else if (melody[i + 1] == '-') {        // flat
            sharpFlat = -1;
            skip = true;
          } else if (melody[i + 1] == '.') {        // dotted
            dotted = true;
            i++;
          }
        }

        if (curChar != '#' && curChar != '-') {
          curFunc = curChar;
        }
        break;

      case 'M':                                     // music length type
        if (i + 1 < melody.length()) {
          switch  (melody[i + 1]) {
            case 'N': case 'n':
              noteLengthType = 0;                   // normal 7/8
              i++;
              break;
            case 'L': case 'l':
              noteLengthType = 1;                   // legato 1/1
              i++;
              break;
            case 'S': case 's':
              noteLengthType = 2;                   // staccato 3/4
              i++;
              break;
            case 'U': case 'u':
              noteLengthType = 9;                   // mute
              i++;
              break;
          }
        }

      case 'L':                                     // L default note/rest length
      case 'O':                                     // O octave
      case 'T':                                     // T tempo
        curFunc = curChar;
        break;

      default:
        if (amp_isNum(curChar)) {
          curVal = curChar - 48;

          for (int j = 0; j <= 2; j++) {             // look ahead to get the next 2 numbers or dot
            if (i + 1 < melody.length()) {
              if (amp_isNum(melody[i + 1])) {
                curVal = curVal * 10 + melody[i + 1] - 48;
                i++;
              } else if (melody[i + 1] == '.') {
                dotted = true;
                break;
              } else {
                break;
              }
            }
          }

          curNoteLength= curVal;
          skip = false;
        }
    }

    if (curFunc > 0 && !skip) {
      #ifdef DEBUG_PLAY_CMD
        DEBUGPRINT("Command " + (String) curFunc + " value " + fillSpace(curVal, 3));
      #endif

      if (curFunc >= 65 and curFunc <= 71 || curFunc == 82) {
        if (!curNoteLength) {
          curNoteLength = defaultNoteLength;
        }

        if (dotted) {
          curNoteLength = curNoteLength * 1.5;
        }

        curNoteLengthMS = amp_noteLengthToMS(curNoteLength, tempo);

        if (curFunc == 82) {
          curNote = 0;
          curNoteName = "";

          #ifdef DEBUG_PLAY_CMD
            DEBUGPRINT(" Pause length "+ fillSpace(curNoteLength, 3) + " " + fillSpace(curNoteLengthMS, 4) + " ms");
          #endif

        } else {
          curNote = notes[curFunc - 65];
          curNote = curNote + transpose + sharpFlat;

          while (curNote < 0) {
            curNote += 12;
            if (octave > 1) octaveOffset -= 1;
          }
          while (curNote > 11) {
            curNote -= 12;
            if (octave < 8) octaveOffset += 1;
          }

          curNoteName = amp_noteName(curNote, sharpFlat);
          #ifdef DEBUG_PLAY_CMD
            DEBUGPRINT(" Transpose " + (String) transpose + " Octave " + (String) (octave + octaveOffset) + " Note " + curNoteName);
          #endif

          curNote = ((octave + octaveOffset) * 12) + curNote;
          #ifdef DEBUG_PLAY_CMD
            DEBUGPRINT(" Notenumber " + fillSpace(curNote, 3) + " Frequency " + fillSpace(pt_notes[curNote], 4) + " length "+ fillSpace(curNoteLength, 3) + " " + fillSpace(curNoteLengthMS, 4) + " ms");
          #endif

          curNoteName += (String) (octave + octaveOffset);  // @EB-todo
        }

        switch (noteLengthType) {
          case 0: // normal 7/8
            amp_playNote(curNote, (curNoteLengthMS / 8 * 7), curNoteName);
            amp_playNote(0, (curNoteLengthMS / 8 * 1), "");
            break;

          case 1: // legato 1/1
            amp_playNote(curNote, curNoteLengthMS, curNoteName);
            break;

          case 2: // staccato
            amp_playNote(curNote, (curNoteLengthMS / 4 * 3), curNoteName);
            amp_playNote(0, (curNoteLengthMS / 4 * 1), "");
            break;

          case 9: // mute
            amp_playNote(0, curNoteLengthMS, curNoteName);
            #ifdef DEBUG_PLAY_CMD
              DEBUGPRINT(" MUTE");
            #endif
            break;
        }

        dotted = false;
        curNoteLength = 0;
        sharpFlat = 0;

      } else {
        switch (curFunc) {
          case 'L':
            switch (curVal) {
              case 1: case 2: case 3: case 4: case 6: case 8: case 12: case 16: case 24: case 32: case 48: case 64:
              case 34: case 38: case 316: case 332:
                defaultNoteLength = curVal;
                break;
            }
            break;

          case 'O':
            octave = constrain(curVal, 1, 8);
            break;

          case 'T':
            tempo = constrain(curVal, 32, 255);
            #ifdef DEBUG_PLAY_CMD
              DEBUGPRINTLN(" Tempo " + (String) tempo);
            #endif
            break;
        }
      }

      curFunc = 0;
      curNoteName = "";
    }

    if (!skip) {
      #ifdef DEBUG_PLAY_CMD
        DEBUGPRINTLN("");
      #endif
    }
  }

  amp_playMelodyEnd();
}

void playSharpFlat() {
  playMelody("$T120 L8 O4");
  playMelody("cc#dd#eff#gg#aa#b>c4 r4");
  playMelody("c<bb-aa-gg-fee-dd-c4");
}

void playToneScale(byte oneOctave) {
  int startOctave = 1;
  int endOctave = 8;

  if (oneOctave) {
    startOctave = oneOctave;
    endOctave = oneOctave;
  }

  playMelody("$T120 L8 O" + (String) startOctave);

  for (int i = startOctave; i <= endOctave; i++) {
    playMelody("cc#dd#eff#gg#aa#b >");
    if (amp_buttonPressed)
      break;
  }

  if (oneOctave)
    playMelody("c");
}

void playTimingTest() {
  playMelody("$T120 L8 O4");
  while (1) {
    playMelody("GRCRCRCR");
    if (amp_buttonPressed)
      break;
  }
}

void playTuneStarWars() {
  String tune;
  tune = "$T105 L8 O4 D38D38D38";
  tune += "G2>D2 C38<B38A38>G2D4 C38<B38A38>G2D4 C38<B38>C38<A2D34D38";
  tune += "G2>D2 C38<B38A38>G2D4 C38<B38A38>G2D4 C38<B38>C38<A2D34D38";
  tune += "E4.E>C<BAG G38A38B38A34E38F#4D34D38 E4.E>C<BAG>D.<A16A2D34D38";
  tune += "E4.E>C<BAG G38A38B38A34E38F#4>D34D38 G.F16E-.D16C.<B-16A.G16>D2.<D38D38D38";
  tune += "G2>D2 C38<B38A38>G2D4 C38<B38A38>G2D4 C38<B38>C38<A2D34D38";
  tune += "G2>D2 C38<B38A38>G2D4 G38F38E-38B-2A4 GR<G38G38G38G4R4";
  playMelody(tune);
}

void playTuneFireStone() {
  playMelody("$T113 L8 O5  D16F#ED");
  String tune = "D.<B.B>D<B>E16F#8. ER16E.<A.A16>A16<A16>F#E  D.<B.B>D<B>E16F#8.  ER16E.DR16D16F#ED  D.<B.B>D<B>E16F#8. ER16E.<A.A16>A16<A16>F#E  D.D.DDDE16F#8. D.D.D16A16D16F#ED16E16D";
  for (int i = 0; i < 2; i++) {
    playMelody(tune);
    if (amp_buttonPressed)
      break;
  }
}

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

void playmelodyloop() {

    DEBUGPRINTLN("Melody " + (String) melodyNum);

    switch (melodyNum) {
      case 0:
        playToneScale(4);
        break;

      case 1:
        playTuneFireStone();
        break;

      case 2:
        playMelody("$T133 L8 O4 [ G4G4GF#GF#4 D4DDEF#A4 E4EF#E#F#E <B>F#F#F#4EED  G4G4GF#GF#4 D4DDEF#A4 E4EF#E#F#E <B>F#F#F#4EED");
        break;

      case 3:
        playMelody("$T116 L8 O4");
        melody = "GAGD FRFRGAGD FRFRGAGD FRFRR2 R2";
        playMelody(melody);
        playMelody(melody);
        break;

      case 4:
        playMelody("$T130 L8 O4 ]] C<B>CA4. R4C<B>CG4. R4C<B>CG4. R4C<B>C>C R<AR4  C<B>CA4. R4C<B>CG4. R4C<B>CG4. R4C<B>C>C R<AR4");
        break;

      case 5:
        playTuneStarWars();
        break;

      case 6:
        playMelody("$T120 L4 O4 ML ECD<GR2 G>DECR2");
        break;

      case 7:
        playMelody("$T128 L8 O5");
        melody = "E-16-E-E-16E-E-16E-E-16E-E-E- E-4R4CCDD E-4GGF4R A-4A-GFE-FR4";
        playMelody(melody);
        playMelody(melody);
        break;

      case 8:
        playMelody("$T240 L4 O4");
        melody = "EEE2 EEE2 EGC.D8 E1 FFF.F8 FEEE8E8 EDDE D2G2";
        melody += "EEE2 EEE2 EGC.D8 E1 FFF.F8 FEEE8E8 GGFD C1";
        playMelody(melody);
        break;

      case 9:
        playMelody("$T124 L8 O4");
        melody = "E>EDC#<B  B2.A>C#2R EDC#<B  B2.A>C#2R EDC#<B  B4RB.R16RB4  R4 EE>EDC#<B  B4RB.R16RB4";
        melody += "R2>EDC#<B  B2.A>C#2R EDC#<B  B2.A>C#2R EDC#<B  B4RB.R16RB4  R EEE>EDC#<B  B4RB.R16RB4.A4";
        playMelody(melody);
        break;

      case 10:
//        displayMessage("Timing test 120 BPM");
//        playTimingTest();
        break;

      default:
        melodyNum = 0; // 1 gets added in loop so the note scale (melody 0) get skipped. set to -1 to not skip
    }


    amp_lyrics = "";

    delayAndCheckEnc(2000);

    if (loopMode == 0)
      melodyNum++;
}

void melodyTask( void * pvParameters ){
    //String taskMessage = "Task running on core ";
    //taskMessage = taskMessage + xPortGetCoreID();
    //Serial.println(taskMessage);
    while(true){
        playmelodyloop();
    }
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
  playMelody("$T120 L16 O4 CEG>C8");
  static int taskCore = 0;
  xTaskCreatePinnedToCore(
                    melodyTask,   /* Function to implement the task */
                    "melody",     /* Name of the task */
                    4096,        /* Stack size in words */
                    NULL,         /* Task input parameter */
                    0,            /* Priority of the task */
                    NULL,         /* Task handle. */
                    taskCore);    /* Core where the task should run */
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
  //FastLED.delay((1000/ FRAMES_PER_SECOND)-t);
}