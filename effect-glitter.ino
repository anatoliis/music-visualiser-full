//GLITTER
//Creates white sparkles on a color palette to the beat
void Glitter() {

  //This visual also fits a whole palette on the entire strip
  //  This just makes the palette cycle more quickly so it's more visually pleasing
  gradient += thresholds[palette] / 204;

  //"val" is used again as the proportional value to pass to ColorPalette() to fit the whole palette.
  for (int i = 0; i < strand.numPixels(); i++) {
    unsigned int val = float(thresholds[palette] + 1) *
                       (float(i) / float(strand.numPixels()))
                       + (gradient);
    val %= thresholds[palette];
    uint32_t  col = ColorPalette(val);

    //We want the sparkles to be obvious, so we dim the background color.
    strand.setPixelColor(i, Color(
                           Split(col, 0) / 6.0 * brightnessKnob,
                           Split(col, 1) / 6.0 * brightnessKnob,
                           Split(col, 2) / 6.0 * brightnessKnob)
                        );
  }

  //Create sparkles every bump
  if (bump) {

    //Random generator needs a seed, and micros() gives a large range of values.
    //  micros() is the amount of microseconds since the program started running.
    randomSeed(micros());

    //Pick a random spot on the strand.
    dotPos = random(strand.numPixels() - 1);

    //Draw  sparkle at the random position, with appropriate brightness.
    strand.setPixelColor(dotPos, Color(
                           255.0 * pow(volume / maxVol, 2.0) * brightnessKnob,
                           255.0 * pow(volume / maxVol, 2.0) * brightnessKnob,
                           255.0 * pow(volume / maxVol, 2.0) * brightnessKnob
                         ));
  }
  Bleed(dotPos);
  strand.show(); //Show the lights.
}
