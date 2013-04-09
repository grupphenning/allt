/*
 * Styrmodul.c
 *
 * Created: 4/2/2013 2:20:45 PM
 *  Author: klawi021 et al
 */ 

#define F_CPU 8000000UL
#include <avr/io.h>
#include "bitmacros.h"
#include <avr/delay.h>
#include "display.h"
#include <avr/interrupt.h>

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


uint8_t test;
uint8_t spi_data_from_comm;

int main(void)
{
	spi_init();
	//aktivera interrupt på INT0 och INT1
	setbit(EIMSK, INT0);
	setbit(EIMSK, INT1);

	//aktivera interrupt-request på "any change"
	setbit(EICRA, ISC00);
	//aktivera global interrupts
	sei();

	
	spi_send_byte(0xAA);
	
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
	send_string("Data: ");
	update();
	//tank_turn_left(180,180);
	//init_spi();
	uint8_t ch = 'a';
	
	while(1)
	{
		_delay_ms(100);
		send_character(ch++);	//Ä
		update();
		
		//spi_send_byte(0xD2);
		
		/*
		//claw_in();
		//_delay_ms(5000);
		claw_out();
		setbit(PORTD, PD0);
		_delay_ms(1000);
		claw_in();
		clearbit(PORTD, PD0);
		_delay_ms(1000);
		*/
		
		//tank_turn_left(200,200);
	}
	
}

void spi_init()
{
	clearbit(DDRD, PIND3);		// Avbrott från kommunikationsenheten är input
	clearbit(DDRD, PIND2);		// Samma för sensorenheten
	setbit(PORTD, PORTD3);		// Slå på internt pull up-motstånd (1 är neutralt läge, 0 är avbrottsförfrågan!)
	setbit(PORTD, PORTD2);		// Samma för sensor
	
	setbit(DDRB, PINB3);	// Slave Select  (SS) för kommunikationsenheten är output!
	setbit(DDRB, PINB2);	// Schutzstaffel (SS) för sensor, också output
	setbit(DDRB, PINB5);	//MOSI output!
	clearbit(DDRB, PINB6);	//MISO input!
	setbit(DDRB, PINB7);	//SCK output!
	
	setbit(PORTB, PORTB3);	// Välj ej komm!  
	setbit(PORTB, PORTB2);	// Välj ej sensor!
	
	clearbit(DDRB, PINB4);	//SS master input och hög
	setbit(PORTB, PINB4);
	
	test = SPSR;
	
	//setbit(PORTB, PORTB3);	// 1 är neutral för komm.
	//setbit(PORTB, PORTB2);	// Samma för sensor
	
	//Sätt SPCR-registret, inställningar om master/slave, spi enable, data order, klockdelning
	SPCR = 0;
	//setbit(SPCR, SPIE);
	setbit(SPCR, SPE);
	setbit(SPCR, MSTR);
	setbit(SPCR, SPR0);
}

void spi_get_data_from_comm()
{
	clearbit(PORTB, PORTB3);	//Väljer komm
	SPDR = 0xFF;				//Lägger in junk i SPDR, startar överföringen
	while(!(SPSR & (1 << SPIF)));
	setbit(PORTB, PORTB3);		//Sätter komm till sleepmode
	spi_data_from_comm = SPDR;
}

void spi_send_byte(uint8_t byte)
{
	clearbit(PORTB, PORTB3); //Välj Komm-enheten måste ändras till allmän slav!
	SPDR = byte;
	
	//SPDR = 0xaa;
	/* Wait for transmission complete */
	while(!(SPSR & (1 << SPIF)));
	setbit(PORTB, PORTB3); //Sätt slave till sleepmode
	//test = SPDR;
	
	//PORTD = test;
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

uint8_t break_prot = 0;
uint8_t drive_prot = 'd';
uint8_t back_prot = 'b';
uint8_t stop_prot = 's';
uint8_t tank_turn_left_prot = 'l';
uint8_t tank_turn_right_prot = 'r';
uint8_t drive_turn_prot = 0b00110100;


ISR(INT1_vect)
{
	spi_get_data_from_comm();	//Sparar undan data från comm
	decode_comm(spi_data_from_comm); 
}

void decode_comm(uint8_t byte)
{
	if (byte == break_prot)
	{
		// Någon som vet vilken "Avbryt"-funktion som avses i designspecen!?!?!?
	}
	else if (byte == drive_prot)
	{
		drive_forwards(120); //Random värde!!!!
	}
	else if (byte == back_prot)
	{
		drive_backwards(120); //Random värde!!!!
	}
	else if (byte == stop_prot)
	{
		stop_motors();
	}
	else if (byte == tank_turn_left_prot)
	{
		tank_turn_left(120, 120); //Random värde!!!!
	}
	else if (byte == tank_turn_right_prot)
	{
		tank_turn_right(120); //Random värde!!!!
	}
	else if (byte == drive_turn_prot)
	{
		spi_get_data_from_comm();
		turn_left(spi_data_from_comm);
		spi_get_data_from_comm();
		turn_right(spi_data_from_comm);
		
	}
	
}
