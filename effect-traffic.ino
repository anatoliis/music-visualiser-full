//Dots racing into each other
void Traffic() {

  //Creates the trail behind each dot
  Fade(0.8);

  //Create a dot to be displayed if a bump is detected
  if (bump) {
    //This mess simply checks if there is an open position (-2) in the pos[] array
    int8_t slot = 0;
    for (slot; slot < sizeof(pos); slot++) {
      if (pos[slot] < -1) break;
      else if (slot + 1 >= sizeof(pos)) {
        slot = -3;
        break;
      }
    }

    //If there is an open slot, set it to an initial position on the strand
    if (slot != -3) {
      //Evens go right, odds go left, so evens start at 0, odds at the largest position.
      pos[slot] = (slot % 2 == 0) ? -1 : strand.numPixels();

      //Give it a color based on the value of "gradient" during its birth.
      uint32_t col = ColorPalette(-1);
      gradient += thresholds[palette] / 24;
      for (int j = 0; j < 3; j++) {
        rgb[slot][j] = Split(col, j);
      }
    }
  }

  //Again, if it's silent we want the colors to fade out.
  if (volume > 0) {
    //If there's sound, iterate each dot appropriately along the strand.
    for (int i = 0; i < sizeof(pos); i++) {

      //If a dot is -2, that means it's an open slot for another dot to take over eventually.
      if (pos[i] < -1) continue;

      //As above, evens go right (+1) and odds go left (-1)
      pos[i] += (i % 2) ? -1 : 1;

      //Odds will reach -2 by subtraction, but if an even dot goes beyond the LED strip, it'll be purged.
      if (pos[i] >= strand.numPixels()) pos[i] = -2;

      //Set the dot to its new position and respective color.
      //It's old position's color will gradually fade out due to Fade(), leaving a trail behind it.
      strand.setPixelColor( pos[i], Color(
                              float(rgb[i][0]) * pow(volume / maxVol, 2.0) * brightnessKnob,
                              float(rgb[i][1]) * pow(volume / maxVol, 2.0) * brightnessKnob,
                              float(rgb[i][2]) * pow(volume / maxVol, 2.0) * brightnessKnob)
                          );
    }
  }
  strand.show();
}
