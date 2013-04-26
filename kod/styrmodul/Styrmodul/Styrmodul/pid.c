/*
 pid.c
 
 Regulatorn ska köras med intervall på ~40ms
 */ 
#include "pid.h"

int16_t input, output, last_input;
uint16_t k_prop, k_, k_der;					//Regulatorkonstanter
uint8_t regulator_flag = 0;						//Flagga som tillåter reglering
int16_t max_out, min_out,I_term = 0;						//Maxvärde för utsignalen, undvika fel med mättad styrsignal.

void init_pid(int16_t max, int16_t min)  //Initiera regulatorn
{
	if (max > min)
	{
		max_out = max;
		min_out = min;
	}
	
	if (output > max_out) output = max_out;
	else if (output < min_out) output = min_out;
}

void update_k_values(uint8_t kp, uint8_t ki, uint8_t kd)		//Uppdatera konstanter. Skala upp 128 för högre noggrannhet.
{
	k_prop = 128*kp;
	k_ = ki;
	k_der = 3*kd;	
}

void clear_pid()		//Nollställ värden.
{
	last_input = 0;
}

void enable_pid()
{
	regulator_flag = 1;
}

void disable_pid()
{
	regulator_flag = 0;
}

void regulator()				//Själva regulatorn, reglerar mot närmaste vägg
{	
	
}