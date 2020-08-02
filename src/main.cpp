/**
 * Tsenter vanakeste voodi raadio ühe nupuga kontrollimine.
 * Tauno Erik
 * 30. juuli 2020
 **/
#include <Arduino.h>

#define DEBUG // Enable debug info serial print

#ifdef DEBUG
  #define DEBUG_PRINT(x)  Serial.print(x)
  #define DEBUG_PRINTLN(x)  Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

// Input Pins
const int RE_CLK_PIN = 5;
const int RE_DT_PIN = 6;
const int RE_SW_PIN = 2;

// Output Pins
const int UP_PIN = 3;
const int DOWN_PIN = 4;
const int ONOFF_PIN = 7;

const int BUTTON_DELAY = 10;

volatile int button_on = 0; // 0 = OFF, 1 = ON
// volatile -  it directs the compiler to load the variable 
// from RAM and not from a storage register, which is a temporary 
// memory location where program variables are stored and manipulated. 
// Under certain conditions, the value for a variable stored in registers can be inaccurate.
// A variable should be declared volatile whenever its value 
// can be changed by something beyond the control of the code 
// section in which it appears, such as a concurrently executing thread. 
// In the Arduino, the only place that this is likely to occur is 
// in sections of code associated with interrupts, called an interrupt service routine.

/**
 * Function to set rotary encoder button status: 0 or 1
 * Handled by interrupt.
 **/
void set_button() {
  if (button_on == 0) {
    button_on = 1;
    DEBUG_PRINTLN("Button ON");
  }
  else {
    button_on = 0;
    DEBUG_PRINTLN("Button OFF");
  }
}


/**
 * Function to read rotary encoder turn direction
 * Returns: -1, 0, or 1
 * -1 == Right turn
 *  1 == Left turn
 *  0 == No turn
 **/
int get_encoder_turn() {
  /**
   * The static keyword is used to create variables that are visible to only one function. 
   * However unlike local variables that get created and destroyed every time a function is called, 
   * static variables persist beyond the function call, preserving their data between function calls.
   * Variables declared as static will only be created and initialized the first time a function is called. 
   **/
  static int old_a = 1;
  static int old_b = 1;
  int result = 0;
  int new_a = digitalRead(RE_CLK_PIN);
  int new_b = digitalRead(RE_DT_PIN);

  // If the value of CLK pin or the DT pin has changed
  if (new_a != old_a || new_b != old_b) {
    if (old_a == 1 && new_a == 0) {
      result = (old_b * 2 - 1);
    }
  }

  old_a = new_a;
  old_b = new_b;

  return result;
}

void press_button(int pin) {
  digitalWrite(pin, HIGH);
  delay(BUTTON_DELAY);
  digitalWrite(pin, LOW);
}


void setup() {
  // Input Pins setup
  pinMode(RE_CLK_PIN, INPUT);
  pinMode(RE_DT_PIN, INPUT);
  pinMode(RE_SW_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RE_SW_PIN), set_button, FALLING);

  // Output Pins setup
  pinMode(UP_PIN, OUTPUT);
  pinMode(DOWN_PIN, OUTPUT);
  pinMode(ONOFF_PIN, OUTPUT);

  // Set all output pins low
  digitalWrite(UP_PIN, LOW);
  digitalWrite(DOWN_PIN, LOW);
  digitalWrite(ONOFF_PIN, LOW);

  Serial.begin(9600);
  DEBUG_PRINT("Compiled: ");
  DEBUG_PRINT(__TIME__);
  DEBUG_PRINT(" ");
  DEBUG_PRINTLN(__DATE__);
  DEBUG_PRINTLN("by Tauno Erik");
}

void loop() {

  int rotate = get_encoder_turn();
  // Left turn
  if (rotate > 0) {  
    press_button(UP_PIN);

    DEBUG_PRINT(rotate);
    DEBUG_PRINTLN(" = Left");
  }
  // Right turn
  else if (rotate < 0) {
    press_button(DOWN_PIN);

    DEBUG_PRINT(rotate);
    DEBUG_PRINTLN(" = Right");
  }

  if (button_on) {
    digitalWrite(ONOFF_PIN, HIGH);
  }
  else {
    digitalWrite(ONOFF_PIN, LOW);
  }

}