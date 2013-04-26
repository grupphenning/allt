/*
 pid.c
 */ 
#include "pid.h"

int16_t input, output;
uint16_t k_prop, k_int, k_der;					//Regulatorkonstanter
static int16_t last_input = 0;				//I-term samt senaste uttryck för derivering.
uint8_t regulator_flag = 0;						//Flagga som tillåter reglering
uint16_t cykle_time, max_I;					    //Vilken intervall reglering körs, begränsa I-term
int16_t max_out, min_out,I_term = 0;						//Maxvärde för utsignalen, undvika fel med mättad styrsignal.

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

void update_k_values(uint8_t kp, uint8_t ki, uint8_t kd)		//Uppdatera konstanter. Skala upp 128 för högre noggrannhet. I-term oftast mindre därför oskalad.
{
	k_prop = 128*kp;
	k_int = ki*cykle_time;
	k_der = 128*kd;	
}

void clear_pid()		//Nollställ värden.
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

int16_t regulator(int16_t input)				//Själva regulatorn
{
	if(!regulator_flag)
	{
		return 0;
	}

	I_term += k_int*input;					//Integreringssumman.
	if (I_term > max_I) I_term = max_I;	//Begränsning för att undvika overflow
	else if (abs(I_term) > max_I) I_term = -max_I;
	
	int16_t dinput = input - last_input;	//Derivering
	last_input = input;
	
	output = k_prop*input /*+ I_term*/ + k_der*dinput/cykle_time;	//Slutsummering och begränsning för att undvika mättad utsignal.
	
	output = output/128;					//Skala ner 128
	
	if (output > max_out) output = max_out;
	else if (output < min_out) output = min_out;		//Begränsa utsignalen
	
	return output;	
}