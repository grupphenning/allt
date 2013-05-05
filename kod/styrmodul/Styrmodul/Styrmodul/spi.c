/*
 * spi.c
 *
 * Created: 5/5/2013 12:18:27 PM
 *  Author: davek282
 */ 
#include "spi.h"
#include <avr/interrupt.h>
#include "bitmacros.h"
// Buffrar som sparar data vi får från de andra SPI-enheterna

static volatile uint8_t spi_data_from_comm[256];
static volatile uint8_t spi_comm_read;
static volatile uint8_t spi_comm_write;

static volatile uint8_t spi_data_from_sensor[256];
static volatile uint8_t spi_sensor_read;
static volatile uint8_t spi_sensor_write;

// Buffrar som sparar den data vi ska skriva till de andra SPI-enheterna

static volatile uint8_t spi_to_comm[256];
static volatile uint8_t spi_to_comm_read;
static volatile uint8_t spi_to_comm_write;

static volatile uint8_t spi_to_sensor[256];
static volatile uint8_t spi_to_sensor_read;
static volatile uint8_t spi_to_sensor_write;

static volatile uint8_t sensor_interrupt, comm_interrupt;

enum { SPI_NONE, SPI_FROM_COMM, SPI_FROM_SENSOR, SPI_TO_COMM, SPI_TO_SENSOR };
static volatile uint8_t transmit_source = SPI_NONE;

// Slut på SPI-buffrar


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
	
	
	//Sätt SPCR-registret, inställningar om master/slave, spi enable, data order, klockdelning
	SPCR = 0;
	setbit(SPCR, SPIE);
	setbit(SPCR, SPE);
	setbit(SPCR, MSTR);
	//setbit(SPCR, SPR0);
	setbit(SPCR, SPI2X);

	//aktivera interrupt på INT0 och INT1
	setbit(EIMSK, INT0);	// Akvitera avbrottsförfrågan från sensorenheten
	setbit(EIMSK, INT1);	// Akvitera avbrottsförfrågan från kommunikationsenheten

	//aktivera interrupt-request på "any change"
	setbit(EICRA, ISC00);
	setbit(EICRA, ISC10);
}


ISR(INT1_vect) { comm_interrupt = 1; }
ISR(INT0_vect) { sensor_interrupt = 1; }

ISR(SPI_STC_vect)
{
	if(transmit_source == SPI_TO_COMM) {
		setbit(PORTB, PORTB3);
	}
	else if(transmit_source == SPI_TO_SENSOR) {
		setbit(PORTB, PORTB2);
	}		
	else if(transmit_source == SPI_FROM_COMM) {
		spi_data_from_comm[spi_comm_write++] = SPDR;
		setbit(PORTB, PORTB3);
	}
	else if(transmit_source == SPI_FROM_SENSOR) {
		spi_data_from_sensor[spi_sensor_write++] = SPDR;
		setbit(PORTB, PORTB2);
	}
	transmit_source = SPI_NONE;
}

void send_byte_to_sensor(uint8_t byte) { spi_to_sensor[spi_to_sensor_write++] = byte; }
void send_byte_to_comm(uint8_t byte) { spi_to_comm[spi_to_comm_write++] = byte; }

// Skickar en sträng till fjärrenheten (via komm) som dumpar den på skärmen
void debug(char *str)
{
	send_byte_to_comm('d');
	while(*str) send_byte_to_comm(*str++);
	send_byte_to_comm('\n');
	send_byte_to_comm(0);
}


void do_spi(uint8_t *has_comm_data_out, uint8_t *has_sensor_data_out, uint8_t *comm_data_out, uint8_t *sensor_data_out)
{
	if(transmit_source == SPI_NONE) {
		if(sensor_interrupt) {
			sensor_interrupt = 0;
			transmit_source = SPI_FROM_SENSOR;
			clearbit(PORTB, PORTB2);
			SPDR = 0;
		}
		else if(comm_interrupt) {
			comm_interrupt = 0;
			transmit_source = SPI_FROM_COMM;
			clearbit(PORTB, PORTB3);
			SPDR = 0;
		}
		else if(spi_to_sensor_read != spi_to_sensor_write) {
			transmit_source = SPI_TO_SENSOR;
			clearbit(PORTB, PORTB2);
			SPDR = spi_to_sensor[spi_to_sensor_read++];
		}
		else if(spi_to_comm_read != spi_to_comm_write) {
			transmit_source = SPI_TO_COMM;
			clearbit(PORTB, PORTB3);
			SPDR = spi_to_comm[spi_to_comm_read++];
		}
	}
	
	if(spi_comm_write != spi_comm_read)
	{
		*has_comm_data_out = 1;
		*comm_data_out = spi_data_from_comm[spi_comm_read++];
	}
	else *has_comm_data_out = 0;
			
	if(spi_sensor_write != spi_sensor_read)
	{
		*has_sensor_data_out = 1;
		*sensor_data_out = spi_data_from_sensor[spi_sensor_read++];
	}
	else *has_sensor_data_out = 0;
}	