/*
 pid.c
 
 Regulatorn ska k�ras med intervall p� ~40ms
 */ 
#include "pid.h"

int16_t input, output, last_input;
uint16_t k_prop, k_, k_der;					//Regulatorkonstanter
uint8_t regulator_flag = 0;						//Flagga som till�ter reglering
int16_t max_out, min_out,I_term = 0;						//Maxv�rde f�r utsignalen, undvika fel med m�ttad styrsignal.

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

void update_k_values(uint8_t kp, uint8_t ki, uint8_t kd)		//Uppdatera konstanter. Skala upp 128 f�r h�gre noggrannhet.
{
	k_prop = 128*kp;
	k_ = ki;
	k_der = 3*kd;	
}

void clear_pid()		//Nollst�ll v�rden.
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

void regulator()				//Sj�lva regulatorn, reglerar mot n�rmaste v�gg
{	
	
}