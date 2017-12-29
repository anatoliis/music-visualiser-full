//Fades lights by multiplying them by a value between 0 and 1 each pass of loop().
//"damper" must be between 0 and 1, or else you'll end up brightening the lights or doing nothing.
void Fade(float damper) {
  for (int i = 0; i < strand.numPixels(); i++) {
    //Retrieve the color at the current position.
    uint32_t col = strand.getPixelColor(i);
    //If it's black, you can't fade that any further.
    if (col == 0) {
        continue;
    }
    float colors[3]; //Array of the three RGB values
    //Multiply each value by "damper"
    for (int j = 0; j < 3; j++) {
        colors[j] = Split(col, j) * damper;
    }
    //Set the dampened colors back to their spot.
    strand.setPixelColor(i, Color(colors[0] , colors[1], colors[2]));
  }
}


//"Bleeds" colors currently in the strand by averaging from a designated "Point"
void Bleed(uint8_t Point) {
  for (int i = 1; i < strand.numPixels(); i++) {

    //Starts by look at the pixels left and right of "Point"
    //  then slowly works its way out
    int sides[] = {Point - i, Point + i};

    for (int i = 0; i < 2; i++) {

      //For each of Point+i and Point-i, the pixels to the left and right, plus themselves, are averaged together.
      //  Basically, it's setting one pixel to the average of it and its neighbors, starting on the left and right
      //  of the starting "Point," and moves to the ends of the strand
      int point = sides[i];
      uint32_t colors[] = {strand.getPixelColor(point - 1), strand.getPixelColor(point), strand.getPixelColor(point + 1)  };

      //Sets the new average values to just the central point, not the left and right points.
      strand.setPixelColor(point, Color(
                             float( Split(colors[0], 0) + Split(colors[1], 0) + Split(colors[2], 0) ) / 3.0,
                             float( Split(colors[0], 1) + Split(colors[1], 1) + Split(colors[2], 1) ) / 3.0,
                             float( Split(colors[0], 2) + Split(colors[1], 2) + Split(colors[2], 2) ) / 3.0)
                          );
    }
  }
}
