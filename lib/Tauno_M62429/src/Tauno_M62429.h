/*
 *  File:        Tauno_M62429.h
 *  Last edited: 05.04.2021
 *  GitHub:
 * 
 *  Copyright 2021 Tauno Erik
 *  https://taunoerik.art/
 * 
 */

#ifndef LIB_TAUNO_M62429_SRC_TAUNO_M62429_H_
#define LIB_TAUNO_M62429_SRC_TAUNO_M62429_H_

#include <Arduino.h>

#define CH_ONE        0
#define CH_TWO        1
#define BOTH_CH       0
#define INDIVIDUAL_CH 1


class Tauno_M62429 {
 private:
  // Pins
  uint8_t _DT_PIN;
  uint8_t _CLK_PIN;

 public:
  // Constructor
  Tauno_M62429(
    uint8_t DT_PIN,
    uint8_t CLK_PIN);

  ~Tauno_M62429();

  void begin();
  void write_volume(uint8_t volume);
};

#endif  // LIB_TAUNO_M62429_SRC_TAUNO_M62429_H_
