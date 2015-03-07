#include "word.h"

Word::Word(void) {
}

Word::~Word(void) {
}

void Word::initialize(Adafruit_NeoMatrix * matrix_, int x_, int y_, int length_, uint16_t * color_) {
  matrix = matrix_;
  x = x_;
  y = y_;
  length = length_;
  color = color_;
}

void Word::hide() {
  if (length > 10) {
    for (int p = 0; p < length - 10; p++) {
      matrix->drawPixel(x, y + p, 0);
    }
  } else {
    for (int p = 0; p < length; p++) {
      matrix->drawPixel(x + p, y, 0);
    }
  }
}

void Word::show() {
  if (length > 10) {
    for (int p = 0; p < length - 10; p++) {
      matrix->drawPixel(x, y + p, *color);
    }
  } else {
    for (int p = 0; p < length; p++) {
      matrix->drawPixel(x + p, y, *color);
    }
  }
}

