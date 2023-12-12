#include "button.h"
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

AudioInputI2S i2s1;                 //xy=241,89
AudioEffectGranular granular1;      //xy=442,95
AudioEffectBitcrusher bitcrusher1;  //xy=634,196
AudioMixer4 mixer1;                 //xy=710,54
AudioOutputI2S i2s2;                //xy=843,96
AudioConnection patchCord1(i2s1, 0, granular1, 0);
AudioConnection patchCord2(granular1, 0, mixer1, 0);
AudioConnection patchCord3(granular1, bitcrusher1);
AudioConnection patchCord4(bitcrusher1, 0, mixer1, 1);
AudioConnection patchCord5(mixer1, 0, i2s2, 0);
AudioConnection patchCord6(mixer1, 0, i2s2, 1);
AudioControlSGTL5000 sgtl5000_1;  //xy=424,319



// display definitions
#define SCREEN_WIDTH 128     // OLED display width, in pixels
#define SCREEN_HEIGHT 64     // OLED display height, in pixels
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES 10  // Number of snowflakes in the animation example

// create button using button class for granulizer
Button granularControlButton(32);


// pots
int pots[3] = { A10, A11, A12 };

// granulizer variables section

int pitchShift = 1;
int freeze = 2;
int grainStop = 0;
int grainMode = grainStop;
float grainSpeed = 1.0;
int grainSize = 200;
float dryWet = 0;
int freezeButton1 = 32;
bool buttonState = false;
bool lastButtonState = false;
bool freezeButtonState = false;

#define GRANULAR_MEMORY_SIZE 12800  // enough for 290 ms at 44.1 kHz
int16_t granularMemory[GRANULAR_MEMORY_SIZE];

int mappedPotValue = 0;

void setup() {
  Serial.begin(9600);
  AudioMemory(1000);

  //displayStuff
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;  // Don't proceed, loop forever
  }
  display.display();
  delay(2000);  // Pause for 2 seconds
  // Clear the buffer
  display.clearDisplay();
  // Draw a single pixel in white
  display.drawPixel(10, 10, SSD1306_WHITE);
  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
  delay(2000);
  //set button functions
  displaySequence();

  sgtl5000_1.enable();
  sgtl5000_1.volume(1.00);
  granular1.begin(granularMemory, GRANULAR_MEMORY_SIZE);
  granularControlButton.setOnPress(granulizerModeSelect);

}


void loop() {
  bruhCrush();
  updateButtons();
  updatePots();
  updateGranularSpeed();
  mixer1.gain(0, 1.0 - dryWet);
  mixer1.gain(1, dryWet);
  //displaySequence();
}


void displaySequence() {
  testdrawcircle();
  testfillcircle();
  testdrawline();
  testscrolltext();
}

void testdrawline() {
  int16_t i;

  display.clearDisplay(); // Clear display buffer

  for(i=0; i<display.width(); i+=4) {
    display.drawLine(0, 0, i, display.height()-1, SSD1306_WHITE);
    display.display(); // Update screen with each newly-drawn line
    delay(1);
  }
  for(i=0; i<display.height(); i+=4) {
    display.drawLine(0, 0, display.width()-1, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }
  delay(50);

}

void testdrawcircle(void) {
  display.clearDisplay();

  for (int16_t i = 0; i < max(display.width(), display.height()) / 2; i += 2) {
    display.drawCircle(display.width() / 2, display.height() / 2, i, SSD1306_WHITE);
    display.display();
    delay(1);
  }

  delay(50);
}

void testfillcircle(void) {
  display.clearDisplay();

  for (int16_t i = max(display.width(), display.height()) / 2; i > 0; i -= 3) {
    // The INVERSE color is used so circles alternate white/black
    display.fillCircle(display.width() / 2, display.height() / 2, i, SSD1306_INVERSE);
    display.display();  // Update screen with each newly-drawn circle
    delay(1);
  }

  delay(50);
}

void testscrolltext(void) {
  display.clearDisplay();

  display.setTextSize(2);  // Draw 2X-scale text

  display.setTextColor(SSD1306_WHITE);


  display.setCursor(11, 5);
  display.println(F("troxell"));
  display.display();  // Show initial text
  delay(100);

  // Scroll in various directions, pausing in-between:
  display.startscrollright(0x00, 0x0F);
  delay(5000);
  display.stopscroll();
  delay(0);
  display.startscrollright(0x00, 0x0F);
  delay(250);
  display.stopscroll();
  delay(0);
  display.startscrollleft(0x00, 0x0F);
  delay(250);
  display.stopscroll();
  delay(0);
  display.startscrollright(0x00, 0x0F);
  delay(250);
  display.stopscroll();
  delay(0);
  display.startscrollleft(0x00, 0x0F);
  delay(250);
  display.stopscroll();
  delay(0);
  display.startscrollright(0x00, 0x0F);
  delay(250);
  display.stopscroll();
  delay(0);
  display.startscrollleft(0x00, 0x0F);
  delay(250);
  display.stopscroll();
  delay(0);
}




void bruhCrush() {
  bitcrusher1.bits(8);
  bitcrusher1.sampleRate(44100);
}


void granulizerModeSelect() {
  // pitch shift mode -> freeze mode

  /*
  if (grainMode == pitchShift){
    granular1.beginFreeze(grainSize);
    grainMode = freeze;
    Serial.print("Freeze Mode - Grain Size: ");
    Serial.println(grainSize);
  } */

  // stop -> pitch shift mode
  if (grainMode == grainStop) {
    granular1.beginFreeze(grainSize);
    grainMode = freeze;
    Serial.print("Freeze Mode - Grain Size: ");
    Serial.println(grainSize);
  }
  // freeze mode -> stop
  else if (grainMode == freeze) {
    granular1.stop();
    grainMode = grainStop;
    Serial.println("The granulizer has stopped...");
  }
}

void updateButtons() {
  granularControlButton.update();
}

// read pot values

void updatePots() {
  grainSize = map(analogRead(pots[0]), 0, 1023, 1, 280);

  grainSpeed = map(analogRead(pots[1]), 0, 1023, 0.125, 8.0);

  dryWet = float(analogRead(pots[2])) / 1023;
  Serial.println(dryWet);
}

void updateGranularSpeed() {
  granular1.setSpeed(grainSpeed);
}






