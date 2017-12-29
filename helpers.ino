//Gives you one 8-bit color value from the composite 32-bit
//value that the Adafruit_WS2801 library deals with.
//This is accomplished with the right bit shift operator, ">>"
uint8_t Split(uint32_t color, uint8_t i) {
  //0 = Red, 1 = Green, 2 = Blue
  if (i == 0) return color >> 16;
  if (i == 1) return color >> 8;
  if (i == 2) return color >> 0;
  return -1;
}

uint32_t Color(byte r, byte g, byte b) {
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}