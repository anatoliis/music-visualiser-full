//Recycles Glitter()'s random positioning; simulates "paintballs" of
//color splattering randomly on the strand and bleeding together.
void Paintball() {

  //If it's been twice the average time for a "bump" since the last "bump," start fading.
  if ((millis() / 1000.0) - timeBump > avgTime * 2.0) Fade(0.99);

  //Bleeds colors together. Operates similarly to fade. For more info, see its definition below
  Bleed(dotPos);

  //Create a new paintball if there's a bump (like the sparkles in Glitter())
  if (bump) {

    //Random generator needs a seed, and micros() gives a large range of values.
    //  micros() is the amount of microseconds since the program started running.
    randomSeed(micros());

    //Pick a random spot on the strip. Random was already reseeded above, so no real need to do it again.
    dotPos = random(strand.numPixels() - 1);

    //Grab a random color from our palette.
    uint32_t col = ColorPalette(random(thresholds[palette]));

    //Array to hold final RGB values
    uint8_t colors[3];

    //Relates brightness of the color to the relative volume and potentiometer value.
    for (int i = 0; i < 3; i++) colors[i] = Split(col, i) * pow(volume / maxVol, 2.0) * brightnessKnob;

    //Splatters the "paintball" on the random position.
    strand.setPixelColor(dotPos, Color(colors[0], colors[1], colors[2]));

    //This next part places a less bright version of the same color next to the left and right of the
    //  original position, so that the bleed effect is stronger and the colors are more vibrant.
    for (int i = 0; i < 3; i++) colors[i] *= .8;
    strand.setPixelColor(dotPos - 1, Color(colors[0], colors[1], colors[2]));
    strand.setPixelColor(dotPos + 1, Color(colors[0], colors[1], colors[2]));
  }
  strand.show(); //Show lights.
}
