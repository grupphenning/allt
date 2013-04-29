/*
 pid.c
 
 Regulatorn ska k�ras med intervall p� ~40ms
 */ 
#include "pid.h"
#include "Styrmodul.h"

int8_t last_diff_right, last_diff_left, diff_right, diff_left, diff_front, diff_back;
uint16_t k_prop, k_, k_der;																//Regulatorkonstanter
uint8_t regulator_flag = 0;																//Flagga som till�ter reglering
int16_t max_out, min_out;																//Maxv�rde f�r utsignalen, undvika fel med m�ttad styrsignal.

void init_pid(uint16_t time, int16_t max, int16_t min)  //Initiera regulatorn
{
	if (max > min)
	{
		max_out = max;
		min_out = min;
	}
}

void update_k_values(uint8_t kp, uint8_t ki, uint8_t kd)		//Uppdatera konstanter. Skala upp 128 f�r h�gre noggrannhet. 3/128 = ~1/40
{
	k_prop = 128*kp;
	k_ = ki;
	k_der = 3*kd;	
}

void clear_pid()		//Nollst�ll v�rden.
{
	last_diff_right = 0;
	last_diff_left = 0;
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
Styrsignal till motorerna, u: ()

Reglera mot m�lv�gg.
1 h�ger.
0 v�nster.
*/
void regulator()
{	
	dirbits = 3;
	int16_t output_u;
	diff_right = IR_RIGHT_FRONT - IR_RIGHT_BACK;
	diff_left = IR_LEFT_FRONT - IR_LEFT_BACK;
	diff_front = IR_RIGHT_FRONT - IR_LEFT_FRONT;
	diff_back = IR_RIGHT_BACK - IR_LEFT_BACK;
		
	//Unders�k vilken v�gg som �r n�rmast.
	uint8_t targetwall;
	targetwall = (IR_LEFT_BACK+IR_LEFT_FRONT) > (IR_RIGHT_BACK + IR_RIGHT_FRONT)? 1 : 0;
	
	//Dubbelkolla om f�r n�ra en v�gg, eller f�r l�ngt ifr�n.
	if (((IR_LEFT_FRONT <= 20) && (IR_LEFT_BACK <= 20)) || IR_LEFT_FRONT > 80) targetwall = 1;
	else if (((IR_RIGHT_FRONT <= 20) && (IR_RIGHT_BACK <= 20)) || IR_RIGHT_FRONT > 80) targetwall = 0;
	
<<<<<<< HEAD
	if (targetwall == 1) //Reglera mot h�ger
=======
	if (targetwall == 1)		//Reglera mot h�ger v�gg
	{
		output_u = k_prop*diff_right;						//P-del
		output_u += k_der*(diff_right - last_diff_right);	//D-del
	}
	else if (targetwall == 0)	//Reglera mot v�nster v�gg
	{
		output_u = -k_prop*diff_left;						//P-del
		output_u -= k_der*(diff_left - last_diff_left);		//D-del
	}
	
	last_diff_right = diff_right;
	last_diff_left = diff_left;
	
	if ((IR_LEFT_FRONT <= 20) && (IR_LEFT_BACK <= 20))
	{
		output_u += 0;			//Om n�ra en v�gg, dra p� lite extra.
	}
	if else ((IR_RIGHT_FRONT <= 20) && (IR_RIGHT_BACK <= 20))
>>>>>>> 5108763d23eb052a8be0c26f8586b24c8a236d12
	{
		output_u -= 0;
	}
<<<<<<< HEAD
	else if (targetwall == 0) //Reglera mot v�nster
=======
	else						//Styr mot mitten.
>>>>>>> 5108763d23eb052a8be0c26f8586b24c8a236d12
	{
		output_u += (diff_front+diff_back)/2;
	}
	
	output_u = output_u/128;	//Skala ner igen;
	
	if (output_u > max_out) output_u = max_out;
	else if(output_u < min_out) output_u = min_out;
	uint8_t b;
	
// 	if (output_u > 0)
// 	{
// 		b = SPEED - (uint8_t)output_u;
// 		RIGHT_AMOUNT = b;
// 		LEFT_AMOUNT = SPEED;
// 	}
// 	else if (output_u < 0)
// 	{
// 		b = SPEED - (uint8_t)abs(output_u)
// 		RIGHT_AMOUNT
// 		LEFT_AMOUNT
// 	}
// 	else if (output_u == 0)
// 	{
// 		RIGHT_AMOUNT = SPEED;
// 		LEFT_AMOUNT = SPEED;
// 	}
}