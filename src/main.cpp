/**
 * Tsenter vanakeste voodi raadio ühe nupuga kontrollimine.
 * Tauno Erik
 * 07. august 2020
 * 
 **/
#include <Arduino.h>
#include <RDA5807.h>  // https://github.com/pu2clr/RDA5807
// #include "M62429.h"   // https://github.com/CGrassin/M62429_Arduino_Library
// Ei saanud tööle:
// Digital volume // https://github.com/RobTillaart/M62429

#define DEBUG // Enable debug info serial print
#ifdef DEBUG
  #define DEBUG_PRINT(x)  Serial.print(x)
  #define DEBUG_PRINTLN(x)  Serial.println(x)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
#endif

// Rotary encoder Input Pins
const int RE_SW_PIN = 2;
const int RE_CLK_PIN = 6;
const int RE_DT_PIN = 5;

// Output Pin
const int MUTE_PIN = 4;
// Input Pins
const int NEXT_BUTTON_PIN = 7;
const int SAVE_BUTTON_PIN = 8;

// FM62429 Volume control
const int VOLUME_DT_PIN = 9;
const int VOLUME_CLK_PIN = 10;


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

RDA5807 Raadio;

// M62429 Volume(VOLUME_CLK_PIN, VOLUME_DT_PIN);
int volume_level = 20; // 0 is max, 83 in minimum volume (-83dB).


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



/**
 * Returns true if I2C device found
 */
bool checkI2C() {
  Wire.begin();
  byte error, address;
  int nDevices;
  Serial.println("I2C bus Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("\nI2C device found at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
      nDevices++;
    }
    else if (error==4) {
      Serial.print("\nUnknow error at address 0x");
      if (address<16) {
        Serial.print("0");
      }
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0) {
    Serial.println("No I2C devices found\n");
    return false;
  }
  else {
    Serial.println("done\n");
    return true;
  }
}

void setVolume (uint8_t volume)
{
	uint8_t bits;
	uint16_t data = 0; // control word is built by OR-ing in the bits

	// convert attenuation to volume
	volume = (volume > 100) ? 0 : (((volume * 83) / -100) + 83); // remember 0 is full volume!
       // generate 10 bits of data
	data |= (0 << 0); // D0 (channel select: 0=ch1, 1=ch2)
	data |= (0 << 1); // D1 (individual/both select: 0=both, 1=individual)
	data |= ((21 - (volume / 4)) << 2); // D2...D6 (ATT1: coarse attenuator: 0,-4dB,-8dB, etc.. steps of 4dB)
	data |= ((3 - (volume % 4)) << 7); // D7...D8 (ATT2: fine attenuator: 0...-1dB... steps of 1dB)
	data |= (0b11 << 9); // D9...D10 // D9 & D10 must both be 1

	for (bits = 0; bits < 11; bits++) { // send out 11 control bits
		delayMicroseconds (2); // pg.4 - M62429P/FP datasheet
		digitalWrite (VOLUME_DT_PIN, 0);
		delayMicroseconds (2);
		digitalWrite (VOLUME_CLK_PIN, 0);
		delayMicroseconds (2);
		digitalWrite (VOLUME_DT_PIN, (data >> bits) & 0x01);
		delayMicroseconds (2);
		digitalWrite (VOLUME_CLK_PIN, 1);
	}
	delayMicroseconds (2);
	digitalWrite (VOLUME_DT_PIN, 1); // final clock latches data in
	delayMicroseconds (2);
	digitalWrite (VOLUME_CLK_PIN, 0);
	//return data; // return bit pattern in case you want it :)
}


/*****************************************************/

void setup() {
  Serial.begin(9600);
  DEBUG_PRINTLN("Raadio starts!");
  DEBUG_PRINT("Compiled: ");
  DEBUG_PRINT(__TIME__);
  DEBUG_PRINT(" ");
  DEBUG_PRINTLN(__DATE__);
  DEBUG_PRINTLN("by Tauno Erik");

  // Input Pins setup
  pinMode(RE_CLK_PIN, INPUT);
  pinMode(RE_DT_PIN, INPUT);
  pinMode(RE_SW_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(RE_SW_PIN), set_button, FALLING);

  pinMode(NEXT_BUTTON_PIN, INPUT);
  pinMode(SAVE_BUTTON_PIN, INPUT);

  // Output Pins setup
  pinMode(MUTE_PIN, OUTPUT);
  pinMode(VOLUME_CLK_PIN, OUTPUT);
  pinMode(VOLUME_DT_PIN, OUTPUT);

  // Activ low
  digitalWrite(MUTE_PIN, LOW); // amp

/*
  if (!checkI2C())
  {
      Serial.println("\nCheck your circuit!");
      while(1);
  }
  */

  Raadio.setup();
  Raadio.setMono(true);
  Raadio.setVolume(15); // 0 - 15  
  delay(500);

  DEBUG_PRINTLN("set 106.1MHz");
  Raadio.setFrequency(10610); // The frequency you want to select in MHz multiplied by 100.
  
  DEBUG_PRINT("Current Channel: ");
  DEBUG_PRINTLN(Raadio.getRealChannel());

  DEBUG_PRINT("get Frequency.: ");
  DEBUG_PRINTLN(Raadio.getRealFrequency());
  
  // Mute
  //Raadio.setMute(true);
  //Raadio.setMute(false);

  // Seek
  //Raadio.seek(0,1);
  setVolume(volume_level);
  delay(5);
}

void loop() {

  int rotate = get_encoder_turn();
  // Left turn
  if (rotate > 0) {  
    DEBUG_PRINT(rotate);
    DEBUG_PRINTLN(" = Left");
    if (volume_level > 0) {
      --volume_level;
      setVolume(volume_level);
      delay(5);
      DEBUG_PRINTLN(volume_level);
    }
  }
  // Right turn
  else if (rotate < 0) {
    DEBUG_PRINT(rotate);
    DEBUG_PRINTLN(" = Right");
    if (volume_level < 100) {
      ++volume_level;
      setVolume(volume_level);
      delay(5);
      DEBUG_PRINTLN(volume_level);
    }
  }

  if (button_on) {
    
  }
  else {
    //DEBUG_PRINT("Low");
  }
  
/*
  if (Serial.available()){
     if (Serial.read() == '0'){
      down();
      Serial.print("-1");
     }
     else {
      up();
      Serial.print("+1");
      
     }
  }
  */

 

}