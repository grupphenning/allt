/*
 pid.c
 */ 
#include "pid.h"

int16_t input, output;
uint16_t k_prop, k_int, k_der;					//Regulatorkonstanter
static int16_t last_input = 0;				//I-term samt senaste uttryck f�r derivering.
uint8_t regulator_flag = 0;						//Flagga som till�ter reglering
uint16_t cykle_time, max_I;					    //Vilken intervall reglering k�rs, begr�nsa I-term
int16_t max_out, min_out,I_term = 0;						//Maxv�rde f�r utsignalen, undvika fel med m�ttad styrsignal.

void init_pid(uint16_t time, int16_t max, int16_t min, uint16_t max_Iterm)  //Initiera regulatorn
{
	if (time > 0)  //Kan inte reglera i framtiden. Uppdatera konstanterna med ny tidskonstant.
	{
		cykle_time = time;
	}
	
	if (max > min)
	{
		max_out = max;
		min_out = min;
	}
	
	if (max_Iterm > 0)
	{
		max_I = max_Iterm;
	}
	
	if (output > max_out) output = max_out;
	else if (output < min_out) output = min_out;
	
	if (I_term > max_I) I_term = max_I;
	else if (abs(I_term) > max_I) I_term = -max_I; 
}

void update_k_values(uint8_t kp, uint8_t ki, uint8_t kd)		//Uppdatera konstanter. Skala upp 128 f�r h�gre noggrannhet. I-term oftast mindre d�rf�r oskalad.
{
	k_prop = 128*kp;
	k_int = ki*cykle_time;
	k_der = 128*kd;	
}

void clear_pid()		//Nollst�ll v�rden.
{
	I_term = 0;
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

int16_t regulator(int16_t input)				//Sj�lva regulatorn
{
	if(!regulator_flag)
	{
		return 0;
	}

	I_term += k_int*input;					//Integreringssumman.
	if (I_term > max_I) I_term = max_I;	//Begr�nsning f�r att undvika overflow
	else if (abs(I_term) > max_I) I_term = -max_I;
	
	int16_t dinput = input - last_input;	//Derivering
	last_input = input;
	
	output = k_prop*input /*+ I_term*/ + k_der*dinput/cykle_time;	//Slutsummering och begr�nsning f�r att undvika m�ttad utsignal.
	
	output = output/128;					//Skala ner 128
	
	if (output > max_out) output = max_out;
	else if (output < min_out) output = min_out;		//Begr�nsa utsignalen
	
	return output;	
}