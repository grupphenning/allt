#ifndef DISPLAY_H
#define DISPLAY_H
#define F_CPU 1000000UL // 1 MHz Internal oscillator FIXME!!! Kolla detta!
/*
 * L�gniv�bibliotek f�r displayen
 *
 * De globala funktionerna kan anv�ndas var som helst, (n�stan) n�r som helst.
 * Det enda de g�r �r uppdaterar en lokal buffer. F�r att verkligen uppdatera
 * displayen p� roboten m�ste update() kallas.
 */

#define LCD_RS		PINC7
#define LCD_RW		PIND7
#define LCD_ENABLE 	PINC6
#define LCD_DATA 	PORTA
#define LCD_DATA_DIR	DDRA
#define LCD_CONTROL	PORTC
#define LCD_CONTROL_DIR	DDRC

/************************************************************************/
/* Funktioner som anv�nds globalt                                       */
/************************************************************************/

// Initierar displayen
void init_display(void);
// Skicka kommando till displayen
void send_command(char command);
// Skicka ett tecken till display-bufferten
void send_character(char character);
// Skicka en str�ng av tecken till display-bufferten
void send_string(char *string);
// Flytta aktuell position i display-bufferten
void gotoyx(unsigned y, unsigned x);
// Rensa display-bufferten
void clear_screen();
// Skicka bufferten till displayen
void update();
// Registrera ett anv�ndarkonfigurerat tecken
void register_character(char* font, unsigned position);

/************************************************************************/
/* Funktioner som endast anv�nds internt i display.c                    */
/************************************************************************/
// Kolla om displayen �r upptagen
void check_busy(void);
// Toggla in data fr�n bussen till displayen
void finish_stuff(void);
// Initiera svenska tecken
void init_swedish(void);

#endif // DISPLAY_H
