#include <Adafruit_GFX.h>
#include <Adafruit_NeoMatrix.h>
#include <Adafruit_NeoPixel.h>
#include <DS3231.h>
#include <Wire.h>
#include "word.h"


#define PIN 9 //10
#define WIDTH 11
#define HEIGHT 11

#define PHOTO_RANGE 724.0 // add lower limit, up to 255
#define PHOTO_LOWER 300
#define LED_BRIGHTNESS_RANGE 245.0 // add lower limit, up to 255
#define LED_BRIGHTNESS_LOWER 10

#define CLICK_PIN 7//12
#define PHOTO_PIN A3//A0

#define NUM_MODES 3
// 0 = DEFAULT CLOCK
// 1 = TEMPERATURE
// 2 = COLORS
// 3 = SET TIME
// 4 = DEMO

#define THRESHOLD_DEMO_SEC 3
#define DEMO_MODE 4

DS3231 Clock;

int clickNow = 1;
int clickOld = 1;
int rotaryVal = 0;
uint8_t currentMode = 0;
uint8_t demoIndex = 0;
bool h12;
bool hPM;
bool century = false;


Word nowWords[12];
uint8_t currentIndex = 0;

Adafruit_NeoMatrix matrix = Adafruit_NeoMatrix(WIDTH, HEIGHT, PIN,
NEO_MATRIX_BOTTOM     + NEO_MATRIX_LEFT +
NEO_MATRIX_COLUMNS + NEO_MATRIX_PROGRESSIVE,
NEO_GRB            + NEO_KHZ800);

Word wHappy, wBirthday, wNhan, wUyen;
Word wHalf, wA, wQuarter, wTwenty, wFive, wTen;
Word wTo, wPast;
Word wNine, wOne, wSix, wThree, wFour, wFive2, wTwo, wEight, wEleven, wSeven, wTwelve, wTen2;
Word wOclock;

Word* hourWordsMapping[] =
    {&wTwelve, &wOne, &wTwo, &wThree, &wFour, &wFive, &wSix, &wSeven, &wEight, &wNine, &wTen2, &wEleven};
    
uint16_t colors[] = {matrix.Color(255, 255, 255) /* white */,
                     matrix.Color(0, 255, 255) /* cyan */,
                     matrix.Color(255, 255, 0) /* yellow */,
                     matrix.Color(255, 0, 0) /* red */,
                     matrix.Color(0, 255, 0) /* green */,
                     matrix.Color(0, 0, 255) /* blue */
                     };
uint16_t currentColor = colors[0];
uint8_t numColors = 6;
uint8_t colorIndex = 0;
uint8_t currentSecond = 0;
boolean playedDemo;

uint8_t hour;
uint8_t minute;
uint8_t second;
uint8_t month;
uint8_t date;

boolean dstOffset = false;

// Variables to calculate new time to set.
int totalMinutes;
int newHour;
int newMinute;

boolean hasClicked = false;

void setup() {
  pinMode(2, INPUT);
  attachInterrupt(0, Rotated, CHANGE);

  pinMode(3, INPUT);
  attachInterrupt(1, Rotated, CHANGE);

  pinMode(CLICK_PIN, INPUT);
  digitalWrite(CLICK_PIN, HIGH);

  Wire.begin();

  matrix.begin();
  matrix.setTextWrap(false);
  matrix.setBrightness(200);
  matrix.setTextSize(1);
  //matrix.setRemapFunction(clockCoordinateRemapFn);

  matrix.setTextColor(colors[0]);

  setupWords();
  updateNowWords();
  Clock.setClockMode(false);
  //Serial.begin(9600);
  introAnimation();
}

void introAnimation() {
  colorWipe(colors[5], 10);
  currentColor = colors[0];
}

void showLetter(uint8_t x, uint8_t y) {
  matrix.drawPixel(x, y, currentColor);
  matrix.show();
  delay(500);
}

void colorWipe(uint32_t c, uint8_t wait) {
  for(uint16_t i=0; i<matrix.numPixels(); i++) {
      matrix.setPixelColor(i, c);
      matrix.show();
      delay(wait);
  }
}


void setupWords() {
  wHappy.initialize(&matrix, 1, 0, 5, &currentColor);
  wBirthday.initialize(&matrix, 3, 2, 8, &currentColor);
  wNhan.initialize(&matrix, 0, 4, 4, &currentColor);
  wUyen.initialize(&matrix, 4, 5, 4, &currentColor);
  
  wHalf.initialize(&matrix, 7, 0, 4, &currentColor);
  wA.initialize(&matrix, 0, 1, 1, &currentColor);
  wQuarter.initialize(&matrix, 2, 1, 7, &currentColor);
  wTwenty.initialize(&matrix, 0, 3, 6, &currentColor);
  wFive.initialize(&matrix, 7, 3, 4, &currentColor);
  wTen.initialize(&matrix, 5, 4, 3, &currentColor);

  wTo.initialize(&matrix, 9, 4, 2, &currentColor);
  wPast.initialize(&matrix, 0, 5, 4, &currentColor);
  
  wOne.initialize(&matrix, 0, 6, 3, &currentColor);
  wTwo.initialize(&matrix, 8, 7, 3, &currentColor);
  wThree.initialize(&matrix, 6, 6, 5, &currentColor);
  wFour.initialize(&matrix, 0, 7, 4, &currentColor);
  wFive2.initialize(&matrix, 4, 7, 4, &currentColor);
  wSix.initialize(&matrix, 3, 6, 3, &currentColor);
  wSeven.initialize(&matrix, 0, 9, 5, &currentColor);
  wEight.initialize(&matrix, 0, 8, 5, &currentColor);
  wNine.initialize(&matrix, 7, 5, 4, &currentColor);
  wTen2.initialize(&matrix, 0, 10, 3, &currentColor);
  wEleven.initialize(&matrix, 5, 8, 6, &currentColor);
  wTwelve.initialize(&matrix, 5, 9, 6, &currentColor);
  
  wOclock.initialize(&matrix, 5, 10, 6, &currentColor);
}

void updateNowWords() {
  currentIndex = 0;
  addMinuteWords(minute);
  addHourWords(hour, minute);  
}

void showNowWords() {
  matrix.fillScreen(0); 
  for (uint8_t i = 0; i < currentIndex; i++) {
    nowWords[i].show();
  }
  matrix.show();
}

void addHourWords(uint8_t h, uint8_t m) {
  addWord(hourWordsMapping[(h + dstOffset) % 12]);
  if (m < 5) {
    addWord(&wOclock);
  } 
}

void addMinuteWords(uint8_t m) {
  if (m < 5 || m == 60) {
    addWord(&wOclock);
  }
  else if (m < 10) {
    addWord(&wFive);
    addWord(&wPast);
  } 
  else if (m < 15) {
    addWord(&wTen);
    addWord(&wPast);
  } 
  else if (m < 20) {
    addWord(&wA);
    addWord(&wQuarter);
    addWord(&wPast);
  } 
  else if (m < 25) {
    addWord(&wTwenty);
    addWord(&wPast);
  } 
  else if (m < 30) {
    addWord(&wTwenty);
    addWord(&wFive);
    addWord(&wPast);
  } 
  else if (m < 35) {
    addWord(&wHalf);
    addWord(&wPast);
  } 
  else if (m < 40) {
    addWord(&wTwenty);
    addWord(&wFive);
    addWord(&wTo);
  } 
  else if (m < 45) {
    addWord(&wTwenty);
    addWord(&wTo);
  } 
  else if (m < 50) {
    addWord(&wA);
    addWord(&wQuarter);
    addWord(&wTo);
  } 
  else if (m < 55) {
    addWord(&wTen);
    addWord(&wTo);
  } 
  else if (m < 60) {
    addWord(&wFive);
    addWord(&wTo);
  }
}

void addWord(Word * word) {
  nowWords[currentIndex] = *word;
  currentIndex++;
}

void loop() {  
  updateCurrentMode();
  hour = Clock.getHour(h12, hPM);
  minute = Clock.getMinute();  
  second = Clock.getSecond();  
  month = Clock.getMonth(century);
  date = Clock.getDate();
  dstOffset = 0;//isDaylightSavingTime();

  switch (currentMode) {
    case 0:
      if (isSpecialMoments()) {
        updateSpecialMoments();
      } else {
        updateTime();
      }
      break;
    case 1:
      updateTemperature();
      break;
    case 2:
      updateColors();
      break;
    case 3:
      setTime();
      break;
    case 4:
      showDemo();
      break;
    default:
      break;
  }
  
  updateBrightness();
  delay(10);
}

void updateBrightness() {
  float photoRead = analogRead(PHOTO_PIN);
  if (photoRead < PHOTO_LOWER) {
    photoRead = PHOTO_LOWER;
  }
  float newBrightnessCal =
      (photoRead - PHOTO_LOWER)/ PHOTO_RANGE * LED_BRIGHTNESS_RANGE + LED_BRIGHTNESS_LOWER; 
  matrix.setBrightness((int) newBrightnessCal);
}

void updateTemperature() {
  int t = Clock.getTemperature();
  matrix.fillScreen(0); 
  matrix.setTextColor(currentColor);
  matrix.setCursor(0, 3);
  matrix.write(t/10 + 48);
  matrix.setCursor(6, 3);
  matrix.write(t%10 + 48);
  matrix.drawRoundRect(9, 0, 2, 2, 1, currentColor);
  matrix.show();
}

void updateTime() {
  updateNowWords();
  showNowWords();
}

void updateCurrentMode() {
  clickNow = digitalRead(CLICK_PIN);
  if (clickNow != clickOld) {
    if (clickNow == 0) {
      currentMode = (currentMode + 1) % NUM_MODES;
      resetModeVariables();
      rotaryVal = 0;
      hasClicked = true;
      currentSecond = second;
    } else {
      Serial.println(second - currentSecond);
      if (hasClicked && (second - currentSecond) >= THRESHOLD_DEMO_SEC) {
        currentMode = DEMO_MODE;
      }
    }
  }
  clickOld = clickNow;
}

void resetModeVariables() {
  playedDemo = false;
  demoIndex = 0;
}

void updateColors() {
  if (rotaryVal != 0) {
    colorIndex = max(0, colorIndex + numColors + rotaryVal) % numColors;
    currentColor = colors[colorIndex];
    rotaryVal = 0;
  }
  matrix.fillScreen(0);
  matrix.drawCircle(5, 5, 3, currentColor);
  matrix.show();
}

uint16_t clockCoordinateRemapFn(uint16_t x, uint16_t y) {
  return WIDTH * y + x*((y+1)%2) +  (y % 2)*(WIDTH-x-1);
}

void Rotated() {
  static uint8_t old_AB = 3;  //lookup table index
  static int8_t encval = 0;   //encoder value  
  static int8_t total_val = 0;   //encoder value  
  static uint8_t enc_states [] = {
    0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0  };  //encoder lookup table
  /**/

  old_AB <<=2;  //remember previous state
  old_AB |= ( (PIND >> 2) & 0x03 );

  encval += enc_states[( old_AB & 0x0f )];
  total_val += enc_states[( old_AB & 0x0f )];

  /* post "Navigation forward/reverse" event */
  if( encval > 3 ) {  //four steps forward
    rotaryVal += 1;
    encval = 0;
  }
  else if( encval < -3 ) {  //four steps backwards
    rotaryVal -= 1;
    encval = 0;
  }   
}

boolean isSpecialMoments() {
  if (minute % 5 == 0) {
    if (month == 11 && date == 9) {
      return true;
    }
    if (month == 11 && date == 3) {
      return true;
    }
  }
  return false;
}

void updateSpecialMoments() {
  currentIndex = 0;
  // Example to flash on a birthday
  /*
  if (month == 5 && date == 13) {
    addWord(&wHappy);
    addWord(&wBirthday);
    addWord(&wNhan);
  }*/
  showNowWords();
}

void setTime() {
  if (rotaryVal != 0) {
    updateSetTime();
  }
  updateNowWords();
  matrix.fillScreen(0); 
  for (uint8_t i = 0; i < currentIndex; i++) {
    nowWords[i].show();
  }
  // Blinking minute.
  boolean isOddSecond = Clock.getSecond() % 2;
  if (isOddSecond) {
    wOclock.hide();
  } else {
    wOclock.show();
  }
  matrix.show();
}

void updateSetTime() {
  if (rotaryVal < 0) {
    changeTime(5);
  } else {
    changeTime(-5);
  }
  rotaryVal = 0;
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
uint32_t Wheel(byte WheelPos) {
  WheelPos = 255 - WheelPos;
  if(WheelPos < 85) {
   return matrix.Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else if(WheelPos < 170) {
    WheelPos -= 85;
   return matrix.Color(0, WheelPos * 3, 255 - WheelPos * 3);
  } else {
   WheelPos -= 170;
   return matrix.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  }
}

void changeTime(int minuteDiff) {
  totalMinutes = hour * 60 + minute;
  totalMinutes = (totalMinutes + minuteDiff + 24 * 60) % (24 * 60);
  newHour = totalMinutes / 60;
  newMinute = (totalMinutes % 60) / 5 * 5;
  Clock.setHour(newHour);
  Clock.setMinute(newMinute);
}

void showDemo() {
  if (!playedDemo) {
    switch (demoIndex) {
      case 0:
        playDemo0();
        break;
      case 1:
        playDemo1();
      case 2:
        playDemo2();
        break;
    }
    demoIndex += 1;
    if (demoIndex >= 3) {
      currentMode = 0;
    }
    playedDemo = true;
  }
  if (rotaryVal != 0) {
    playedDemo = false;
    rotaryVal = 0;
  } 
}

void playDemo0() {
  currentColor = colors[0];
  for(uint16_t k=0; k<matrix.numPixels(); k++) {
    matrix.setPixelColor(k, colors[4]);
    wHappy.show();
    wBirthday.show();
    wNhan.show();
    matrix.show();
    delay(10);
  }
  delay(2000);
  rotaryVal = 0;
}

void playDemo1() {
  currentColor = colors[0];
  for(uint16_t k=0; k<matrix.numPixels(); k++) {
    matrix.setPixelColor(k, colors[2]);
    wHappy.show();
    wBirthday.show();
    wNhan.show();
    matrix.show();
    delay(10);
  }
  delay(2000);
  rotaryVal = 0;
}
void playDemo2() {
  uint16_t i, j;
  for(i=0; i < matrix.numPixels(); i++) {
    matrix.setPixelColor(i, Wheel(((i * 256 / matrix.numPixels())) & 255));
    matrix.show();
    delay(100);
  }
  for(j=0; j<5*256; j++) { // 5 cycles of all colors on wheel
    for(i=0; i< matrix.numPixels(); i++) {
      matrix.setPixelColor(i, Wheel(((i * 256 / matrix.numPixels()) + j) & 255));
    }
    currentColor = colors[2];
    wHappy.show();
    wBirthday.show();
    wUyen.show();
    matrix.show();
    delay(10);
  }
  currentColor = colors[0];
  currentMode = 0;
}

boolean isDaylightSavingTime() {
  if (month >= 3 && month <= 10) {
    if (month == 3 && date < 8) {
      return false;
    }
    return true;
  }
  return false;
}

