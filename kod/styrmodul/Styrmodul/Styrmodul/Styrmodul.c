/*
 * Styrmodul.c
 *
 * Created: 4/2/2013 2:20:45 PM
 *  Author: klawi021
 */ 

#define F_CPU 8000000UL
 #include <avr/io.h>
#include "bitmacros.h"
#include <avr/delay.h>
#include "display.h"

#define LEFT_DIR PB1
#define RIGHT_DIR PB0
#define PORT_DIR PORTB

#define LEFT_AMOUNT OCR2A
#define RIGHT_AMOUNT OCR2B

#define CLAW_AMOUNT OCR1A

#define LEFT_PWM PD7
#define RIGHT_PWM PD6
#define CLAW PD5
#define PORT_PWM PORTD

#define DISPLAY PORTA
#define DISPLAY_POWER PA2
#define DISPLAY_BLINK PA0
#define DISPLAY_CURSOR PA1
#define DISPLAY_RS PC7
#define DISPLAY_ENABLE PC6




int main(void)
{
	//display ska ut
	DDRA = 0xFF;
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
	
	
	//sätt klockan, f = fclk/1024
	//NEJ! Sätt f = fclk/256
	setbit(TCCR1B, CS10);
	setbit(TCCR1B, CS11);
	//setbit(TCCR1B, CS12);
	
	
	//TCCR1A = (1 << COM1A1) | (1 << WGM11);
	//TCCR1B = (1 << WGM11) | (1 << WGM13) | (1 << WGM12) | (1 << CS12) | (1 << CS10);
	TIMSK1 = (1 << OCIE1A);  // Enable Interrupt TimerCounter1 Compare Match A (TIMER1_COMPA_vect)
	//ICR1 = 390;
	//ICR1 = 625;
	//ICR1 = 313;
	ICR1 = 2500;
	//sätt OCR1A också!
	OCR1A = 300;
	
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
	setbit(TCCR2B, CS20);
	//TCCR2B = (1 << CS20);
	TIMSK2 = (1 << OCIE2A);
	//fullt ös på OCR=0xff, inget på 0x00
	
	init_display();
	clear_screen();
	update();
	send_string("Data:");
	update();
	//tank_turn_left(180,180);
	init_spi();
	uint8_t ch = 'a';
	while(1)
	{
		_delay_ms(100);
		send_character(ch++);	// Ä
		update();
	}
}

void init_spi()
{
	clearbit(DDRD, PIND3);		// Avbrott från kommunikationsenheten är input
	clearbit(DDRD, PIND2);		// Samma för sensorenheten
	setbit(PORTD, PORTD3);		// Slå på internt pull up-motstånd (1 är neutralt läge, 0 är avbrottsförfrågan!)
	setbit(PORTD, PORTD2);		// Samma för sensor
	
	setbit(DDRB, PINB3);	// Slave Select för kommunikationsenheten är output!
	setbit(DDRB, PINB2);	// Schutzstaffel för sensor, också output
	setbit(PORTB, PORTB3);	// 1 är neutral för komm.
	setbit(PORTB, PORTB2);	// Samma för sensor
}


void drive_forwards(uint8_t amount)
{
	LEFT_AMOUNT = amount;
	RIGHT_AMOUNT = amount;
	
	//sätt ettor (framåt) på DIR-pinnarna
	setbit(PORT_DIR, LEFT_DIR);
	setbit(PORT_DIR, RIGHT_DIR);
}

void drive_backwards(uint8_t amount)
{
	LEFT_AMOUNT = amount;
	RIGHT_AMOUNT = amount;
	
	//sätt nollor (bakåt) på DIR-pinnarna
	clearbit(PORT_DIR, LEFT_DIR);
	clearbit(PORT_DIR, RIGHT_DIR);
}

//sväng vänster!
void turn_left(uint8_t amount)
{
	LEFT_AMOUNT = amount;
}

//sväng höger!
void turn_right(uint8_t amount)
{
	RIGHT_AMOUNT = amount;
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
	LEFT_AMOUNT = 0;
	RIGHT_AMOUNT = 0;
}

//sväng vänster som en stridsvagn!
void tank_turn_left(uint8_t amount_l, uint8_t amount_h)
{
	LEFT_AMOUNT = amount_l;
	RIGHT_AMOUNT = amount_h;
	
	clearbit(PORT_DIR, LEFT_DIR);
	setbit(PORT_DIR, RIGHT_DIR);
}


//sväng höger som en stridsvagn!
void tank_turn_right(uint8_t amount)
{
	LEFT_AMOUNT = amount;
	RIGHT_AMOUNT = amount;
	
	setbit(PORT_DIR, LEFT_DIR);
	clearbit(PORT_DIR, RIGHT_DIR);
}

void claw_out()
{
	CLAW_AMOUNT = 64;
}

void claw_in()
{
	CLAW_AMOUNT = 314;
}

void display_on()
{
	DISPLAY = 0;
	clearbit(PORTC, DISPLAY_RS);
	setbit(DISPLAY, DISPLAY_BLINK);
	setbit(DISPLAY, DISPLAY_CURSOR);
	setbit(DISPLAY,DISPLAY_POWER);
	setbit(DISPLAY,PA3);
	display_enable();
	display_set_two_lines();
	display_enable();
	display_home();
	display_enable();
	display_clear();
	display_enable();
	
	display_write();
	display_enable();
}

void display_off()
{
	clearbit(DISPLAY,DISPLAY_POWER);
}

void display_set_two_lines()
{
	DISPLAY = 0;
	setbit(DISPLAY,PA5);
	setbit(DISPLAY,PA4);
	setbit(DISPLAY,PA3);
	
	
	
}


void display_clear()
{
	DISPLAY = 0;
	setbit(DISPLAY, PA0);
}

void display_enable()
{
	setbit(PORTC,DISPLAY_ENABLE);
	_delay_ms(500);
	clearbit(PORTC,DISPLAY_ENABLE);
	_delay_ms(500);
	setbit(PORTC,DISPLAY_ENABLE);
	_delay_ms(500);
}

void display_home()
{
	DISPLAY = 0;
	setbit(DISPLAY,PA1);
}

void display_write()
{
	setbit(PORTC, DISPLAY_RS);
	DISPLAY = 0b01001000;
	
}
