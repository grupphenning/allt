/*
 * Kommunikationsmodul.c
 *
 * Created: 4/6/2013 2:39:23 PM
 *  Author: rasme879
 */
//Måste definieras först!
//#define F_CPU 8000000UL

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

uint8_t spi_data_from_master;
uint8_t spi_data_to_master;

void SPI_read_byte()
{
	//PORTA = SPDR;	// Bra för felsökning!
	spi_data_from_master = SPDR;
}


void SPI_write_byte(uint8_t byte)
{
	SPDR = byte;
}


//unsigned char USART_Recieve(void);
unsigned char USART_Receive(void);
void USART_Init(unsigned int baud);
void USART_Transmit(unsigned char data);

uint8_t out;


int main(void)
{
	out = 1;
	//PORTA = 0;
	setbit(DDRB, PORTB1);
	setbit(DDRA, PORTA7);
	setbit(DDRA, PORTA6);
	sei(); //Enable global interrupts

	init_spi();
	SPDR = 0x32;
	

	/*DDRA = 0xff;
	DDRB = 0xff;
	DDRC = 0xff;
	DDRD = 0xff;
	PORTA = 0xff;
	PORTB = 0xff;*/
	//PORTC = 0xff;
	//PORTD = 0xff;
	//	USART_Init(9600);

	// kom (ff) cts=(is) clear to send (?), rts=request to send
	// d0 cts <- (rts)
	// d1 tx  -> (rx)
	// d2 rx  <- (tx)
	// d3 rts -> (cts)

	// 1 byte ska ta runt 80 us
	// Ok nu kör vi 2400 baud i stället
	//UCSRB = (1<<RXEN)|(1<<TXEN);

	//ggr 20 us det tar att överföra 'a' då UBRR=3:
	//c0:4.3 b0:4.3 a0:4.3 90:5 80:5 70:5.4 60:5.7 50:6 40:6.4 30:6.7 20:7
	OSCCAL = 0x70;
	unsigned ubrr = F_CPU / (16 * 9600) - 1;
	//UBRRH = (unsigned char)(ubrr>>8);
	//UBRRL = (unsigned char)ubrr;
	UBRRH = 0x00;
	UBRRL = 0x02;
	UCSRC = (1<<URSEL)|(1<<USBS)|(3<<UCSZ0);
	UCSRB = (1<<TXEN)|(1<<RXEN);

	uint8_t data_from_bt, data_from_bt_old;
	//unsigned data_from_bt, data_from_bt_old;
	
    while(1)
    {
		////setbit(PORTA, PORTA6);
		data_from_bt = USART_Receive();
		USART_Transmit(data_from_bt);
// 		if ( data_from_bt != data_from_bt_old)
// 		{
			data_from_bt_old = data_from_bt;
			decode_remote(data_from_bt);
		//}
		////clearbit(PORTA, PORTA6);
		//send_to_master(b);
		//clearbit(PORTB, PB0);
		//PORTA = 2;
		//_delay_ms(100);
		//setbit(SPCR, SPE);		//Enables spi
		//PORTA = SPI_SlaveReceive();
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

void serial_send_byte(uint8_t val)
{
	while((UCSRA &(1<<UDRE)) == 0);	// Vänta på att föregående värde redan skickats
	UDR = val;
}

void USART_Init(unsigned int baud)
{
	unsigned ubrr;
	ubrr = F_CPU / (16 * baud) - 1;
	
	//Set baud rate
	UBRRH = (unsigned char)(ubrr>>8);
	UBRRL = (unsigned char)ubrr;
	
	//Enable receiver and transmitter
	UCSRB = (1<<RXEN)|(1<<TXEN);
	
	//Set frame format: 8data, 2stop bit
	UCSRC = (1<<URSEL)|(1<<USBS)|(3<<UCSZ0);
	
	// Set RTS output and 1
	setbit(DDRC, PINC0);
	setbit(PORTC, PINC0);
	
	// CTS input
	clearbit(DDRC, PINC1);
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

unsigned char USART_Receive(void)
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

ISR(SPI_STC_vect)
{
	SPI_read_byte(); //Sparar ner SPDR till PORTA och spi_data_from_master
	//decode_spi_from_master();
	//SPI_write_byte(spi_data_to_master);  //Ska ta något argument!
}	

void create_master_interrupt()
{
	//PORTA &= ~(1 << PINA7);
	PORTA ^= (1 << PORTA7);
	/*
	if(out)
	{
		setbit(PORTA, PORTA7);
		out = 0;
	}
	else
	{
		clearbit(PORTA, PORTA7);
		out = 1;
	}
	*/
}

// ------------------------------------------------------------------------
// --------------------------------PROTOKOLL-------------------------------
// ------------------------------------------------------------------------

/*
uint8_t break_prot = 0b00000000;
uint8_t drive_prot = 0b00100000;
uint8_t back_prot = 0b00100100;
uint8_t stop_prot = 0b00101000;
uint8_t tank_turn_left_prot = 0b00101100;
uint8_t tank_turn_right_prot = 0b00110000;
*/
uint8_t drive_turn_prot = 0b00110100;
uint8_t drive_turn_left_request = 0b00111000;
uint8_t drive_turn_right_request = 0b00111100;  // Ska sättas till ngt fint!!!!
uint8_t drive_turn_left_value;
uint8_t drive_turn_right_value;

void decode_remote(uint8_t ch)
{
	uint8_t commando;
	
	// Konverterar från Blåtand till styr-komm-protokollet!
	switch(ch) {
		case 'l': commando = 0b00101100; break;
		case 'd': commando = 0b00100000; break;
		case 'r': commando = 0b00110000; break;
		case 's': commando = 0b00101000; break;
		case 'b': commando = 0b00100100; break;
		//claw in
		case 'c': commando = 0b01100000; break;
		//claw out
		case 'o': commando = 0b01100100; break;
		default: commando = 0xff; break;
	}
	// Ej löst då vi skickar flera byte!
	/*if (ch == 'v') // fram vänster
	{
		commando = 0b00110100;
		// Sparar undan värdet för vänster resp. höger hjulpar!!!
		drive_turn_left_value = 0x00; // OBS!!!! Blajvärde!!!!
		drive_turn_right_value = 0xff; // OBS!!!! Blajvärde!!!!
	}
	if (ch == 'h') // fram  höger
	{
		commando = 0b00110100;
		// Sparar undan värdet för vänster resp. höger hjulpar!!!
		drive_turn_left_value = 0xff; // OBS!!!! Blajvärde!!!!
		drive_turn_right_value = 0x00; // OBS!!!! Blajvärde!!!!
	}*/
	
	//om det var något av l,d,r,s,b
	if(commando != 0xff)
	{
		send_to_master(commando);
	}		
	
	/*if(ch == 'v' || ch == 'h') {
		send_to_master(drive_turn_right_value);
		send_to_master(drive_turn_right_value);
	}*/
}

/*void decode_spi_from_master()
{
	if (spi_data_from_master = drive_turn_left_request)
	{
		SPDR = drive_turn_left_value;
		create_master_interrupt();
	}
	else if (spi_data_from_master = drive_turn_left_request)
	{
		SPDR = drive_turn_right_value;
		create_master_interrupt();
	}
	
}*/

void send_to_master(uint8_t byte)
{
	SPDR = byte;
	char temp[5];
	sprintf(temp,"0x%02X", byte);
	for (int i = 0; i<4; i++)
	{
		USART_Transmit(temp[i]);
	}
	create_master_interrupt();
}