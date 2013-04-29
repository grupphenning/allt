/*
 pid.c
 
 Regulatorn ska köras med intervall på ~40ms
 */ 
#include "pid.h"
#include "Styrmodul.h"

int16_t input, output, last_input;
uint16_t k_prop, k_, k_der;					//Regulatorkonstanter
uint8_t regulator_flag = 0;						//Flagga som tillåter reglering
int16_t max_out, min_out,I_term = 0;						//Maxvärde för utsignalen, undvika fel med mättad styrsignal.

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

void update_k_values(uint8_t kp, uint8_t ki, uint8_t kd)		//Uppdatera konstanter. Skala upp 128 för högre noggrannhet. 3/128 = ~1/40
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

/*
Väggregulatorn
Input:
IR_LEFT_FRONT, IR_RIGHT_FRONT, IR_LEFT_BACK & IR_RIGHT_BACK.
Frontsensorer: (16 -- 150)	Backsensorer: (8 -- 80)
Output:
Styrsignal till motorerna, u.

Reglera mot målvägg.
1 höger.
0 vänster.
*/
void regulator()
{	
	//Undersök vilken vägg som är närmast.
	uint8_t targetwall;
	targetwall = (IR_LEFT_BACK+IR_LEFT_FRONT) > (IR_RIGHT_BACK + IR_RIGHT_FRONT)? 1 : 0;
	
	//Dubbelkolla om för nära en vägg, eller för långt.
	if (((IR_LEFT_FRONT <= 20) && (IR_LEFT_BACK <= 20)) || IR_LEFT_FRONT > 80) targetwall = 1;
	else if (((IR_RIGHT_FRONT <= 20) && (IR_RIGHT_BACK <= 20)) || IR_RIGHT_FRONT > 80) targetwall = 0;
	
	if (targetwall == 1) //Reglera mot höger
	{
					//P-del
					//D-del
	}
	else if (targetwall == 0) //Reglera mot vänster
	{
					//P-del
					//D-del
	}
}