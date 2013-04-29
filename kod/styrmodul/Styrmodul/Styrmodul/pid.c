/*
 pid.c
 
 Regulatorn ska köras med intervall på ~40ms
 */ 
#include "pid.h"
#include "Styrmodul.h"

int8_t last_diff_right, last_diff_left, diff_right, diff_left, diff_front, diff_back;
uint16_t k_prop, k_, k_der;																//Regulatorkonstanter
uint8_t regulator_flag = 0;																//Flagga som tillåter reglering
int16_t max_out, min_out;																//Maxvärde för utsignalen, undvika fel med mättad styrsignal.

void init_pid(uint16_t time, int16_t max, int16_t min)  //Initiera regulatorn
{
	if (max > min)
	{
		max_out = max;
		min_out = min;
	}
}

void update_k_values(uint8_t kp, uint8_t ki, uint8_t kd)		//Uppdatera konstanter. Skala upp 128 för högre noggrannhet. 3/128 = ~1/40
{
	k_prop = 128*kp;
	k_ = ki;
	k_der = 3*kd;	
}

void clear_pid()		//Nollställ värden.
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
Väggregulatorn
Input:
IR_LEFT_FRONT, IR_RIGHT_FRONT, IR_LEFT_BACK & IR_RIGHT_BACK.
Frontsensorer: (16 -- 150)	Backsensorer: (8 -- 80)
Output:
Styrsignal till motorerna, u: ()

Reglera mot målvägg.
1 höger.
0 vänster.
*/
void regulator()
{	
	dirbits = 3;
	int16_t output_u;
	diff_right = IR_RIGHT_FRONT - IR_RIGHT_BACK;
	diff_left = IR_LEFT_FRONT - IR_LEFT_BACK;
	diff_front = IR_RIGHT_FRONT - IR_LEFT_FRONT;
	diff_back = IR_RIGHT_BACK - IR_LEFT_BACK;
		
	//Undersök vilken vägg som är närmast.
	uint8_t targetwall;
	targetwall = (IR_LEFT_BACK+IR_LEFT_FRONT) > (IR_RIGHT_BACK + IR_RIGHT_FRONT)? 1 : 0;
	
	//Dubbelkolla om för nära en vägg, eller för långt ifrån.
	if (((IR_LEFT_FRONT <= 20) && (IR_LEFT_BACK <= 20)) || IR_LEFT_FRONT > 80) targetwall = 1;
	else if (((IR_RIGHT_FRONT <= 20) && (IR_RIGHT_BACK <= 20)) || IR_RIGHT_FRONT > 80) targetwall = 0;
	
<<<<<<< HEAD
	if (targetwall == 1) //Reglera mot höger
=======
	if (targetwall == 1)		//Reglera mot höger vägg
	{
		output_u = k_prop*diff_right;						//P-del
		output_u += k_der*(diff_right - last_diff_right);	//D-del
	}
	else if (targetwall == 0)	//Reglera mot vänster vägg
	{
		output_u = -k_prop*diff_left;						//P-del
		output_u -= k_der*(diff_left - last_diff_left);		//D-del
	}
	
	last_diff_right = diff_right;
	last_diff_left = diff_left;
	
	if ((IR_LEFT_FRONT <= 20) && (IR_LEFT_BACK <= 20))
	{
		output_u += 0;			//Om nära en vägg, dra på lite extra.
	}
	if else ((IR_RIGHT_FRONT <= 20) && (IR_RIGHT_BACK <= 20))
>>>>>>> 5108763d23eb052a8be0c26f8586b24c8a236d12
	{
		output_u -= 0;
	}
<<<<<<< HEAD
	else if (targetwall == 0) //Reglera mot vänster
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