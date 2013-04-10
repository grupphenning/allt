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

uint8_t read_adc();
uint8_t read_ir(uint8_t sensor_no);
uint8_t read_gyro();
void init_adc();
int main(void)
{
	DDRA = 0b10000000;
	DDRD = 0xFF;
	init_spi();
	init_adc();
	
    while(1)
    {
		/*PSEUDO
			send_to_styr(read_gyro());
			for(i=0 to 5)
				send_to_styr(read_ir(i));
			*/
			
			
    }
}


void init_spi()
{
	sei();
	setbit(SPCR, SPE);		//Enables spi
	clearbit(DDRB, PINB4);	// SS är input
	clearbit(DDRB, PINB5);	// MOSI är input
	setbit(DDRB, PINB6);	// MISO är output
	clearbit(DDRB, PINB7);	//CLK är input
	setbit(DDRA, PINA7);	// Avbrottsförfrågan är output
	setbit(PORTA, PINA7);	// 1 = normal, 0 = avbrottsförfrågan
}

void init_adc()
{
	setbit(ADCSRA,ADEN);
}


uint8_t read_gyro()
{
	ADMUX = 0b00000010;
	return read_adc();
}	
uint8_t read_ir(uint8_t sensor_no)
{
	ADMUX = 0x00;
	sensor_no = sensor_no << 4;
	PORTD = 0b01110000 & sensor_no; //Tell mux where to read from
	return read_adc();
}

uint8_t read_adc()
{
	setbit(ADCSRA,ADSC); //start_reading
	while(ADSC); //Vänta tills färdigläst, fixa avbrott senare !
	return ADCL;
}

	
	

