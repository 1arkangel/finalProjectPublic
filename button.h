#ifndef Button_h
#define Button_h

class Button {
private:
  int buttonPin;
  bool stateWas = LOW;
  bool stateIs = LOW;
  void (*k_onPress)();

public:

  Button(int pin);
  void setupPin();
  void setOnPress(void(*onPress)());
  void update();
};


#endif 