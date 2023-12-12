#include "Arduino.h"
#include "button.h"

// constructor

Button::Button(int pin)
{
  buttonPin = pin;
  setupPin();
}

// automatically set button to input pinMode

void Button::setupPin(){
  pinMode(buttonPin, INPUT);
}

// set onPress to given the desired function

void Button::setOnPress(void (*onPress)()){
  this-> k_onPress = onPress;
};

// check if button is pressed and call given function if so

void Button::update(){
  stateWas = stateIs;
  stateIs = digitalRead(buttonPin);
  if (stateWas == LOW && stateIs == HIGH){
    Serial.print("Button on pin: ");
    Serial.print(buttonPin);
    Serial.println(" pressed");
    k_onPress();
    delay(5);
  }
  else if(stateWas == HIGH && stateIs == LOW){
    delay(5);
  }
}