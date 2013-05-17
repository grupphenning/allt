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
	
	TIMSK1 = (1 << OCIE1A);  // Enable Interrupt TimerCounter1 Compare Match A (TIMER1_COMPA_vect)
	ICR1 = 625*4;
	
	//pwm-styrning för motorerna, pinne OC2A, register OCR2A för vänster, pinne OC2B, register OCR2B för höger.
	//PB1 DIR höger, PB0 vänster
	TCCR2A=0;
	setbit(TCCR2A, COM2A1);
	setbit(TCCR2A, COM2B1);
	setbit(TCCR2A, WGM20);
	setbit(TCCR2A, WGM21);
	OCR2A = 0;
	OCR2B = 0;
		
	//Sätter på pwm!
	TCCR2B=0;
	//fclk/64, ger 488 Hz ut på PWM
	setbit(TCCR2B, CS20);
	setbit(TCCR2B, CS21);
}


uint8_t dirbits;

/*
Kör framåt, genom att sätta DIR-pinnarna till 1 (framåt)
och pulsbreddsregistren till värdet amount, mellan 0 och 255.
*/
void drive_forwards(uint8_t amount)
{
	//sätt ettor (framåt) på DIR-pinnarna
	setbit(PORT_DIR, LEFT_DIR);
	setbit(PORT_DIR, RIGHT_DIR);
	
	dirbits=3;
	LEFT_AMOUNT = amount - SPEED_OFFSET;
	RIGHT_AMOUNT = amount;
}

/*
Kör bakåt, genom att sätta DIR-pinnarna till 0 (bakåt)
och pulsbreddsregistren till värdet amount, mellan 0 och 255.
*/
void drive_backwards(uint8_t amount)
{
	stop_motors();
	spi_delay_ms(10);
	
	//sätt nollor (bakåt) på DIR-pinnarna
	clearbit(PORT_DIR, LEFT_DIR);
	clearbit(PORT_DIR, RIGHT_DIR);
	dirbits = 0;
	
	LEFT_AMOUNT = amount - SPEED_OFFSET;
	RIGHT_AMOUNT = amount;
}

/*
Kör vänster, genom att sätta DIR-pinnarna till 1 (framåt)
och pulsbreddsregistren till ett mindre värde på vänstra hjulparet
så den svänger åt vänster.
*/
void turn_left(uint8_t amount)
{
	setbit(PORT_DIR, LEFT_DIR);
	setbit(PORT_DIR, RIGHT_DIR);
	
	dirbits=3;
	LEFT_AMOUNT = 60;
	RIGHT_AMOUNT = amount;
}

/*
Kör höger, genom att sätta DIR-pinnarna till 1 (framåt)
och pulsbreddsregistren till ett mindre värde på högra hjulparet
så den svänger åt höger.
*/
void turn_right(uint8_t amount)
{
	setbit(PORT_DIR, LEFT_DIR);
	setbit(PORT_DIR, RIGHT_DIR);

	dirbits=3;
	LEFT_AMOUNT = amount - SPEED_OFFSET;
	RIGHT_AMOUNT = 60;
}

/*
Stoppa motorerna, genom att sätta hastigheten (pulsbredden)
på båda registren till 0.
*/
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

/*
Sväng vänster som en stridsvagn, genom att sätta en etta
respektive en nolla på höger respektive vänster hjulpar,
så den svänger på stället.
*/
void tank_turn_left(uint8_t amount)
{
	//	stop_motors();
	//	_delay_ms(10);
	
	dirbits = 2;
	clearbit(PORT_DIR, LEFT_DIR);
	setbit(PORT_DIR, RIGHT_DIR);
	
	LEFT_AMOUNT = amount - SPEED_OFFSET;
	RIGHT_AMOUNT = amount;
}


/*
Sväng höger som en stridsvagn, genom att sätta en etta
respektive en nolla på höger respektive vänster hjulpar,
så den svänger på stället.
*/
void tank_turn_right(uint8_t amount)
{
	//	stop_motors();
	//	_delay_ms(10);
	dirbits = 1;
	
	setbit(PORT_DIR, LEFT_DIR);
	clearbit(PORT_DIR, RIGHT_DIR);
	
	LEFT_AMOUNT = amount - SPEED_OFFSET;
	RIGHT_AMOUNT = amount;
}

/*
Öppnar gripklon genom att sätta pulsbredden till ett värde
enligt specifikation.
*/
void claw_out()
{
	CLAW_AMOUNT = 312;
}

/*
Stänger gripklon genom att sätta pulsbredden till ett värde
enligt specifikation.
*/
void claw_in()
{
	CLAW_AMOUNT = 156;
}

