/*
 * sensormodul.c
 *
 * Created: 2013-04-09 11:03:56
 *  Author: eriha181
 */
//definiera klockhastighet
#define F_CPU 8000000UL

#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>
#include "bitmacros.h"

#define GYRO PINA2
#define INTERUPT_REQUEST PINA7
#define TAPE_SENSOR PINA1
#define IR_SENSOR PINA0

void read_adc();
void read_ir(uint8_t sensor_no);
void read_gyro();
void init_adc();

uint8_t spi_data_from_master;
uint8_t spi_data_to_master;
uint8_t test_data;
uint8_t	adc_interrupt = 0;

int main(void)
{
	setbit(DDRA, PINA7); //Sätter avbrottpinne mot styr som output
	clearbit(DDRA,PINA0);
	DDRD = 0xFF;
	//DDRB = 0xFF; //OBS!! TEST TAS BORT SÅ FORT BUSS SKALL UPP
	init_spi();
	init_adc();
	
    while(1)
    {
		
		read_ir(1);
		_delay_ms(100);
		if(adc_interrupt)
		{
			
			send_to_master(test_data);
			adc_interrupt = 0;
		}
// 		make_crossing_decision ('l', 'k');
// 		
// 		_delay_ms(5000);
// 		make_crossing_decision( 'k', 'l');
// 		_delay_ms(5000);
// 		//_delay_ms(1);
    }
}


void init_spi()
{
	setbit(SPCR, SPE);		//Enables spi
	clearbit(DDRB, PINB4);	// SS är input
	clearbit(DDRB, PINB5);	// MOSI är input
	setbit(DDRB, PINB6);	// MISO är output
	clearbit(DDRB, PINB7);	//CLK är input
	setbit(DDRA, PINA7);	// Avbrottsförfrågan är output
	setbit(PORTA, PINA7);	// 1 = normal, 0 = avbrottsförfrågan
	sei();
}

void init_adc()
{
	//reference voltage
	setbit(ADCSRA,REFS0);
	
	//ADC enable
	setbit(ADCSRA,ADEN);
	
	//Left adjust measurment
	setbit(ADMUX,ADLAR);
	
	//Set ADC in free running- mode
	//setbit(ADCSRA,ADATE);
	
	//Enable interupt
	setbit(ADCSRA,ADIE);
	
	//SKalning av klockfrekvens
	setbit(ADCSRA,ADPS2);
	setbit(ADCSRA,ADPS1);
	clearbit(ADCSRA,ADPS0);
	
	
}


void read_gyro()
{
	clearbit(ADMUX,MUX0);
	setbit(ADMUX,MUX1);
	clearbit(ADMUX,MUX2);
	read_adc();
}	
void read_ir(uint8_t sensor_no)
{
	clearbit(ADMUX,MUX0);
	clearbit(ADMUX,MUX1);
	clearbit(ADMUX,MUX2);
	sensor_no = sensor_no << 4;
	PORTD = 0x70 & sensor_no; //Tell mux where to read from
	read_adc();
}

void read_tape(uint8_t sensor_no)
{
	setbit(ADMUX,MUX0);
	clearbit(ADMUX,MUX1);
	clearbit(ADMUX,MUX2);
}

void read_adc()
{
	setbit(ADCSRA,ADSC); //start_reading
}


ISR(ADC_vect)
{
	test_data = ADCH;
	adc_interrupt = 1;
}



void SPI_read_byte()
{
	spi_data_from_master = SPDR;
}

ISR(SPI_STC_vect)
{
	SPI_read_byte();
}

void create_master_interrupt()
{
	PORTA ^= (1 << PORTA7);
}

/*
-----------------Reflex------------------------
uint8_t crossing_right_prot = 0b01000000;
uint8_t crossing_left_prot = 0b01001000;
uint8_t crossing_forward_prot = 0b01010000;
uint8_t goal_prot = 0b01100000;
------------------IR---------------------------
------------------Gyro------------------------
*/

void send_crossing_decision(uint8_t ch)
{
	uint8_t commando;
	
	// Konverterar från sensordata till meddelande som mottas av styrenheten!!!
	switch(ch) {
		case 'h': commando = 0b01000000; break; // Sväng höger i korsning
		case 'l': commando = 0b01001000; break; // Sväng vänster i korsning
		case 'f': commando = 0b01010000; break; // Rakt fram i korsning
		case 'g': commando = 0b01100000; break; // Målet

		default: commando = 0xff; break;
	}
	if(commando != 0xff)
	send_to_master(commando);
}

void send_to_master(uint8_t byte)
{
	SPDR = byte;
	create_master_interrupt();
}

void make_crossing_decision(uint8_t tape_one, uint8_t tape_two)
{
	if (tape_one == 'l' && tape_two == 'k')	 // Om höger (första tejpen lång, andra kort) i korsning!
	{
		send_crossing_decision('h');
	}
	else if (tape_one == 'k' && tape_two == 'l')	 // Om vänster (första tejpen kort, andra lång) i korsning!
	{
		send_crossing_decision('l');
	}
	else if (tape_one == 'k' && tape_two == 'k')	 // Om framåt (första tejpen kort, andra kort) i korsning!
	{
		send_crossing_decision('f');
	}
	else if (tape_one == 'k' && tape_two == 's')	 // Om mål (första tejpen kort, andra "smal" (???))
	{
		send_crossing_decision('g');
	}
}
	
	
	

