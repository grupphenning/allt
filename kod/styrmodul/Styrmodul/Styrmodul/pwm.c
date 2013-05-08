/*
 * pwm.c
 *
 * Created: 5/5/2013 12:52:43 PM
 *  Author: davek282
 */ 
void debug(char *str);

#include "spi.h"
#include <avr/interrupt.h>
#include "bitmacros.h"
#include "pwm.h"
#include "Styrmodul.h"


void pwm_init()
{
	//_delay_ms(1000);
	
	///////////////////////////////////spi_send_byte(0xAA);
	
	//DDRB = 0xFF;
	//DDRC = 0xFF;
	//DDRD = 0xFF;
	
	//sätt riktning på displaystyrpinnar!
	setbit(DDRC, PC6);
	setbit(DDRC, PC7);
	
	
	//sätt riktning på motorer + gripklo
	setbit(DDRB, PB0);
	setbit(DDRB, PB1);
	setbit(DDRD, PD7);
	setbit(DDRD, PD6);
	setbit(DDRD, PD5);
	
	//pwm-styrning för gripklon, pin OC1A, register OCR1A
	TCCR1A = 0;
	setbit(TCCR1A, COM1A1);
	setbit(TCCR1A, WGM11);
	
	TCCR1B = 0;
	setbit(TCCR1B, WGM11);
	setbit(TCCR1B, WGM12);
	setbit(TCCR1B, WGM13);
	
	//sätt klockan på fclk/64!
	setbit(TCCR1B, CS10);
	setbit(TCCR1B, CS11);
	//setbit(TCCR1B, CS12);
	
	
	//TCCR1A = (1 << COM1A1) | (1 << WGM11);
	//TCCR1B = (1 << WGM11) | (1 << WGM13) | (1 << WGM12) | (1 << CS12) | (1 << CS10);
	TIMSK1 = (1 << OCIE1A);  // Enable Interrupt TimerCounter1 Compare Match A (TIMER1_COMPA_vect)
	//ICR1 = 390;
	//ICR1 = 625;
	//ICR1 = 313;
	ICR1 = 625*4;
	//sätt OCR1A också!
	//CLAW_AMOUNT = 10*4;
	//_delay_ms(1000);
	//CLAW_AMOUNT = 78*4;
	
	//pwm-styrning för motorerna, pinne OC2A, register OCR2A för vänster, pinne OC2B, register OCR2B för höger.
	//PB1 DIR höger, PB0 vänster
	TCCR2A=0;
	setbit(TCCR2A, COM2A1);
	setbit(TCCR2A, COM2B1);
	setbit(TCCR2A, WGM20);
	setbit(TCCR2A, WGM21);
	OCR2A = 0;
	OCR2B = 0;
	
	//TCCR2A = (1 << COM2A1) | (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
	
	//sätter på pwm!
	TCCR2B=0;
	//fclk/64, ger 488 Hz ut på PWM
	setbit(TCCR2B, CS20);
	setbit(TCCR2B, CS21);
	
	
	
	//TCCR2B = (1 << CS20);
	////////////////////////////////////////////TIMSK2 = (1 << OCIE2A);
	//fullt ös på OCR=0xff, inget på 0x00
	
	
}


uint8_t dirbits;
void drive_forwards(uint8_t amount)
{
	//sätt ettor (framåt) på DIR-pinnarna
	setbit(PORT_DIR, LEFT_DIR);
	setbit(PORT_DIR, RIGHT_DIR);
	
	dirbits=3;
	LEFT_AMOUNT = amount;
	RIGHT_AMOUNT = amount;
}

void drive_backwards(uint8_t amount)
{
	stop_motors();
	_delay_ms(10);
	
	//sätt nollor (bakåt) på DIR-pinnarna
	clearbit(PORT_DIR, LEFT_DIR);
	clearbit(PORT_DIR, RIGHT_DIR);
	dirbits = 0;
	
	LEFT_AMOUNT = amount;
	RIGHT_AMOUNT = amount;
}

//sväng vänster!
void turn_left(uint8_t amount)
{
	setbit(PORT_DIR, LEFT_DIR);
	setbit(PORT_DIR, RIGHT_DIR);
	
	dirbits=3;
	LEFT_AMOUNT = 60;
	RIGHT_AMOUNT = amount;
}

//sväng höger!
void turn_right(uint8_t amount)
{
	setbit(PORT_DIR, LEFT_DIR);
	setbit(PORT_DIR, RIGHT_DIR);

	dirbits=3;
	LEFT_AMOUNT = amount;
	RIGHT_AMOUNT = 60;
}

//stanna allt!
void stop_motors()
{
	//sätter ingen klocka
	//clearbit(TCCR2B, CS20);
	//clearbit(TCCR2B, CS21);
	//clearbit(TCCR2B, CS22);
	//LEFT_PWM = 0;
	//RIGHT_PWM = 0;
	setbit(PORT_DIR, LEFT_DIR);
	setbit(PORT_DIR, RIGHT_DIR);
	
	dirbits = 3;
	LEFT_AMOUNT = 0;
	RIGHT_AMOUNT = 0;
}

//sväng vänster som en stridsvagn!
void tank_turn_left(uint8_t amount)
{
	//	stop_motors();
	//	_delay_ms(10);
	
	dirbits = 2;
	clearbit(PORT_DIR, LEFT_DIR);
	setbit(PORT_DIR, RIGHT_DIR);
	
	LEFT_AMOUNT = amount;
	RIGHT_AMOUNT = amount;
}


//sväng höger som en stridsvagn!
void tank_turn_right(uint8_t amount)
{
	//	stop_motors();
	//	_delay_ms(10);
	dirbits = 1;
	
	setbit(PORT_DIR, LEFT_DIR);
	clearbit(PORT_DIR, RIGHT_DIR);
	
	LEFT_AMOUNT = amount;
	RIGHT_AMOUNT = amount;
}

void claw_out()
{
	CLAW_AMOUNT = 312;
}

void claw_in()
{
	CLAW_AMOUNT = 63;
}

