/*
  colorNameToRGB - Dead simple code by Jeremy Williams (@jerware)
  for converting color names to RGB values. Handy for Amazon Alexa.

  Pass it a String with a color name and three references to R, G, B bytes.
  E.g. colorNameToRGB(c, &r, &g, &b);

  Uses color names defined by FastLED library:
  https://github.com/FastLED/FastLED/wiki/Pixel-reference#predefined-colors-list
*/

void colorNameToRGB(String colorName, byte *r, byte *g, byte *b)
{
  colorName.replace(" ", ""); // remove spaces
  if (colorName.equalsIgnoreCase("AliceBlue"))
  {
  	*r = 0xF0;
  	*g = 0xF8;
  	*b = 0xFF;
  }
  else if (colorName.equalsIgnoreCase("Amethyst"))
  {
  	*r = 0x99;
  	*g = 0x66;
  	*b = 0xCC;
  }
  else if (colorName.equalsIgnoreCase("AntiqueWhite"))
  {
  	*r = 0xFA;
  	*g = 0xEB;
  	*b = 0xD7;
  }
  else if (colorName.equalsIgnoreCase("Aqua"))
  {
  	*r = 0x00;
  	*g = 0xFF;
  	*b = 0xFF;
  }
  else if (colorName.equalsIgnoreCase("Aquamarine"))
  {
  	*r = 0x7F;
  	*g = 0xFF;
  	*b = 0xD4;
  }
  else if (colorName.equalsIgnoreCase("Azure"))
  {
  	*r = 0xF0;
  	*g = 0xFF;
  	*b = 0xFF;
  }
  else if (colorName.equalsIgnoreCase("Beige"))
  {
  	*r = 0xF5;
  	*g = 0xF5;
  	*b = 0xDC;
  }
  else if (colorName.equalsIgnoreCase("Bisque"))
  {
  	*r = 0xFF;
  	*g = 0xE4;
  	*b = 0xC4;
  }
  else if (colorName.equalsIgnoreCase("Black"))
  {
  	*r = 0x00;
  	*g = 0x00;
  	*b = 0x00;
  }
  else if (colorName.equalsIgnoreCase("BlanchedAlmond"))
  {
  	*r = 0xFF;
  	*g = 0xEB;
  	*b = 0xCD;
  }
  else if (colorName.equalsIgnoreCase("Blue"))
  {
  	*r = 0x00;
  	*g = 0x00;
  	*b = 0xFF;
  }
  else if (colorName.equalsIgnoreCase("BlueViolet"))
  {
  	*r = 0x8A;
  	*g = 0x2B;
  	*b = 0xE2;
  }
  else if (colorName.equalsIgnoreCase("Brown"))
  {
  	*r = 0xA5;
  	*g = 0x2A;
  	*b = 0x2A;
  }
  else if (colorName.equalsIgnoreCase("BurlyWood"))
  {
  	*r = 0xDE;
  	*g = 0xB8;
  	*b = 0x87;
  }
  else if (colorName.equalsIgnoreCase("CadetBlue"))
  {
  	*r = 0x5F;
  	*g = 0x9E;
  	*b = 0xA0;
  }
  else if (colorName.equalsIgnoreCase("Chartreuse"))
  {
  	*r = 0x7F;
  	*g = 0xFF;
  	*b = 0x00;
  }
  else if (colorName.equalsIgnoreCase("Chocolate"))
  {
  	*r = 0xD2;
  	*g = 0x69;
  	*b = 0x1E;
  }
  else if (colorName.equalsIgnoreCase("Coral"))
  {
  	*r = 0xFF;
  	*g = 0x7F;
  	*b = 0x50;
  }
  else if (colorName.equalsIgnoreCase("CornflowerBlue"))
  {
  	*r = 0x64;
  	*g = 0x95;
  	*b = 0xED;
  }
  else if (colorName.equalsIgnoreCase("Cornsilk"))
  {
  	*r = 0xFF;
  	*g = 0xF8;
  	*b = 0xDC;
  }
  else if (colorName.equalsIgnoreCase("Crimson"))
  {
  	*r = 0xDC;
  	*g = 0x14;
  	*b = 0x3C;
  }
  else if (colorName.equalsIgnoreCase("Cyan"))
  {
  	*r = 0x00;
  	*g = 0xFF;
  	*b = 0xFF;
  }
  else if (colorName.equalsIgnoreCase("DarkBlue"))
  {
  	*r = 0x00;
  	*g = 0x00;
  	*b = 0x8B;
  }
  else if (colorName.equalsIgnoreCase("DarkCyan"))
  {
  	*r = 0x00;
  	*g = 0x8B;
  	*b = 0x8B;
  }
  else if (colorName.equalsIgnoreCase("DarkGoldenrod"))
  {
  	*r = 0xB8;
  	*g = 0x86;
  	*b = 0x0B;
  }
  else if (colorName.equalsIgnoreCase("DarkGray"))
  {
  	*r = 0xA9;
  	*g = 0xA9;
  	*b = 0xA9;
  }
  else if (colorName.equalsIgnoreCase("DarkGreen"))
  {
  	*r = 0x00;
  	*g = 0x64;
  	*b = 0x00;
  }
  else if (colorName.equalsIgnoreCase("DarkKhaki"))
  {
  	*r = 0xBD;
  	*g = 0xB7;
  	*b = 0x6B;
  }
  else if (colorName.equalsIgnoreCase("DarkMagenta"))
  {
  	*r = 0x8B;
  	*g = 0x00;
  	*b = 0x8B;
  }
  else if (colorName.equalsIgnoreCase("DarkOliveGreen"))
  {
  	*r = 0x55;
  	*g = 0x6B;
  	*b = 0x2F;
  }
  else if (colorName.equalsIgnoreCase("DarkOrange"))
  {
  	*r = 0xFF;
  	*g = 0x8C;
  	*b = 0x00;
  }
  else if (colorName.equalsIgnoreCase("DarkOrchid"))
  {
  	*r = 0x99;
  	*g = 0x32;
  	*b = 0xCC;
  }
  else if (colorName.equalsIgnoreCase("DarkRed"))
  {
  	*r = 0x8B;
  	*g = 0x00;
  	*b = 0x00;
  }
  else if (colorName.equalsIgnoreCase("DarkSalmon"))
  {
  	*r = 0xE9;
  	*g = 0x96;
  	*b = 0x7A;
  }
  else if (colorName.equalsIgnoreCase("DarkSeaGreen"))
  {
  	*r = 0x8F;
  	*g = 0xBC;
  	*b = 0x8F;
  }
  else if (colorName.equalsIgnoreCase("DarkSlateBlue"))
  {
  	*r = 0x48;
  	*g = 0x3D;
  	*b = 0x8B;
  }
  else if (colorName.equalsIgnoreCase("DarkSlateGray"))
  {
  	*r = 0x2F;
  	*g = 0x4F;
  	*b = 0x4F;
  }
  else if (colorName.equalsIgnoreCase("DarkTurquoise"))
  {
  	*r = 0x00;
  	*g = 0xCE;
  	*b = 0xD1;
  }
  else if (colorName.equalsIgnoreCase("DarkViolet"))
  {
  	*r = 0x94;
  	*g = 0x00;
  	*b = 0xD3;
  }
  else if (colorName.equalsIgnoreCase("DeepPink"))
  {
  	*r = 0xFF;
  	*g = 0x14;
  	*b = 0x93;
  }
  else if (colorName.equalsIgnoreCase("DeepSkyBlue"))
  {
  	*r = 0x00;
  	*g = 0xBF;
  	*b = 0xFF;
  }
  else if (colorName.equalsIgnoreCase("DimGray"))
  {
  	*r = 0x69;
  	*g = 0x69;
  	*b = 0x69;
  }
  else if (colorName.equalsIgnoreCase("DodgerBlue"))
  {
  	*r = 0x1E;
  	*g = 0x90;
  	*b = 0xFF;
  }
  else if (colorName.equalsIgnoreCase("FireBrick"))
  {
  	*r = 0xB2;
  	*g = 0x22;
  	*b = 0x22;
  }
  else if (colorName.equalsIgnoreCase("FloralWhite"))
  {
  	*r = 0xFF;
  	*g = 0xFA;
  	*b = 0xF0;
  }
  else if (colorName.equalsIgnoreCase("ForestGreen"))
  {
  	*r = 0x22;
  	*g = 0x8B;
  	*b = 0x22;
  }
  else if (colorName.equalsIgnoreCase("Fuchsia"))
  {
  	*r = 0xFF;
  	*g = 0x00;
  	*b = 0xFF;
  }
  else if (colorName.equalsIgnoreCase("Gainsboro"))
  {
  	*r = 0xDC;
  	*g = 0xDC;
  	*b = 0xDC;
  }
  else if (colorName.equalsIgnoreCase("GhostWhite"))
  {
  	*r = 0xF8;
  	*g = 0xF8;
  	*b = 0xFF;
  }
  else if (colorName.equalsIgnoreCase("Gold") || colorName.equalsIgnoreCase("Golden"))
  {
  	*r = 0xFF;
  	*g = 0xD7;
  	*b = 0x00;
  }
  else if (colorName.equalsIgnoreCase("Goldenrod"))
  {
  	*r = 0xDA;
  	*g = 0xA5;
  	*b = 0x20;
  }
  else if (colorName.equalsIgnoreCase("Gray"))
  {
  	*r = 0x80;
  	*g = 0x80;
  	*b = 0x80;
  }
  else if (colorName.equalsIgnoreCase("Green"))
  {
  	*r = 0x00;
  	*g = 0x80;
  	*b = 0x00;
  }
  else if (colorName.equalsIgnoreCase("GreenYellow"))
  {
  	*r = 0xAD;
  	*g = 0xFF;
  	*b = 0x2F;
  }
  else if (colorName.equalsIgnoreCase("Honeydew"))
  {
  	*r = 0xF0;
  	*g = 0xFF;
  	*b = 0xF0;
  }
  else if (colorName.equalsIgnoreCase("HotPink"))
  {
  	*r = 0xFF;
  	*g = 0x69;
  	*b = 0xB4;
  }
  else if (colorName.equalsIgnoreCase("IndianRed"))
  {
  	*r = 0xCD;
  	*g = 0x5C;
  	*b = 0x5C;
  }
  else if (colorName.equalsIgnoreCase("Indigo"))
  {
  	*r = 0x4B;
  	*g = 0x00;
  	*b = 0x82;
  }
  else if (colorName.equalsIgnoreCase("Ivory"))
  {
  	*r = 0xFF;
  	*g = 0xFF;
  	*b = 0xF0;
  }
  else if (colorName.equalsIgnoreCase("Khaki"))
  {
  	*r = 0xF0;
  	*g = 0xE6;
  	*b = 0x8C;
  }
  else if (colorName.equalsIgnoreCase("Lavender"))
  {
  	*r = 0xE6;
  	*g = 0xE6;
  	*b = 0xFA;
  }
  else if (colorName.equalsIgnoreCase("LavenderBlush"))
  {
  	*r = 0xFF;
  	*g = 0xF0;
  	*b = 0xF5;
  }
  else if (colorName.equalsIgnoreCase("LawnGreen"))
  {
  	*r = 0x7C;
  	*g = 0xFC;
  	*b = 0x00;
  }
  else if (colorName.equalsIgnoreCase("LemonChiffon"))
  {
  	*r = 0xFF;
  	*g = 0xFA;
  	*b = 0xCD;
  }
  else if (colorName.equalsIgnoreCase("LightBlue"))
  {
  	*r = 0xAD;
  	*g = 0xD8;
  	*b = 0xE6;
  }
  else if (colorName.equalsIgnoreCase("LightCoral"))
  {
  	*r = 0xF0;
  	*g = 0x80;
  	*b = 0x80;
  }
  else if (colorName.equalsIgnoreCase("LightCyan"))
  {
  	*r = 0xE0;
  	*g = 0xFF;
  	*b = 0xFF;
  }
  else if (colorName.equalsIgnoreCase("LightGoldenrodYellow"))
  {
  	*r = 0xFA;
  	*g = 0xFA;
  	*b = 0xD2;
  }
  else if (colorName.equalsIgnoreCase("LightGreen"))
  {
  	*r = 0x90;
  	*g = 0xEE;
  	*b = 0x90;
  }
  else if (colorName.equalsIgnoreCase("LightGrey"))
  {
  	*r = 0xD3;
  	*g = 0xD3;
  	*b = 0xD3;
  }
  else if (colorName.equalsIgnoreCase("LightPink"))
  {
  	*r = 0xFF;
  	*g = 0xB6;
  	*b = 0xC1;
  }
  else if (colorName.equalsIgnoreCase("LightSalmon"))
  {
  	*r = 0xFF;
  	*g = 0xA0;
  	*b = 0x7A;
  }
  else if (colorName.equalsIgnoreCase("LightSeaGreen"))
  {
  	*r = 0x20;
  	*g = 0xB2;
  	*b = 0xAA;
  }
  else if (colorName.equalsIgnoreCase("LightSkyBlue"))
  {
  	*r = 0x87;
  	*g = 0xCE;
  	*b = 0xFA;
  }
  else if (colorName.equalsIgnoreCase("LightSlateGray"))
  {
  	*r = 0x77;
  	*g = 0x88;
  	*b = 0x99;
  }
  else if (colorName.equalsIgnoreCase("LightSteelBlue"))
  {
  	*r = 0xB0;
  	*g = 0xC4;
  	*b = 0xDE;
  }
  else if (colorName.equalsIgnoreCase("LightYellow"))
  {
  	*r = 0xFF;
  	*g = 0xFF;
  	*b = 0xE0;
  }
  else if (colorName.equalsIgnoreCase("Lime"))
  {
  	*r = 0x00;
  	*g = 0xFF;
  	*b = 0x00;
  }
  else if (colorName.equalsIgnoreCase("LimeGreen"))
  {
  	*r = 0x32;
  	*g = 0xCD;
  	*b = 0x32;
  }
  else if (colorName.equalsIgnoreCase("Linen"))
  {
  	*r = 0xFA;
  	*g = 0xF0;
  	*b = 0xE6;
  }
  else if (colorName.equalsIgnoreCase("Magenta"))
  {
  	*r = 0xFF;
  	*g = 0x00;
  	*b = 0xFF;
  }
  else if (colorName.equalsIgnoreCase("Maroon"))
  {
  	*r = 0x80;
  	*g = 0x00;
  	*b = 0x00;
  }
  else if (colorName.equalsIgnoreCase("MediumAquamarine"))
  {
  	*r = 0x66;
  	*g = 0xCD;
  	*b = 0xAA;
  }
  else if (colorName.equalsIgnoreCase("MediumBlue"))
  {
  	*r = 0x00;
  	*g = 0x00;
  	*b = 0xCD;
  }
  else if (colorName.equalsIgnoreCase("MediumOrchid"))
  {
  	*r = 0xBA;
  	*g = 0x55;
  	*b = 0xD3;
  }
  else if (colorName.equalsIgnoreCase("MediumPurple"))
  {
  	*r = 0x93;
  	*g = 0x70;
  	*b = 0xDB;
  }
  else if (colorName.equalsIgnoreCase("MediumSeaGreen"))
  {
  	*r = 0x3C;
  	*g = 0xB3;
  	*b = 0x71;
  }
  else if (colorName.equalsIgnoreCase("MediumSlateBlue"))
  {
  	*r = 0x7B;
  	*g = 0x68;
  	*b = 0xEE;
  }
  else if (colorName.equalsIgnoreCase("MediumSpringGreen"))
  {
  	*r = 0x00;
  	*g = 0xFA;
  	*b = 0x9A;
  }
  else if (colorName.equalsIgnoreCase("MediumTurquoise"))
  {
  	*r = 0x48;
  	*g = 0xD1;
  	*b = 0xCC;
  }
  else if (colorName.equalsIgnoreCase("MediumVioletRed"))
  {
  	*r = 0xC7;
  	*g = 0x15;
  	*b = 0x85;
  }
  else if (colorName.equalsIgnoreCase("MidnightBlue"))
  {
  	*r = 0x19;
  	*g = 0x19;
  	*b = 0x70;
  }
  else if (colorName.equalsIgnoreCase("MintCream"))
  {
  	*r = 0xF5;
  	*g = 0xFF;
  	*b = 0xFA;
  }
  else if (colorName.equalsIgnoreCase("MistyRose"))
  {
  	*r = 0xFF;
  	*g = 0xE4;
  	*b = 0xE1;
  }
  else if (colorName.equalsIgnoreCase("Moccasin"))
  {
  	*r = 0xFF;
  	*g = 0xE4;
  	*b = 0xB5;
  }
  else if (colorName.equalsIgnoreCase("NavajoWhite"))
  {
  	*r = 0xFF;
  	*g = 0xDE;
  	*b = 0xAD;
  }
  else if (colorName.equalsIgnoreCase("Navy"))
  {
  	*r = 0x00;
  	*g = 0x00;
  	*b = 0x80;
  }
  else if (colorName.equalsIgnoreCase("OldLace"))
  {
  	*r = 0xFD;
  	*g = 0xF5;
  	*b = 0xE6;
  }
  else if (colorName.equalsIgnoreCase("Olive"))
  {
  	*r = 0x80;
  	*g = 0x80;
  	*b = 0x00;
  }
  else if (colorName.equalsIgnoreCase("OliveDrab"))
  {
  	*r = 0x6B;
  	*g = 0x8E;
  	*b = 0x23;
  }
  else if (colorName.equalsIgnoreCase("Orange"))
  {
  	*r = 0xFF;
  	*g = 0xA5;
  	*b = 0x00;
  }
  else if (colorName.equalsIgnoreCase("OrangeRed"))
  {
  	*r = 0xFF;
  	*g = 0x45;
  	*b = 0x00;
  }
  else if (colorName.equalsIgnoreCase("Orchid"))
  {
  	*r = 0xDA;
  	*g = 0x70;
  	*b = 0xD6;
  }
  else if (colorName.equalsIgnoreCase("PaleGoldenrod"))
  {
  	*r = 0xEE;
  	*g = 0xE8;
  	*b = 0xAA;
  }
  else if (colorName.equalsIgnoreCase("PaleGreen"))
  {
  	*r = 0x98;
  	*g = 0xFB;
  	*b = 0x98;
  }
  else if (colorName.equalsIgnoreCase("PaleTurquoise"))
  {
  	*r = 0xAF;
  	*g = 0xEE;
  	*b = 0xEE;
  }
  else if (colorName.equalsIgnoreCase("PaleVioletRed"))
  {
  	*r = 0xDB;
  	*g = 0x70;
  	*b = 0x93;
  }
  else if (colorName.equalsIgnoreCase("PapayaWhip"))
  {
  	*r = 0xFF;
  	*g = 0xEF;
  	*b = 0xD5;
  }
  else if (colorName.equalsIgnoreCase("PeachPuff"))
  {
  	*r = 0xFF;
  	*g = 0xDA;
  	*b = 0xB9;
  }
  else if (colorName.equalsIgnoreCase("Peru"))
  {
  	*r = 0xCD;
  	*g = 0x85;
  	*b = 0x3F;
  }
  else if (colorName.equalsIgnoreCase("Pink"))
  {
  	*r = 0xFF;
  	*g = 0xC0;
  	*b = 0xCB;
  }
  else if (colorName.equalsIgnoreCase("Plaid"))
  {
  	*r = 0xCC;
  	*g = 0x55;
  	*b = 0x33;
  }
  else if (colorName.equalsIgnoreCase("Plum"))
  {
  	*r = 0xDD;
  	*g = 0xA0;
  	*b = 0xDD;
  }
  else if (colorName.equalsIgnoreCase("PowderBlue"))
  {
  	*r = 0xB0;
  	*g = 0xE0;
  	*b = 0xE6;
  }
  else if (colorName.equalsIgnoreCase("Purple"))
  {
  	*r = 0x80;
  	*g = 0x00;
  	*b = 0x80;
  }
  else if (colorName.equalsIgnoreCase("Red"))
  {
  	*r = 0xFF;
  	*g = 0x00;
  	*b = 0x00;
  }
  else if (colorName.equalsIgnoreCase("RosyBrown"))
  {
  	*r = 0xBC;
  	*g = 0x8F;
  	*b = 0x8F;
  }
  else if (colorName.equalsIgnoreCase("RoyalBlue"))
  {
  	*r = 0x41;
  	*g = 0x69;
  	*b = 0xE1;
  }
  else if (colorName.equalsIgnoreCase("SaddleBrown"))
  {
  	*r = 0x8B;
  	*g = 0x45;
  	*b = 0x13;
  }
  else if (colorName.equalsIgnoreCase("Salmon"))
  {
  	*r = 0xFA;
  	*g = 0x80;
  	*b = 0x72;
  }
  else if (colorName.equalsIgnoreCase("SandyBrown"))
  {
  	*r = 0xF4;
  	*g = 0xA4;
  	*b = 0x60;
  }
  else if (colorName.equalsIgnoreCase("SeaGreen"))
  {
  	*r = 0x2E;
  	*g = 0x8B;
  	*b = 0x57;
  }
  else if (colorName.equalsIgnoreCase("Seashell"))
  {
  	*r = 0xFF;
  	*g = 0xF5;
  	*b = 0xEE;
  }
  else if (colorName.equalsIgnoreCase("Sienna"))
  {
  	*r = 0xA0;
  	*g = 0x52;
  	*b = 0x2D;
  }
  else if (colorName.equalsIgnoreCase("Silver"))
  {
  	*r = 0xC0;
  	*g = 0xC0;
  	*b = 0xC0;
  }
  else if (colorName.equalsIgnoreCase("SkyBlue"))
  {
  	*r = 0x87;
  	*g = 0xCE;
  	*b = 0xEB;
  }
  else if (colorName.equalsIgnoreCase("SlateBlue"))
  {
  	*r = 0x6A;
  	*g = 0x5A;
  	*b = 0xCD;
  }
  else if (colorName.equalsIgnoreCase("SlateGray"))
  {
  	*r = 0x70;
  	*g = 0x80;
  	*b = 0x90;
  }
  else if (colorName.equalsIgnoreCase("Snow"))
  {
  	*r = 0xFF;
  	*g = 0xFA;
  	*b = 0xFA;
  }
  else if (colorName.equalsIgnoreCase("SpringGreen"))
  {
  	*r = 0x00;
  	*g = 0xFF;
  	*b = 0x7F;
  }
  else if (colorName.equalsIgnoreCase("SteelBlue"))
  {
  	*r = 0x46;
  	*g = 0x82;
  	*b = 0xB4;
  }
  else if (colorName.equalsIgnoreCase("Tan"))
  {
  	*r = 0xD2;
  	*g = 0xB4;
  	*b = 0x8C;
  }
  else if (colorName.equalsIgnoreCase("Teal"))
  {
  	*r = 0x00;
  	*g = 0x80;
  	*b = 0x80;
  }
  else if (colorName.equalsIgnoreCase("Thistle"))
  {
  	*r = 0xD8;
  	*g = 0xBF;
  	*b = 0xD8;
  }
  else if (colorName.equalsIgnoreCase("Tomato"))
  {
  	*r = 0xFF;
  	*g = 0x63;
  	*b = 0x47;
  }
  else if (colorName.equalsIgnoreCase("Turquoise"))
  {
  	*r = 0x40;
  	*g = 0xE0;
  	*b = 0xD0;
  }
  else if (colorName.equalsIgnoreCase("Violet"))
  {
  	*r = 0xEE;
  	*g = 0x82;
  	*b = 0xEE;
  }
  else if (colorName.equalsIgnoreCase("Wheat"))
  {
  	*r = 0xF5;
  	*g = 0xDE;
  	*b = 0xB3;
  }
  else if (colorName.equalsIgnoreCase("White"))
  {
  	*r = 0xFF;
  	*g = 0xFF;
  	*b = 0xFF;
  }
  else if (colorName.equalsIgnoreCase("WhiteSmoke"))
  {
  	*r = 0xF5;
  	*g = 0xF5;
  	*b = 0xF5;
  }
  else if (colorName.equalsIgnoreCase("Yellow"))
  {
  	*r = 0xFF;
  	*g = 0xFF;
  	*b = 0x00;
  }
  else if (colorName.equalsIgnoreCase("YellowGreen"))
  {
  	*r = 0x9A;
  	*g = 0xCD;
  	*b = 0x32;
  }
}
