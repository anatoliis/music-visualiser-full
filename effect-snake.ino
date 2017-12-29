//Dot sweeping back and forth to the beat
void Snake() {

  if (bump) {
    //Change color a little on a bump
    gradient += thresholds[palette] / 30;
    //Change the direction the dot is going to create the illusion of "dancing"
    left = !left;
  }

  //Leave a trail behind the dot
  Fade(0.975);

  //Get the color at current "gradient"
  uint32_t col = ColorPalette(-1);

  //The dot should only be moved if there's sound happening.
  //Otherwise if noise starts and it's been moving, it'll appear to teleport.
  if (volume > 0) {
    //Sets the dot to appropriate color and intensity
    strand.setPixelColor(dotPos, Color(
                           float(Split(col, 0)) * pow(volume / maxVol, 1.5) * brightnessKnob,
                           float(Split(col, 1)) * pow(volume / maxVol, 1.5) * brightnessKnob,
                           float(Split(col, 2)) * pow(volume / maxVol, 1.5) * brightnessKnob)
                        );

    //This is where "avgTime" comes into play.
    //That variable is the "average" amount of time between each "bump" detected.
    //So we can use that to determine how quickly the dot should move so it matches the tempo of the music.
    //The dot moving at normal loop speed is pretty quick, so it's the max speed if avgTime < 0.15 seconds.
    //Slowing it down causes the color to update, but only change position every other amount of loops.
    if (avgTime < 0.15)                                               dotPos += (left) ? -1 : 1;
    else if (avgTime >= 0.15 && avgTime < 0.5 && gradient % 2 == 0)   dotPos += (left) ? -1 : 1;
    else if (avgTime >= 0.5 && avgTime < 1.0 && gradient % 3 == 0)    dotPos += (left) ? -1 : 1;
    else if (gradient % 4 == 0)                                       dotPos += (left) ? -1 : 1;
  }

  strand.show();

  //Check if dot position is out of bounds
  if (dotPos < 0) {
    dotPos = strand.numPixels() - 1;
  }
  else if (dotPos >= strand.numPixels()) {
    dotPos = 0;
  }
}
