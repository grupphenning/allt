/*
 pid.h
 */ 
#ifndef PID_H
#define PID_H

#include <inttypes.h>
#include "bitmacros.h"

uint16_t input, output;
uint16_t k_prop, k_int, k_der; //Regulatorkonstanter
int8_t I_term = 0, last_input = 0; 
uint16_t cykle_time; //Vilken intervall reglering körs.
uint8_t max_out, min_out; //Maxvärde för utsignalen, undvika fel med mättad styrsignal.

void init_pid(uint16_t time, uint8_t max, uint8_t min);

void update_k_values(uint8_t kp, uint8_t ki, uint8_t kd);

void clear_pid();

void regulator(uint16_t input);

#endif // PID_H