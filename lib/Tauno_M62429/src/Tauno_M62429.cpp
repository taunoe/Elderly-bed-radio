/*
 *  File:        Tauno_M62429.cpp
 *  Last edited: 05.04.2021
 *  GitHub:
 * 
 *  Copyright 2021 Tauno Erik
 *  https://taunoerik.art/
 * 
 * Links:
 * - https://www.arduino.cc/en/Reference/APIStyleGuide
 * - https://www.arduino.cc/en/Reference/StyleGuide
 */

#include <Arduino.h>
#include "Tauno_M62429.h"

// Constructor
Tauno_M62429::Tauno_M62429(
  uint8_t DT_PIN,
  uint8_t CLK_PIN)
  : _DT_PIN(DT_PIN),
    _CLK_PIN(CLK_PIN)
  {}

Tauno_M62429::~Tauno_M62429() {
  // Cleanup
}

void Tauno_M62429::begin() {
  pinMode(_DT_PIN, OUTPUT);
  pinMode(_CLK_PIN, OUTPUT);
}

void Tauno_M62429::write_volume(uint8_t volume) {
  uint16_t data = 0;  // control word is built by OR-ing in the bits

  // generate 10 bits of data
  data |= (0 << 0);  // D0 (channel select: 0=ch1, 1=ch2)
  data |= (0 << 1);  // D1 (individual/both select: 0=both, 1=individual)

  data |= ((21 - (volume / 4)) << 2);
  // D2...D6 (ATT1: coarse attenuator: 0,-4dB,-8dB, etc.. steps of 4dB)
  data |= ((3 - (volume % 4)) << 7);
  // D7...D8 (ATT2: fine attenuator: 0...-1dB... steps of 1dB)
  data |= (0b11 << 9);  // D9...D10 // D9 & D10 must both be 1

  for (uint8_t bits = 0; bits < 11; bits++) {  // send out 11 control bits
    delayMicroseconds(2);  // pg.4 - M62429P/FP datasheet
    digitalWrite(_DT_PIN, 0);
    delayMicroseconds(2);
    digitalWrite(_CLK_PIN, 0);
    delayMicroseconds(2);
    digitalWrite(_DT_PIN, (data >> bits) & 0x01);
    delayMicroseconds(2);
    digitalWrite(_CLK_PIN, 1);
  }

  delayMicroseconds(2);
  digitalWrite(_DT_PIN, 1);  // final clock latches data in
  delayMicroseconds(2);
  digitalWrite(_CLK_PIN, 0);

  // return data;
  // return bit pattern in case you want it :)
}
