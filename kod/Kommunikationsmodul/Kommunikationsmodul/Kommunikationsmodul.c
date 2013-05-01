﻿/*
 * Kommunikationsmodul.c
 *
 * Created: 4/6/2013 2:39:23 PM
 *  Author: rasme879
 */
//Måste definieras först!
#define F_CPU 7380000UL

#include <avr/io.h>
#include <avr/delay.h>
#include "bitmacros.h"
#include "Kommunikationsmodul.h"
#include "../../Styrmodul/Styrmodul/Styrmodul/komm_styr_protokoll.h"
#include <avr/interrupt.h>

#define BUF_SZ 256

volatile uint8_t data_from_styr;


uint8_t
	spiw_data[256], spiw_r, spiw_w,
	usartw_data[256], usartw_r, usartw_w, usartw_byte;

void send_spi(uint8_t data) { spiw_data[spiw_w++] = data; }
void send_usart(uint8_t data) { usartw_data[usartw_w++] = data; }

int main(void)
{
//	out = 1;

	init_spi();
	USART_init();
	
	sei(); //Enable global interrupts

	clearbit(PORTC, PINC0); // Ready to receive

    while(1)
    {
		static uint8_t spi_state = 0; // 0 väntar, 1 skriver
		static uint8_t usart_state = 0; // 0 väntar, 1 skriver
		uint8_t spir, has_spir = 0, usartr, has_usartr = 0;
		
		// Processa SPI
		if(spi_state == 0) {
			// Kolla om vi har fått nåt
			if(SPSR & (1 << SPIF)) {
				spir = SPDR;
				has_spir = 1;
			}
			// Finns det nåt att skicka?
			else if(spiw_r != spiw_w) {
				SPDR = spiw_data[spiw_r++];
				PORTA ^= (1 << PORTA7);
				spi_state = 1;
			}				
		}
		else if(spi_state == 1) {
			if(SPSR & (1 << SPIF)) spi_state = 0;
		}

		// Processa USART
		if(usart_state == 0) {
			// Firefly vill sända data
			if(UCSRA & (1 << RXC)) {
				usartr = UDR;
				has_usartr = 1;
			}
			// Vi vill sända data
			else if(usartw_r != usartw_w) {
				usart_state = 1;
				setbit(PORTC, PINC0);
			}				
		}
		else if(usart_state == 1) {
			if(UCSRA & (1 << UDRE)) {
				UDR = usartw_data[usartw_r++];
				usart_state = 0;
				clearbit(PORTC, PINC0);
			}				
		}
	
		// Nu finns data i usartr om has_usartr == 1
		// och i spir om has_spir == 1
		
		if(has_usartr) {
			send_usart(usartr);
			send_usart('\n');
/*			
if(usartr == '1')
{
send_usart_string("a0123456781234568123456778123215\n");
send_usart_string("b0123456781234568123456778123215\n");
send_usart_string("c0123456781234568123456778123215\n");
send_usart_string("d0123456781234568123456778123215\n");
send_usart_string("e0123456781234568123456778123215\n");
send_usart_string("f0123456781234568123456778123215\n");
}
*/
			decode_remote(usartr);
		}
		if(has_spir) {
		//	send_usart(spir);
		}
		
//		data = USART_Receive();
//		uint8_t data;
//		data = SPI_SlaveReceive();
//		decode_remote(data);
//		USART_Transmit(data);
		
//		char tmp[10];sprintf(tmp, "%02X", data_from_styr);
//		USART_Transmit(tmp[0]);
//		USART_Transmit(tmp[1]);
    }
}

void send_usart_string(uint8_t *str)
{
	while(*str)
		send_usart(*str++);
}


void init_spi()
{
//	setbit(DDRB, PORTB1);		// Va ä dä här?

	setbit(SPCR, SPE);		//Enables spi
	clearbit(DDRB, PINB4);	// SS är input
	clearbit(DDRB, PINB5);	// MOSI är input
	setbit(DDRB, PINB6);	// MISO är output
	clearbit(DDRB, PINB7);	//CLK är input
	setbit(DDRA, PINA7);	// Avbrottsförfrågan är output
	setbit(PORTA, PINA7);	// 1 = normal, 0 = avbrottsförfrågan
	SPDR = 0x32;
}

void serial_send_byte(uint8_t val)
{
	while((UCSRA &(1<<UDRE)) == 0);	// Vänta på att föregående värde redan skickats
	UDR = val;
}

void USART_Transmit(unsigned char data)
{
	// Wait until firefly is ready to receive
	while(PORTC & (1<<PINC1));
	
	//Put data into buffer, sends the data
	UDR = data;
	
	//Wait for empty transmit buffer
	while(!(UCSRA & (1<<UDRE)));
}

uint8_t USART_Receive(void)
{
	// Ready to receive
	clearbit(PORTC, PINC0);
	
	//Wait for data to be received
	while(!(UCSRA & (1<<RXC)));
	
	// Not ready to receive anymore
	setbit(PORTC, PINC0);

	//Get and return received data from buffer
	return UDR;
}

void USART_init()
{
	OSCCAL = 0x70;		// Sänk klockhastigheten (0x7f betyder standard, dvs 8MHz).
	// Funkar inte om man debuggar och därför går det inte att använda seriell då.
	
	UBRRH = 0x00;
	UBRRL = 0x02;
	UCSRC = (1<<URSEL)|(1<<USBS)|(3<<UCSZ0);
	UCSRB = (1<<TXEN)|(1<<RXEN);
}

ISR(SPI_STC_vect)
{
	data_from_styr = SPDR;
}	


void decode_remote(uint8_t ch)
{
	
	/* Hanterar kommando 'p', som tar tre argument: de tre PID-värden som används till regleringen */
	static uint8_t pid = 0;	// Flagga som anger att sist mottagna kommando från fjärrenheten var just pid!
	static uint8_t display = 0; // Indikerar att nästa byte är en byte som ska vidare till styr
	static uint8_t speed_all = 0;
	static uint8_t p, i, dh, dl;	// Argumenten till PID!

	/* Om PID är aktiverat (dvs. icke-noll), behandla den aktuella byten som ett argument till PID */
	if(pid) {
		if(pid == 4) p = ch;
		else if(pid == 3) i = ch;
		else if(pid == 2) dh = ch;
		else if(pid == 1) {
			dl = ch;
			send_spi(COMM_SET_PID);
			send_spi(p);
			send_spi(i);
			send_spi(dh);
			send_spi(dl);
			--pid;
			return;			// This ought to be needed, shound't it?
		}
		--pid;
	} else if(display)	/* Nästa byte är ett tecken som ska vidare till displayen på styrenheten */
	{
		display = 0;
		send_spi(ch);
		return;
	}	
	else if(speed_all) {
		send_spi(COMM_SET_SPEED);
		send_spi(ch);
		speed_all = 0;
		return;
	}
	else {
	
		// Konverterar från Blåtand till styr-komm-protokollet!
		switch(ch) {
			case 'l': send_spi(COMM_LEFT); break;					// vänster
			case 'd': send_spi(COMM_DRIVE); break;					// fram
			case 'r': send_spi(COMM_RIGHT); break;					// höger
			case 's': send_spi(COMM_STOP); break;								// stop
			case 'b': send_spi(COMM_BACK); break;					// bakåt
			case 'c': send_spi(COMM_CLAW_IN); break;					// claw in
			case 'o': send_spi(COMM_CLAW_OUT); break;				// claw out
			case 'v': send_spi(COMM_DRIVE_LEFT); break;				// kör framåt och vänster
			case 'h': send_spi(COMM_DRIVE_RIGHT); break;				// framåt och höger
			case 'q': send_spi(COMM_CLEAR_DISPLAY); break;						// Rensa displayen
			case 'z': send_spi(COMM_DISPLAY); display = 1; break;				// tecken till displayen
			case 'p': pid = 4; break;											// PID-konstanter
			case 'n': send_spi(COMM_ENABLE_PID); break;							// Slå på reglering
			case 'm': send_spi(COMM_DISABLE_PID); break;						// Slå av reglering
			case 't': send_spi(COMM_TOGGLE_SENSORS); break;						// Aktivera/deaktivera sensorer
			case 'w': send_spi(COMM_TURN_90_DEGREES_LEFT); break;				// Vrid 90 grader vänster
			case 'e': send_spi(COMM_TURN_90_DEGREES_RIGHT); break;				// Vrid 90 grader höger
			case '1': send_spi(COMM_CALIBRATE_SENSORS); break;					// Kalibrera sensorer
			case '#': speed_all = 1; break;
		}			
	}
}

void send_to_master(uint8_t byte)
{
	SPDR = byte;
	PORTA ^= (1 << PORTA7);
}
