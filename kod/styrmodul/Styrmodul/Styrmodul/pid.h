/*
 pid.h
 */ 
#ifndef PID_H
#define PID_H

#include <inttypes.h>
#include "bitmacros.h"

void init_pid(uint16_t time, int16_t max, int16_t min);
void update_k_values(uint8_t kp, uint8_t ki, uint8_t kd);
void clear_pid();
void regulator(uint16_t input);

#endif // PID_H