/*
 * Kommunikationsmodul.c
 *
 * Created: 4/6/2013 2:39:23 PM
 *  Author: rasme879
 */ 
#include <avr/io.h>
#include <avr/delay.h>
#include "bitmacros.h"

// Our dear baud rate
#define F_CPU 8000000UL
#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

int main(void)
{
//	init_spi();
//    init_firefly();

	DDRA = 0xff;
	DDRB = 0xff;
	DDRC = 0xff;
	DDRD = 0xff;
	PORTA = 0xff;
	PORTB = 0xff;
	PORTC = 0xff;
	PORTD = 0xff;
    while(1)
    {
    }
}

void init_spi()
{
	clearbit(DDRB, PINB4);	// SS är input
	clearbit(DDRB, PORTB6);	// MOSI är input
	setbit(DDRB, PINB5);	// MISO är output
	setbit(DDRA, PINA7);	// Avbrottsförfrågan är output
	setbit(PORTA, PINA7);	// 1 = normal, 0 = avbrottsförfrågan
}

void init_firefly()
{
	setbit(PORTD, PIND1);
	UBRRL = BAUD_PRESCALE;			// De minst signifikanta bitarna av baud scale
	UBRRH = (BAUD_PRESCALE >> 8);	// De mest signifikanta bitarna
	UCSRB = ((1<<TXEN) | (1<<RXEN)); //RXCIE för att enejjbla avbrott
}

void serial_send_byte(uint8_t val)
{
	while((UCSRA &(1<<UDRE)) == 0);	// Vänta på att föregående värde redan skickats
	UDR = val;
}	
