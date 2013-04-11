/*
 * Styrmodul.c
 *
 * Created: 4/2/2013 2:20:45 PM
 *  Author: klawi021 ET AL!
 */ 
#define F_CPU 8000000UL

#include <avr/io.h>
#include "bitmacros.h"
#include <avr/delay.h>
#include "display.h"
#include <avr/interrupt.h>
#include "komm_styr_protokoll.h"
#include "sensor_styr_protokoll.h"


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
uint8_t spi_data_from_sensor;
//uint8_t amount = 255;
#define SPEED 255

int main(void)
{
	//aktivera global interrupts
	sei();
	//OSCCAL = 0x70;
	init_display();
	clear_screen();
	update();
	send_string("Data: ");
	update();
	spi_init();
	
	//aktivera interrupt på INT0 och INT1
	//setbit(EIMSK, INT0);
	setbit(EIMSK, INT1);

	//aktivera interrupt-request på "any change"
	//setbit(EICRA, ISC00);
	setbit(EICRA, ISC10);
	
	//_delay_ms(1000);
	
	///////////////////////////////////spi_send_byte(0xAA);
	
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
	////////////////////////////////////////////TIMSK1 = (1 << OCIE1A);  // Enable Interrupt TimerCounter1 Compare Match A (TIMER1_COMPA_vect)
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
	setbit(TCCR2B, CS20);
	//TCCR2B = (1 << CS20);
	////////////////////////////////////////////TIMSK2 = (1 << OCIE2A);
	//fullt ös på OCR=0xff, inget på 0x00
	
	
	//tank_turn_left(180);
	//init_spi();
	uint8_t ch = 'a';
	
	while(1)
	{
		//_delay_ms(100);
// 		send_character(ch++);	//Ä
// 		update();
		
		//spi_send_byte(0xD2);
		
		/*
		claw_out();
		_delay_ms(1000);
		claw_in();
		_delay_ms(1000);
		*/
		
		//tank_turn_left(200);
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
	
	//Sätt SPCR-registret, inställningar om master/slave, spi enable, data order, klockdelning
	SPCR = 0;
	//setbit(SPCR, SPIE);
	setbit(SPCR, SPE);,
	setbit(SPCR, MSTR);
	setbit(SPCR, SPR0);
}

void spi_get_data_from_comm(uint8_t message_byte)
{
	clearbit(PORTB, PORTB3);	//Väljer komm
	SPDR = message_byte;		//Lägger in meddelande i SPDR, startar överföringen
	while(!(SPSR & (1 << SPIF)));
	setbit(PORTB, PORTB3);		//Sätter komm till sleepmode
	spi_data_from_comm = SPDR;
}

/*void spi_get_data_from_sensor()
{
	clearbit(PORTB, PORTB2);	//Väljer sensor
	//SPDR = message_byte;		//Lägger in meddelande i SPDR, startar överföringen
	while(!(SPSR & (1 << SPIF)));
	setbit(PORTB, PORTB2);		//Sätter sensor till sleepmode
	spi_data_from_sensor = SPDR;
}*/

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
void tank_turn_left(uint8_t amount)
{
	LEFT_AMOUNT = amount;
	RIGHT_AMOUNT = amount;
	
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
	CLAW_AMOUNT = 78*4;
}

void claw_in()
{
	CLAW_AMOUNT = 15*4;
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

/*
..................................................|         /  _________________     __              __    __              __.........................
..................................................|        /  |  ______________  |  |  \            /  |  |  \            /  |........................
..................................................|       /   | |              | |  |   \          /   |  |   \          /   |........................
..................................................|      /    | |              | |  |    \        /    |  |    \        /    |.......................
..................................................|     /     | |              | |  |     \      /     |  |     \      /     |........................
..................................................|    /      | |   O     =    | |  |      \    /      |  |      \    /      |.......................
..................................................|   /       | |              | |  |       \  /       |  |       \  /       |.......................
..................................................|  /        | |     /        | |  |        \/        |  |        \/        |........................ 
..................................................|  \        | |    /_        | |  |                  |  |                  |........................
..................................................|   \       | |              | |  |                  |  |                  |........................
..................................................|    \      | |              | |  |                  |  |                  |........................
..................................................|     \     | |              | |  |                  |  |                  |........................
..................................................|      \    | |   \_____/    | |  |                  |  |                  |........................
..................................................|       \   | |              | |  |                  |  |                  |........................
..................................................|        \  | |______________| |  |                  |  |                  |........................
..................................................|         \ |__________________|  |                  |  |                  |........................
*/

/*//-----------------AVBRYT-------------------

//-----------------STYRKOMMANDON------------
uint8_t break_prot = 0b00000000;
uint8_t control_command_prot = 0b00100000;
uint8_t drive_prot = 0b00100000;
uint8_t back_prot = 0b00100100;
uint8_t stop_prot = 0b00101000;
uint8_t tank_turn_left_prot = 0b00101100;
uint8_t tank_turn_right_prot = 0b00110000;
uint8_t drive_turn_prot = 0b00110100;

uint8_t drive_turn_left_request = 0b00111000;
uint8_t drive_turn_right_request = 0b00111100;
//-----------KALIBRERING AV SENSORER---------

//-------------GRIPKLOKOMMANDON--------------
uint8_t claw_in_prot = 0b01100000;
uint8_t claw_out_prot = 0b01100100;
//----------SÄTT PD-KONSTANTER---------------
*/

ISR(INT1_vect)
{
//	send_string("A ");
	//update();
	spi_get_data_from_comm(0xFF);	//Sparar undan data från comm
	decode_comm(spi_data_from_comm); 
//	send_string("C");
//	update();
	
}

/*ISR(INT0_vect)
{
	spi_get_data_from_sensor();
	char tmp[15];
	sprintf(tmp, "Sensor: %d", spi_data_from_sensor);
	send_string(tmp);
	update();
	decode_sensor(spi_data_from_sensor);
	
}*/

void decode_comm(uint8_t command)
{
	char tmp[10];
	sprintf(tmp, "decode: 0x%02X", command);
	send_string(tmp);
	update();
	uint8_t byte = command & 0b11100000;
	
	if (command == BREAK_PROT)
	{
		// Någon som vet vilken "Avbryt"-funktion som avses i designspecen!?!?!?
		// Kör iaf den avbrytfunktion som avses i designspecen!!!!!!
	send_string("break");
	update();
	}
	else if (byte == CONTROL_COMMAND_PROT)
	{
		if (command == DRIVE_PROT)
		{
			drive_forwards(SPEED);
			send_string("Fram");	// Lägger ut "Fram" på displayen.
			update();
		}
		else if (command == BACK_PROT)
		{
			drive_backwards(SPEED);
			send_string("Bak");
			update();
		}
		else if (command == STOP_PROT)
		{
			stop_motors();
			send_string("Stopp");
			update();
		}
		else if (command == TANK_TURN_LEFT_PROT)
		{
			tank_turn_left(SPEED);
			send_string("Rotera vänster");
			update();
		}
		else if (command == TANK_TURN_RIGHT_PROT)
		{
			tank_turn_right(SPEED);
			send_string("Rotera höger");
			update();
		}
		/*else if (command == drive_turn_prot)
		{
			spi_get_data_from_comm(drive_turn_left_request);
			spi_get_data_from_comm(drive_turn_left_request);
			turn_left(spi_data_from_comm);
			spi_get_data_from_comm(drive_turn_right_request);
			spi_get_data_from_comm(drive_turn_right_request);
			turn_right(spi_data_from_comm);
		}*/
	}	
	else if (command == CLAW_IN_PROT)
	{
		claw_in();
		send_string("Klo in");
		update();
	}
	else if (command == CLAW_OUT_PROT)
	{
		claw_out();
		send_string("Klo ut");
		update();
	}
	else 
	{
		send_string("B");	
		update();
	}
	
}

void decode_sensor(uint8_t command)
{
	uint8_t sensor_type = command & TYPE_OF_SENSOR;
	if( sensor_type == REFLEX )
	{
		if(command == CROSSING_RIGHT_PROT)	
		{
			tank_turn_right(SPEED);
		}
		else if (command == CROSSING_LEFT_PROT)
		{
			tank_turn_left(SPEED);	
		}
		else if (command == CROSSING_FORWARD_PROT)
		{
			drive_forwards(SPEED);
		}

	}	
	
}	




