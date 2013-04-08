/*
 * Kommunikationsmodul.c
 *
 * Created: 4/6/2013 2:39:23 PM
 *  Author: rasme879
 */
//Måste definieras först!
#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/delay.h>
#include "bitmacros.h"
#include <avr/interrupt.h>


// Our dear baud rate
//#define F_CPU 8000000UL
#define USART_BAUDRATE 9600
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)

uint8_t SPI_SlaveReceive()
{
	//SPDR = 0x32;
	/* Wait for reception complete */
	setbit(PORTB, PB1);
	
	while(!(SPSR & (1<<SPIF)));
	clearbit(PORTB, PB1);
	/* Return data register */
	return SPDR;
}
unsigned char USART_Recieve(void);
void USART_Init(unsigned int baud);
void USART_Transmit(unsigned char data);
int main(void)
{
	PORTA = 0;
	setbit(DDRB, PB1);
	
	sei(); //Enable global interrupts

	init_spi();
	SPDR = 0x32;
    init_firefly();

	/*DDRA = 0xff;
	DDRB = 0xff;
	DDRC = 0xff;
	DDRD = 0xff;
	PORTA = 0xff;
	PORTB = 0xff;*/
	//PORTC = 0xff;
	//PORTD = 0xff;
	USART_Init(9600);

	
    while(1)
    {
		USART_Transmit('a');
		//USART_Recieve();
		//clearbit(PORTB, PB0);
		//PORTA = 2;
		//_delay_ms(100);
		//setbit(SPCR, SPE);		//Enables spi
		PORTA = SPI_SlaveReceive();
    }
}


void init_spi()
{
	setbit(SPCR, SPE);		//Enables spi
	//SPCR = 0;
	clearbit(DDRB, PINB4);	// SS är input
	clearbit(DDRB, PINB5);	// MOSI är input
	setbit(DDRB, PINB6);	// MISO är output
	clearbit(DDRB, PINB7);	//CLK är input
	setbit(DDRA, PINA7);	// Avbrottsförfrågan är output
	setbit(PORTA, PINA7);	// 1 = normal, 0 = avbrottsförfrågan
	//setbit(SPCR, SPE);		//Enables spi
	//setbit(SPCR,SPIE);		//Enable interupt
	//setbit(SPCR,SPR0);		//FCK/16
	//setbit(SPCR,SPR1);		
	
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

void USART_Init(unsigned int baud)
{
	//Set baud rate
	UBRRH = (unsigned char)(baud>>8);
	UBRRL = (unsigned char)baud;
	
	//Enable reciever and transmitter
	UCSRB = (1<<RXEN)|(1<<TXEN);
	
	//Set frame format: 8data, 2stop bit
	UCSRC = (1<<URSEL)|(1<<USBS)|(3<<UCSZ0);
}

void USART_Transmit(unsigned char data)
{
	//Wait for empty transmit buffer
	while(!(UCSRA & (1<<UDRE)));
	
	//Put data into buffer, sends the data
	UDR = data;
}

unsigned char USART_Recieve(void)
{
	//Wait for data to be recieved
	while(!(UCSRA & (1<<RXC)));
	
	//Get and return recieved data from buffer
	return UDR;
}

ISR(SPI_STC_vect)
{
	//PORTA=SPI_SlaveReceive();
}	
