#include "Adafruit_WS2801.h"
#include "SPI.h"

#define LED_TOTAL   50
#define LED_HALF    LED_TOTAL / 2
#define NUMBER_OF_VISUALS   4   //Change this accordingly if you add/remove a visual in the switch-case in Visualize()

#define MSGEQ7_DCOUT    A0
#define MSGEQ7_RESET    6
#define MSGEQ7_STROBE   10

#define BUTTON_PIN      9

Adafruit_WS2801 strand = Adafruit_WS2801(LED_TOTAL);

uint16_t gradient = 0; //Used to iterate and loop through each color palette gradually

//IMPORTANT:
//  This array holds the "threshold" of each color function (i.e. the largest number they take before repeating).
//  The values are in the same order as in ColorPalette( )'s switch case (Rainbow() is first, etc). This is simply to
//  keep "gradient" from overflowing, the color functions themselves can take any positive value. For example, the
//  largest value Rainbow() takes before looping is 1529, so "gradient" should reset after 1529, as listed.
//  Make sure you add/remove values accordingly if you add/remove a color function in the switch-case in ColorPalette().
uint16_t thresholds[] = {1529, 1019, 764, 764, 764, 1274};

int bandPeaks[7] = {0, 0, 0, 0, 0, 0, 0};
float bandsAverage[7] = {0, 0, 0, 0, 0, 0, 0};
long noiseCutoffs[7] = {200, 220, 230, 215, 220, 230, 230}; //Max noise levels for each band: 63Hz, 160Hz, 400Hz, 1000Hz, 2500Hz, 6250Hz, 16000Hz

uint8_t palette = 0;    //Holds the current color palette.
uint8_t visual = 0;     //Holds the current visual being displayed.

uint8_t volume = 0;     //Holds the volume level read from the sound detector.
uint8_t lastVolume = 0; //Holds the value of volume from the previous loop() pass.
float maxVol = 15;      //Holds the largest volume recorded thus far to proportionally adjust the visual's responsiveness.
float avgBump = 0;      //Holds the "average" volume-change to trigger a "bump."
float avgVol = 0;       //Holds the "average" volume-level to proportionally adjust the visual experience.

float brightnessKnob = 20.0;    //Holds the percentage of how twisted the trimpot is. Used for adjusting the max brightness.
float shuffleTime = 0;  //Holds how many seconds of runtime ago the last shuffle was (if shuffle mode is on).

////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//NOTE: The reason "average" is quoted is because it is not a true mathematical average. This is because I have
//      found what I call a "sequenced average" is more successful in execution than a real average. The difference
//      is that the sequenced average doesn't use the pool of all values recorded thus far, but rather averages the
//      last average and the current value received (in sequence). Concretely:
//
//          True average: (1 + 2 + 3) / 3 = 2
//          Sequenced: (1 + 2) / 2 = 1.5 --> (1.5 + 3) / 2 = 2.25  (if 1, 2, 3 was the order the values were received)
//
//      All "averages" in the program operate this way. The difference is subtle, but the reason is that sequenced
//      averages are more adaptive to changes in the overall volume. In other words, if you went from loud to quiet,
//      the sequenced average is more likely to show an accurate and proportional adjustment more fluently.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool debugging = false;
bool shuffle = true;    //Toggles shuffle mode.
bool bump = false;      //Used to pass if there was a "bump" in volume

//For Traffic() visual
int8_t pos[LED_TOTAL] = { -2};     //Stores a population of color "dots" to iterate across the LED strand.
uint8_t rgb[LED_TOTAL][3] = {0};    //Stores each dot's specific RGB values.

//For Snake() visual
bool left = false;  //Determines the direction of iteration. Recycled in PaletteDance()
int8_t dotPos = 0;  //Holds which LED in the strand the dot is positioned at. Recycled in most other visuals.
float timeBump = 0; //Holds the time (in runtime seconds) the last "bump" occurred.
float avgTime = 0;  //Holds the "average" amount of time between each "bump" (used for pacing the dot's movement).
int spectrumValue[7];

void setup() {
  Serial.begin(115200);
  strand.begin();
  strand.show();

  pinMode(MSGEQ7_RESET, OUTPUT);
  pinMode(MSGEQ7_STROBE, OUTPUT);
  // SETUP
  digitalWrite(MSGEQ7_RESET, LOW);
  digitalWrite(MSGEQ7_STROBE, HIGH);
  // RESET
  digitalWrite(MSGEQ7_RESET, HIGH);
  delayMicroseconds(1);
  digitalWrite(MSGEQ7_RESET, LOW);
}

unsigned long lastPowerOfAverageCalculation = millis();

int NormalizeBandPeakValue(long value, long bandIndex) {
  long timePassed = millis() - lastPowerOfAverageCalculation;
  float powerOfAverage = timePassed;
  bandsAverage[bandIndex] = bandsAverage[bandIndex] / powerOfAverage * (powerOfAverage - 1) + value / powerOfAverage;
  if (value < bandsAverage[bandIndex] / 2.0) {
    return 0;
  }
  value = max(value - noiseCutoffs[bandIndex], 0);
  return value;
}

void ReadBandsPeaksOnInterval(int interval) {
  unsigned long stopTime = millis() + interval;
  
  
  while (millis() < stopTime) {
    for (int i = 0; i < 7; i++) {
      digitalWrite(MSGEQ7_STROBE, HIGH);
//      delayMicroseconds(5);
      digitalWrite(MSGEQ7_STROBE, LOW);
      delayMicroseconds(37);
      int bandPeak = analogRead(MSGEQ7_DCOUT);
      if (bandPeak > bandPeaks[i]) {
        bandPeaks[i] = bandPeak;
      }
    }
  }
  for (byte i = 0; i < 7; i++) {
    bandPeaks[i] = NormalizeBandPeakValue(bandPeaks[i], i);
  }
}

float GetBassPeak() {
//  return max(bandPeaks[0], bandPeaks[1]);
  return (bandPeaks[0] + bandPeaks[1]) / 2.0;
}

float GetHighPeak() {
  return (bandPeaks[5] + bandPeaks[6]) / 2.0;
}

float GetMiddlePeak() {
  return max(max(bandPeaks[2], bandPeaks[3]), bandPeaks[4]);
}

float GetVolume() {
  ReadBandsPeaksOnInterval(30);
  float bassPeak = GetBassPeak();
  float middlePeak = GetMiddlePeak();
  float highPeak = GetHighPeak();
//  Serial.print(bassPeak);
//  Serial.print(" ");
//  Serial.print(middlePeak);
//  Serial.print(" ");
//  Serial.print(highPeak);
//  Serial.print(" ");
  return (bassPeak + middlePeak + highPeak) / 3.0;
}

void Debug(bool forceDebugMessage = false) {
  if (debugging || forceDebugMessage) {
    Serial.print(bandsAverage[6]);
    Serial.print(" ");
    Serial.println(volume);
  }
}

void loop() {
  volume = GetVolume();
//  Serial.println(volume);

  //Sets a threshold for volume.
  //In practice I've found noise can get up to 15, so if it's lower, the visual thinks it's silent.
  //Also if the volume is less than average volume / 2 (essentially an average with 0), it's considered silent.

  Debug();
  if (volume < avgVol / 2.0 || volume < 15) {
    volume = 0;
  } else {
    avgVol = (avgVol + volume) / 2.0; //If non-zeo, take an "average" of volumes.
  }

  if (volume > maxVol) {
    maxVol = volume;
  }

  CyclePalette();   //Changes palette for shuffle mode or button press.
  CycleVisual();    //Changes visualization for shuffle mode or button press.
  ToggleShuffle();  //Toggles shuffle mode. Delete this if you didn't use buttons.

  //This is where "gradient" is modulated to prevent overflow.
  if (gradient > thresholds[palette]) {
    gradient %= thresholds[palette] + 1;

    //Everytime a palette gets completed is a good time to readjust "maxVol," just in case
    //the song gets quieter; we also don't want to lose brightness intensity permanently
    //because of one stray loud sound.
    maxVol = (maxVol + volume) / 2.0;
  }

  //If there is a decent change in volume since the last pass, average it into "avgBump"
  if (volume - lastVolume > 10) {
    avgBump = (avgBump + (volume - lastVolume)) / 2.0;
  }

  //If there is a notable change in volume, trigger a "bump"
  //avgbump is lowered just a little for comparing to make the visual slightly more sensitive to a beat.
  bump = (volume - lastVolume > avgBump * .9);

  //If a "bump" is triggered, average the time between bumps
  if (bump) {
    avgTime = (((millis() / 1000.0) - timeBump) + avgTime) / 2.0;
    timeBump = millis() / 1000.0;
  }

  Visualize();          //Calls the appropriate visualization to be displayed with the globals as they are.
  gradient++;           //Increments gradient
  lastVolume = volume;  //Records current volume for next pass
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//NOTE: All of these visualizations feature some aspect that affects brightness based on the volume relative to
//      maxVol, so that louder = brighter. Initially, I did simple proportions (volume/maxvol), but I found this
//      to be visually indistinct. I then tried an exponential method (raising the value to the power of
//      volume/maxvol). While this was more visually satisfying, I've opted for a balance between the two. You'll
//      notice something like pow(volume/maxVol, 2.0) in the functions below. This simply squares the ratio of
//      volume to maxVol to get a more exponential curve, but not as exaggerated as an actual exponential curve.
//      In essence, this makes louder volumes brighter, and lower volumes dimmer, to be more visually distinct.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Visualize() {
  switch (visual) {
    case 0: return Pulse();
    case 1: return PalettePulse();
    case 2: return Traffic();
    case 3: return PaletteDance();
    case 4: return Paintball();
    default: return Pulse();
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//NOTE: The strand displays RGB values as a 32 bit unsigned integer (uint32_t), which is why ColorPalette()
//      and all associated color functions' return types are uint32_t. This value is a composite of 3
//      unsigned 8bit integer (uint8_t) values (0-255 for each of red, blue, and green). You'll notice the
//      function Split() (listed below) is used to dissect these 8bit values from the 32-bit color value.
//////////////////////////////////////////////////////////////////////////////////////////////////////////


//This function calls the appropriate color palette based on "palette"
//  If a negative value is passed, returns the appropriate palette with "gradient" passed.
//  Otherwise returns the color palette with the passed value (useful for fitting a whole palette on the strand).
uint32_t ColorPalette(float num) {
  switch (palette) {
    case 0: return (num < 0) ? Rainbow(gradient) : Rainbow(num);
    case 1: return (num < 0) ? Sunset(gradient) : Sunset(num);
    case 2: return (num < 0) ? Ocean(gradient) : Ocean(num);
    case 3: return (num < 0) ? PinaColada(gradient) : PinaColada(num);
    case 4: return (num < 0) ? Sulfur(gradient) : Sulfur(num);
    case 5: return (num < 0) ? NoGreen(gradient) : NoGreen(num);
    default: return Rainbow(gradient);
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//DEBUG CYCLE
//No reaction to sound, merely to see gradient progression of color palettes
//NOT implemented in code as is, but is easily includable in the switch-case.
void Cycle() {
  for (int i = 0; i < strand.numPixels(); i++) {
    float val = float(thresholds[palette]) * (float(i) / float(strand.numPixels())) + (gradient);
    val = int(val) % thresholds[palette];
    strand.setPixelColor(i, ColorPalette(val));
  }
  strand.show();
  gradient += 32;
}
/////////////////////////////////////////////////////////////////////////////////////////////////////


//////////<Helper Functions>

void CyclePalette() {
  //IMPORTANT: Delete this whole if-block if you didn't use buttons//////////////////////////////////
  //If a button is pushed, it sends a "false" reading
  //  if (signalToCyclePaletteReceived) {
  //
  //    palette++;     //This is this button's purpose, to change the color palette.
  //
  //    //If palette is larger than the population of thresholds[], start back at 0
  //    //  This is why it's important you add a threshold to the array if you add a
  //    //  palette, or the program will cylce back to Rainbow() before reaching it.
  //    if (palette >= sizeof(thresholds) / 2) {
  //      palette = 0;
  //    }
  //
  //    gradient %= thresholds[palette]; //Modulate gradient to prevent any overflow that may occur.
  //
  //    //The button is close to the microphone on my setup, so the sound of pushing it is
  //    //  relatively loud to the sound detector. This causes the visual to think a loud noise
  //    //  happened, so the delay simply allows the sound of the button to pass unabated.
  //    delay(350);
  //
  //    maxVol = avgVol;  //Set max volume to average for a fresh experience.
  //  }
  //  ///////////////////////////////////////////////////////////////////////////////////////////////////

  //If shuffle mode is on, and it's been 30 seconds since the last shuffle, and then a modulo
  //  of gradient to get a random decision between palette or visualization shuffle
  if (shuffle && millis() / 1000.0 - shuffleTime > 30 && gradient % 2) {

    shuffleTime = millis() / 1000.0; //Record the time this shuffle happened.

    palette++;
    if (palette >= sizeof(thresholds) / 2) palette = 0;
    gradient %= thresholds[palette];
    maxVol = avgVol;  //Set the max volume to average for a fresh experience.
  }
}

void CycleVisual() {
//  IMPORTANT: Delete this whole if-block if you didn't use buttons////////////////////////////////// 
    bool buttonPressed = false;
    if (digitalRead(BUTTON_PIN)) {
      for (int i = 0; i < strand.numPixels(); i++) {
        strand.setPixelColor(i, Color(128, 0, 0));
      }
      strand.show();
      buttonPressed = true;
      while (!digitalRead(BUTTON_PIN)) {
        delay(1);
      }
    }
    if (buttonPressed) {
  
      visual++;     //The purpose of this button: change the visual mode
  
      gradient = 0; //Prevent overflow
  
      //Resets "visual" if there are no more visuals to cycle through.
      if (visual > NUMBER_OF_VISUALS) {
        visual = 0;
      }
      //This is why you should change "NUMBER_OF_VISUALS" if you add a visual, or the program loop over it.
  
      //Resets the positions of all dots to nonexistent (-2) if you cycle to the Traffic() visual.
      if (visual == 1) {
        memset(pos, -2, sizeof(pos));
      }
  
      //Gives Snake() and PaletteDance() visuals a random starting point if cycled to.
      if (visual == 2 || visual == 3) {
        randomSeed(analogRead(0));
        dotPos = random(strand.numPixels());
      }
  
      //Like before, this delay is to prevent a button press from affecting "maxVol."
      delay(350);
  
      maxVol = avgVol; //Set max volume to average for a fresh experience
    }

  ///////////////////////////////////////////////////////////////////////////////////////////////////

  //If shuffle mode is on, and it's been 30 seconds since the last shuffle, and then a modulo
  //  of gradient WITH INVERTED LOGIC to get a random decision between what to shuffle.
  //  This guarantees one and only one of these shuffles will occur.
  if (shuffle && millis() / 1000.0 - shuffleTime > 30 && !(gradient % 2)) {

    shuffleTime = millis() / 1000.0; //Record the time this shuffle happened.

    visual++;
    gradient = 0;
    if (visual > NUMBER_OF_VISUALS) visual = 0;
    if (visual == 1) memset(pos, -2, sizeof(pos));
    if (visual == 2 || visual == 3) {
      randomSeed(analogRead(0));
      dotPos = random(strand.numPixels());
    }
    maxVol = avgVol;
  }
}

//IMPORTANT: Delete this function  if you didn't use buttons./////////////////////////////////////////
void ToggleShuffle() {
  //  if (signalToToggleShuffleReceived) {
  //
  //    shuffle = !shuffle; //This button's purpose: toggle shuffle mode.
  //
  //    //This delay is to prevent the button from taking another reading while you're pressing it
  //    delay(500);
  //
  //    //Reset these things for a fresh experience.
  //    maxVol = avgVol;
  //    avgBump = 0;
  //  }
}

