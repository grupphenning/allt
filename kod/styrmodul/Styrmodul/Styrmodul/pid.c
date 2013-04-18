/*
 pid.c
 */ 
#include "pid.h"

uint16_t input, output;
uint16_t k_prop, k_int, k_der; //Regulatorkonstanter
int8_t I_term = 0, last_input = 0;
uint16_t cykle_time; //Vilken intervall reglering körs.
uint8_t max_out, min_out; //Maxvärde för utsignalen, undvika fel med mättad styrsignal.

void init_pid(uint16_t time, uint8_t max, uint8_t min)
{
	if (time > 0)
	{
		uint16_t	temp = time/cykle_time;
		k_int *= temp;
		k_der /= temp;
		cykle_time = time;
	}
	
	if (max > min)
	{
		max_out = max;
		min_out = min;
	}
	
	if (output > max_out) output = max_out;
	else if (output < min_out) output = min_out;
	
	if (I_term > max_out) I_term = max_out;
	else if (I_term < min_out) I_term = min_out;
}

void update_k_values(uint8_t kp, uint8_t ki, uint8_t kd)
{
	k_prop = 128*kp;
	k_int = ki*cykle_time;
	k_der = 128*kd/cykle_time;	
}

void clear_pid()
{
	I_term = 0;
}

void regulator(uint16_t input)
{
	I_term += k_int*input;
	if (I_term > max_out) I_term = max_out;
	else if (I_term < min_out) I_term = min_out;
	
	uint16_t dinput = input - last_input;
	
	output = k_prop*input + I_term - k_der*dinput;
	if (output > max_out) output = max_out;
	else if (output < min_out) output = min_out;
	
	output = output/128;
}