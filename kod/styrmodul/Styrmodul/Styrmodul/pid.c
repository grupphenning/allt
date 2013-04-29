/*
 pid.c
 
 Regulatorn ska k�ras med intervall p� ~40ms
 */ 
#include "pid.h"
#include "Styrmodul.h"

int16_t input, output, last_input;
uint16_t k_prop, k_, k_der;					//Regulatorkonstanter
uint8_t regulator_flag = 0;						//Flagga som till�ter reglering
int16_t max_out, min_out,I_term = 0;						//Maxv�rde f�r utsignalen, undvika fel med m�ttad styrsignal.

void init_pid(uint16_t time, int16_t max, int16_t min)  //Initiera regulatorn
{
	if (max > min)
	{
		max_out = max;
		min_out = min;
	}
	
	if (output > max_out) output = max_out;
	else if (output < min_out) output = min_out;
}

void update_k_values(uint8_t kp, uint8_t ki, uint8_t kd)		//Uppdatera konstanter. Skala upp 128 f�r h�gre noggrannhet. 3/128 = ~1/40
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

/*
V�ggregulatorn
Input:
IR_LEFT_FRONT, IR_RIGHT_FRONT, IR_LEFT_BACK & IR_RIGHT_BACK.
Frontsensorer: (16 -- 150)	Backsensorer: (8 -- 80)
Output:
Styrsignal till motorerna, u.

Reglera mot m�lv�gg.
1 h�ger.
0 v�nster.
*/
void regulator()
{	
	//Unders�k vilken v�gg som �r n�rmast.
	uint8_t targetwall;
	targetwall = (IR_LEFT_BACK+IR_LEFT_FRONT) > (IR_RIGHT_BACK + IR_RIGHT_FRONT)? 1 : 0;
	
	//Dubbelkolla om f�r n�ra en v�gg, eller f�r l�ngt.
	if (((IR_LEFT_FRONT <= 20) && (IR_LEFT_BACK <= 20)) || IR_LEFT_FRONT > 80) targetwall = 1;
	else if (((IR_RIGHT_FRONT <= 20) && (IR_RIGHT_BACK <= 20)) || IR_RIGHT_FRONT > 80) targetwall = 0;
	
	if (targetwall == 1) //Reglera mot h�ger
	{
					//P-del
					//D-del
	}
	else if (targetwall == 0) //Reglera mot v�nster
	{
					//P-del
					//D-del
	}
}