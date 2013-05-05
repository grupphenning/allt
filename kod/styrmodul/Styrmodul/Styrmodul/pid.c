/*
 pid.c
 
 Regulatorn ska k�ras med intervall p� ~40ms
 */ 
void debug(char *str);
#include "pid.h"
#include "Styrmodul.h"

int8_t last_diff_right, last_diff_left, diff_right, diff_left, diff_front, diff_back;
uint16_t k_prop, k_a, k_der;																//Regulatorkonstanter
uint8_t regulator_flag = 0;																//Flagga som till�ter reglering
int16_t max_out, min_out;																//Maxv�rde f�r utsignalen, undvika fel med m�ttad styrsignal.

void init_pid(int16_t max, int16_t min)  //Initiera regulatorn
{
	if (max > min)
	{
		max_out = max;
		min_out = min;
	}
	else {
		max_out = min;
		min_out = max;
	}
}

void update_k_values(uint8_t kp, uint8_t ki, uint8_t kd)		//Uppdatera konstanter. Skala upp 128 f�r h�gre noggrannhet. 3/128 = ~1/40
{
	k_prop = 128*kp;
	k_a = ki;
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
Styrsignal till motorerna, u: (-50 -- 50)

Reglera mot m�lv�gg.
1 h�ger.
0 v�nster.
*/
void regulator(int8_t diff_right, int8_t diff_left, int8_t diff_front, int8_t diff_back)
{	
	dirbits = 3;
	int16_t output_u;
		
	//Unders�k vilken v�gg som �r n�rmast.
	uint8_t targetwall = 0;
	//targetwall = (sensor_buffer[IR_LEFT_BACK]+sensor_buffer[IR_LEFT_FRONT]) > (sensor_buffer[IR_RIGHT_BACK] + sensor_buffer[IR_RIGHT_FRONT])? 1 : 0;
	
	//Dubbelkolla om f�r n�ra en v�gg, eller f�r l�ngt ifr�n.
// 	if (((sensor_buffer[IR_LEFT_FRONT] <= 20) && (sensor_buffer[IR_LEFT_BACK] <= 20)) || sensor_buffer[IR_LEFT_FRONT] > 80) targetwall = 1;
// 	else if (((sensor_buffer[IR_RIGHT_FRONT] <= 20) && (sensor_buffer[IR_RIGHT_BACK] <= 20)) || sensor_buffer[IR_RIGHT_FRONT] > 80) targetwall = 0;
	
	
	if ((sensor_buffer[IR_LEFT_FRONT] <= 25) || (sensor_buffer[IR_LEFT_BACK] <= 25))
	{
		output_u += 128*50;			//Om n�ra en v�gg, dra p� lite extra.
	}
	else if ((sensor_buffer[IR_RIGHT_FRONT] <= 30) || (sensor_buffer[IR_RIGHT_BACK] <= 30))
	{
		output_u -= 128*50;
	}
	else if (targetwall == 1)		//Reglera mot h�ger v�gg
	{
		output_u = k_prop*diff_right;						//P-del
		output_u += k_der*(diff_right - last_diff_right);	//D-del
	}
	else if (targetwall == 0)	//Reglera mot v�nster v�gg
	{
		output_u = -k_prop*diff_left;						//P-del
		output_u -= k_der*(diff_left - last_diff_left);		//D-del
	}
	else						//Styr mot mitten.
	{
		/*output_u += (diff_front+diff_back)*2*/;
	}
	
	last_diff_right = diff_right;
	last_diff_left = diff_left;
	
	output_u = output_u/128;	//Skala ner igen;
	
	if (output_u > max_out) output_u = max_out;
	else if(output_u < min_out) output_u = min_out;
	uint8_t b;
	
	if (output_u > 0)
	{
		b = SPEED - (uint8_t)output_u;
		RIGHT_AMOUNT = b;
		LEFT_AMOUNT = SPEED;
	}
	else if (output_u < 0)
	{
		b = SPEED - (uint8_t)abs(output_u);
		RIGHT_AMOUNT = SPEED;
		LEFT_AMOUNT = b;
	}
	else if (output_u == 0)
	{
		RIGHT_AMOUNT = SPEED;
		LEFT_AMOUNT = SPEED;
	}
}

void pid_timer_init()
{
	//s�tt "Fast PWM mode", med OCRA (OCR0A?) som toppv�rde!
	setbit(TCCR0A, WGM00);
	setbit(TCCR0A, WGM01);
	setbit(TCCR0B, WGM02);
	
	//s�tt klockskalning, fck = f/1024
	setbit(TCCR0B, CS00);
	setbit(TCCR0B, CS02);
	
	//aktivera interrupts, skickas p� overflow
	setbit(TIMSK0, TOIE0);
	
	//8 bit-register
	//frekvensen blir d� 8000000/(1024*255) = 30.63 Hz
	//vilket �r mingr�nsen!
	OCR0A = 255;
}
