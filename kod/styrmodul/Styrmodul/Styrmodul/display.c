#include <avr/io.h>
#include <util/delay.h>
#include "display.h"

void send_character_real(unsigned char character);
unsigned char position = 0;
char framebuffer[32];

void init_display(void)
{
	LCD_CONTROL_DIR |= 1 << LCD_ENABLE | 1 << LCD_RW | 1 << LCD_RS;
	_delay_ms(200);		// Wait for LCD to turn on

	/* Command is as:
	   001DNF--
	   DL = Data Length (1 = 8 bits, 0 = 4 bits)
	   N  = Number of display line
	   F  = Font
	*/
	send_command(0b00111000);
	_delay_ms(50);			//FIXME!!! This should be 2 ms, shouldn't it?

	/* Command is as:
	   00001DCB
	   D = enable display
	   C = enable cursor
	   B = blink cursor
	*/
	send_command(0b00001101);
	_delay_ms(50);			// FIXME!!! This too should be 2 ms!

	send_command(0x01);	// Clear display screen
	_delay_ms(2);

	LCD_DATA_DIR = 0xFF;		// set LCD data direction to output
	_delay_ms(2);

	init_swedish();
}


void init_swedish()
{
//	char font1[] = {0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x0E, 0x00};
	char font1[] = {0x04, 0x00, 0x0E, 0x01, 0x0F, 0x11, 0x0F, 0x00}; // å
	char font2[] = {0x04, 0x00, 0x0E, 0x11, 0x1F, 0x11, 0x11, 0x00}; // Å
	char font3[] = {0x0A, 0x00, 0x0E, 0x11, 0x1F, 0x11, 0x11, 0x00}; // Ä
	char font4[] = {0x0A, 0x0E, 0x11, 0x11, 0x11, 0x11, 0x0E, 0x00}; // Ö
	register_character(font1, 0x01); // å
	register_character(font2, 0x02); // Å
	register_character(font3, 0x03); // Ä
	register_character(font4, 0x04); // Ö
}


void register_character(char* font, unsigned pos)
{
	if(pos >= 0x10)	// There are only 16 custom characters
		return;

	_delay_ms(20);	// FIXME!!! Why?!? Makes no sense!
	send_command(0x40 | pos * 0x08);
	_delay_ms(2);

	int i;
	for(i = 0; i <= 0x08; i++)
	{
		send_character_real(*font++);
		_delay_ms(2);
	}
}

void gotoyx(unsigned y, unsigned x)
{
	if(y > 1 || x > 16)
		return;
	if(y == 0)
		position = x;
	else // y == 1
		position = 0x10 + x;
	// Hardware way to do it
//	send_command(0x80 | pos);
//	_delay_ms(50); // FIXME Too long, but how long is long enough?
}


void clear_screen()
{
	gotoyx(0, 0);
	int i;
	for(i = 0; i < 32; i++)
		send_character(' ');
}

void check_busy()
{
	LCD_DATA_DIR = 0x00;	// Input
	LCD_CONTROL &= ~(1 << LCD_RS);
	LCD_CONTROL |= 1 << LCD_RW;
	while(LCD_DATA >= 0x80)		
	{
		finish_stuff();
	}
	LCD_DATA_DIR = 0xFF;	// Output

}

void finish_stuff()
{
	LCD_CONTROL |= 1 << LCD_ENABLE;
	_delay_us(2);		// Is this safe?
	/*
	asm volatile("nop");
	asm volatile("nop");
	*/
	LCD_CONTROL &= ~(1 << LCD_ENABLE);
}


void send_string(char *string)
{
	while(*string != 0)
		send_character(*string++);
}

void send_command(char command)
{
//	_delay_ms(20);		// This really shouldn't be necessecary...
	check_busy();
	LCD_DATA = command;
	LCD_CONTROL &= ~((1 << LCD_RW) | (1 << LCD_RS)); // RW = 0 (= read mode), RS = 0 (= command mode)
	finish_stuff();
	LCD_DATA = 0x00;
}


void send_character(char character)
{
	framebuffer[position++] = character;

	if(position >= 32)
		position = 0;
}

void backspace()
{
	if(position == 0)
		position = 31;
	else
		position--;
	send_character(' ');

	update();

	if(position == 0)
		position = 31;
	else
		position--;
}

void newline()
{
	if(position > 15) // Second line
		position = 0;
	else // First line
		position = 16;
}

void update()
{
	send_command(0x02);	// Cursor home
	_delay_ms(20);		// FIXME!!! Seriously... what the hell? Why?!

	int length = 32;	// The total length of the LCD
	unsigned char *string = framebuffer;
	while(length > 0)
	{
		send_character_real(*string++);
		_delay_ms(2);
		if(--length == 16)	// End of first line of LCD
		{
			send_command(0x80 | 0x40);	// Next line
			_delay_ms(2);
		}
	}
}

void send_character_real(unsigned char character)
{
	check_busy();
	LCD_DATA = character;
	LCD_CONTROL &= ~(1 << LCD_RW);	// RW = 0 (= read mode)
	LCD_CONTROL |= 1 << LCD_RS; 	// RS = 1 (= character mode)
	finish_stuff();
	LCD_DATA = 0x00;
}

