#ifndef WORD_H
#define WORD_H

#include <Adafruit_NeoMatrix.h>

class Word {
  public:
  int x;
  int y;
  int length; // If > 10, then minus 10 to get the vertical length.
  uint16_t * color;
  Adafruit_NeoMatrix * matrix;

  Word(void);
  void initialize(Adafruit_NeoMatrix * matrix, int x_, int y_, int length_, uint16_t * color_);
  void hide();
  void show();
  ~Word(void);
};

#endif



