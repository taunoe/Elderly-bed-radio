/**
 * 
 * File: main.cpp
 * Project: "Ühe nupuga raadio"
 * Copyright 2021 Tauno Erik
 * https://taunoerik.art
 * 09. august 2020
 * Last edited: 06.04.2021
 * 
 * Linke:
 * https://github.com/pu2clr/RDA5807
 * https://github.com/CGrassin/M62429_Arduino_Library
 * https://github.com/RobTillaart/M62429
 **/

#include <Arduino.h>
#include <EEPROM.h>
#include <RDA5807.h>
#include <Tauno_rotary_encoder.h>  // Include Rotary Encoder library
#include "Tauno_M62429.h"          // FM62429
#define DEBUG
#include "tauno_debug.h"


// Define Rotary encoder pins
const int RE_SW_PIN  = 2;
const int RE_CLK_PIN = 6;
const int RE_DT_PIN  = 5;

Tauno_Rotary_Encoder RE(RE_SW_PIN, RE_CLK_PIN, RE_DT_PIN);

// Define FM62429 Volume control pins
const int VOLUME_DT_PIN = 9;
const int VOLUME_CLK_PIN = 10;

Tauno_M62429 Volume(VOLUME_DT_PIN, VOLUME_CLK_PIN);

// Define amplifier mute pin
const int MUTE_PIN = 4;

// Define buttons pins
const int NEXT_BUTTON_PIN = 8;
const int SAVE_BUTTON_PIN = 7;

// Store bottons state
bool is_next_button = false;
bool is_save_button = false;



// Define Rotary Encoder button status
volatile bool is_sw_button_on = false;  // 0 = OFF, 1 = ON
// volatile -  it directs the compiler to load the variable
// from RAM and not from a storage register, which is a temporary
// memory location where program variables are stored and manipulated.
// Under certain conditions, the value for a variable stored
// in registers can be inaccurate.
// A variable should be declared volatile whenever its value
// can be changed by something beyond the control of the code
// section in which it appears, such as a concurrently executing thread.
// In the Arduino, the only place that this is likely to occur is
// in sections of code associated with interrupts,
// called an interrupt service routine.

// Define volume levels
const int MIN_VOLUME_LEVEL = 30;  // If lower -> OFF
const int MAX_VOLUME_LEVEL = 100;
int volume_level = 40;  // Default value

uint32_t time_prev = 0;

// Define Radio status
bool is_radio_off = true;

// The frequency you want to select in MHz multiplied by 100.
// uint16_t frequency = 10610;

// Channel is stored on eepromm
unsigned int channel_address = 0;
unsigned int channel = EEPROM.read(channel_address);
unsigned int new_channel = 191;

RDA5807 Raadio;

/**
 * Function to set rotary encoder button status: 0 or 1
 * Handled by interrupt.
 **/
void set_sw_button() {
  if (!is_sw_button_on) {
    is_sw_button_on = true;
    DEBUG_PRINTLN("SW Button ON");
  } else {
    is_sw_button_on = false;
    DEBUG_PRINTLN("SW Button OFF");
  }
}

void volume_down(int step = 1) {
  if (step == 0) {
    step = 1;
  }

  if (volume_level >= MIN_VOLUME_LEVEL) {
    volume_level = volume_level - step;
  }

  if (volume_level < MIN_VOLUME_LEVEL) {
    volume_level = MIN_VOLUME_LEVEL;
  }

  Volume.set(volume_level);

  DEBUG_PRINT(" Step: ");
  DEBUG_PRINT(step);

  DEBUG_PRINT(" Volume: ");
  DEBUG_PRINTLN(volume_level);
}

void volume_up(int step = 1) {
  if (step == 0) {
    step = 1;
  }

  if (volume_level <= MAX_VOLUME_LEVEL) {
    volume_level = volume_level + step;
  }

  if (volume_level > MAX_VOLUME_LEVEL) {
    volume_level = MAX_VOLUME_LEVEL;
  }

  Volume.set(volume_level);

  DEBUG_PRINT(" Step: ");
  DEBUG_PRINT(step);

  DEBUG_PRINT(" Volume: ");
  DEBUG_PRINTLN(volume_level);
}

void amp_on() {
  digitalWrite(MUTE_PIN, LOW);
}

void amp_off() {
  digitalWrite(MUTE_PIN, HIGH);
}

void turn_raadio_on() {
  DEBUG_PRINTLN("\nturn_raadio_on");
  Raadio.setMute(false);

  amp_on();
  is_radio_off = false;

  DEBUG_PRINT("Current Channel: ");
  DEBUG_PRINT(Raadio.getRealChannel());
  DEBUG_PRINT(" Frequency.: ");
  DEBUG_PRINTLN(Raadio.getRealFrequency());
}

void turn_raadio_off() {
  if (!is_radio_off) {
    DEBUG_PRINTLN("\nturn_raadio_off");
    Raadio.setMute(true);
    // Raadio.setVolume(0);
    amp_off();
    is_radio_off = true;
  }
}

/*****************************************************/

void setup() {
  Serial.begin(9600);

  DEBUG_PRINTLN("Raadio starts!");
  DEBUG_PRINT("Compiled: ");
  DEBUG_PRINT(__TIME__);
  DEBUG_PRINT(" ");
  DEBUG_PRINTLN(__DATE__);
  DEBUG_PRINTLN("Made by Tauno Erik.");

  // Start Rotary Encoder
  RE.begin();
  attachInterrupt(digitalPinToInterrupt(RE_SW_PIN), set_sw_button, FALLING);

  pinMode(NEXT_BUTTON_PIN, INPUT);
  pinMode(SAVE_BUTTON_PIN, INPUT);

  // Output Pins setup
  pinMode(MUTE_PIN, OUTPUT);

  Volume.begin();

  amp_off();

  Raadio.setup();
  delay(10);
  Raadio.setMono(true);
  Raadio.setChannel(channel);
  Raadio.setVolume(15);  // Max 15
  turn_raadio_off();
}

void loop() {
  // Read Rotary Encoder rotation direction
  int re_direction = RE.read();

  // Read Rotary Encoder rotation speed:
  uint16_t re_speed = RE.speed();

  // Rotary Encoder turn to right
  if (re_direction == DIR_CW) {
    DEBUG_PRINT(" CW [+]");
    DEBUG_PRINT(" Radio is ");
    if (is_radio_off) {
      DEBUG_PRINT("off.");
    } else {
      DEBUG_PRINT("on.");
    }

    if (is_radio_off) {
      turn_raadio_on();
    }
    volume_up(re_speed);
  } else if (re_direction == DIR_CCW) {
    DEBUG_PRINT(" CCW [-]");
    DEBUG_PRINT(" Radio is ");
    if (is_radio_off) {
      DEBUG_PRINT("off.");
    } else {
      DEBUG_PRINT("on.");
    }

    volume_down(re_speed);
  }

  if (volume_level < MIN_VOLUME_LEVEL + 1) {
    turn_raadio_off();
  }

  /*******************************
   * Read Buttons
   *******************************/
  is_next_button = digitalRead(NEXT_BUTTON_PIN);
  is_save_button = digitalRead(SAVE_BUTTON_PIN);

  if (is_next_button) {
    DEBUG_PRINTLN("Next Button!");
    Raadio.seek(0, 1);  // continue seek, up
    delay(150);
    DEBUG_PRINT("Current Channel: ");
    DEBUG_PRINT(Raadio.getRealChannel());
    DEBUG_PRINT(" Frequency.: ");
    DEBUG_PRINTLN(Raadio.getRealFrequency());
    new_channel = Raadio.getRealChannel();
  }

  if (is_save_button) {
    DEBUG_PRINTLN("Save Button!");
    EEPROM.update(channel_address, new_channel);  // value = 0-255
    // DEBUG_PRINT("Saved: ");
    // DEBUG_PRINTLN(new_channel);
    DEBUG_PRINT(" Read: ");
    DEBUG_PRINTLN(EEPROM.read(channel_address));
    delay(150);
  }

  // If Rotary Encoder sw button is true
  if (is_sw_button_on) {
    //
  } else {
    //
  }
}
