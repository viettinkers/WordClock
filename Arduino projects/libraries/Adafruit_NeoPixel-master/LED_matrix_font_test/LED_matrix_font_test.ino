// DS3231 library
#include <DS3231.h>
#include <Wire.h>

// WS2812B library - Adafruit NeoPixel
#include <Adafruit_NeoPixel.h>
#define PIN 10
#define StartBit 0
#define Base 4
#define Brightness 50
#define test_limit 9
#define strip_length 121

template<int i> struct Fac
{
    static const int result = 11 + 12;
};
 
int f10 = Fac<10>::result;

// DS3231 Setup
DS3231 Clock;
bool Century=false;
bool h12;
bool PM;
byte Index;
int  Year;

int BackwardOut = 9;  
int ForwardOut  = 11;  

int ClickPin  = 12;  // Clicky D11
int ClickOut  = 8;  // Clicky D11

int clickVal = 0;
int eventForward  = 0;
int eventBackward = 0;

int click_now = 0;
int click_old = 1;
int rotary_val = 0;
int test_size = 0;
int test_position = 0;

//WS2812B Setup
Adafruit_NeoPixel strip = Adafruit_NeoPixel(strip_length, PIN, NEO_GRB + NEO_KHZ800);

void Rotated() {
  static uint8_t old_AB = 3;  //lookup table index
  static int8_t encval = 0;   //encoder value  
  static int8_t total_val = 0;   //encoder value  
  static uint8_t enc_states [] = {0,-1,1,0,1,0,0,-1,-1,0,0,1,0,1,-1,0};  //encoder lookup table
  /**/
  old_AB <<=2;  //remember previous state
  old_AB |= ( (PIND >> 2) & 0x03 );
  encval += enc_states[( old_AB & 0x0f )];
  total_val += enc_states[( old_AB & 0x0f )];
  
  /* post "Navigation forward/reverse" event */
  if( encval > 3 ) {  //four steps forward
      rotary_val += 1;
      encval = 0;
  }
  else if( encval < -3 ) {  //four steps backwards
      rotary_val -= 1;
      encval = 0;
  }   
}

void setup() {
   pinMode(2, INPUT);
  attachInterrupt(0, Rotated, CHANGE);
  
  pinMode(3, INPUT);
  attachInterrupt(1, Rotated, CHANGE);
  


  pinMode(ClickPin, INPUT);
  digitalWrite(ClickPin, HIGH);
  pinMode(ClickOut, OUTPUT);
  digitalWrite(ClickOut, LOW);

  pinMode(ForwardOut, OUTPUT);
  digitalWrite(ForwardOut, LOW);
  pinMode(BackwardOut, OUTPUT);
  digitalWrite(BackwardOut, LOW);


  // Start the I2C interface
  Wire.begin();
  // Start the serial interface
  Serial.begin(9600);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  
}

void loop() {
  strip.clear();
  
  // send what's going on to the serial monitor.
  // Start with the year
  click_now = digitalRead(ClickPin);
  if (click_now == 0 && click_old == 1) {
    test_size = (test_size + 1)%test_limit;
  }
  
  click_old = click_now;

  if (test_size > 0) {
    test_position = ( test_position + rotary_val + strip_length) % strip_length;
      
    for (int i=0; i < test_size; i++) {
      strip.setPixelColor(test_position + i, strip.Color(Brightness,Brightness,Brightness));
    }
  }
  
  rotary_val = 0;


  Index = 0;

  Year = 2000 + Clock.getYear();

  push_bits(Year,1024,Index);
  push_bits(Clock.getMonth(Century),4,Index);
  push_bits(Clock.getDate(),16,Index);
  push_bits(Clock.getHour(h12, PM),16,Index);
  push_bits(Clock.getMinute(),16,Index);
  push_bits(Clock.getSecond(),16,Index);
  push_bits(Clock.getTemperature(),16,Index);

  strip.show(); // Initialize all pixels to 'off'  
  delay(10);
}


void push_bits(int Val, int StartVal, byte& Index0) {
  byte remainder;

  while (StartVal > 0) {
    remainder = Val / StartVal;
    switch (remainder) {
    case 1:f
      strip.setPixelColor(StartBit + Index0, strip.Color(Brightness, 0, 0));
      break;
    case 2:
      strip.setPixelColor(StartBit + Index0, strip.Color(0, Brightness, 0));
      break;
    case 3:
      strip.setPixelColor(StartBit + Index0, strip.Color(0, 0, Brightness));
      break;
    default: 
      strip.setPixelColor(StartBit + Index0, strip.Color(Brightness, Brightness, 0));
      break;
    }

    Val = Val % StartVal;
    StartVal = StartVal / Base; 
    Index++;
  }


  Index++;
}


