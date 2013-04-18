/*
 pid.h
 */ 
#ifndef PID_H
#define PID_H

#include <inttypes.h>
#include "bitmacros.h"

void init_pid(uint16_t time, uint8_t max, uint8_t min);
void update_k_values(uint8_t kp, uint8_t ki, uint8_t kd);
void clear_pid();
void regulator(uint16_t input);

#endif // PID_H