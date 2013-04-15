/*
 * pid.h
 */ 
#ifndef PID_H
#define PID_H
#define F_CPU 8000000UL // 8 MHz Internal oscillator

uint8_t k_prop, k_int, k_der;
int8_t error_sum, last_d_value;
uint16_t cykle_time; //Vilken frekvens reglering körs.
uint8_t max_out; //Maxvärde för utsignalen, undvika fel med mättad styrsignal.


void update_k_values(uint8_t k_prop, uint8_t k_int, uint8_t k_der);

void regulator();

#endif // PID_H