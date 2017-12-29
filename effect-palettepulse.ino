//Same as Pulse(), but colored the entire pallet instead of one solid color
void PalettePulse() {
  Fade(0.75);
  if (bump) {
    gradient += thresholds[palette] / 24;
  }
  if (volume > 0) {
    int start = LED_HALF - (LED_HALF * (volume / maxVol));
    int finish = LED_HALF + (LED_HALF * (volume / maxVol)) + strand.numPixels() % 2;
    for (int i = start; i < finish; i++) {
      float damp = sin((i - start) * PI / float(finish - start));
      damp = pow(damp, 2.0);

      //This is the only difference from Pulse(). The color for each pixel isn't the same, but rather the
      //  entire gradient fitted to the spread of the pulse, with some shifting from "gradient".
      int val = thresholds[palette] * (i - start) / (finish - start);
      val += gradient;
      uint32_t col = ColorPalette(val);

      uint32_t col2 = strand.getPixelColor(i);
      uint8_t colors[3];
      float avgCol = 0, avgCol2 = 0;
      for (int k = 0; k < 3; k++) {
        colors[k] = Split(col, k) * damp * brightnessKnob * pow(volume / maxVol, 2);
        avgCol += colors[k];
        avgCol2 += Split(col2, k);
      }
      avgCol /= 3.0, avgCol2 /= 3.0;
      if (avgCol > avgCol2) {
        strand.setPixelColor(i, Color(colors[0], colors[1], colors[2]));
      }
    }
  }

  strand.show();
}
