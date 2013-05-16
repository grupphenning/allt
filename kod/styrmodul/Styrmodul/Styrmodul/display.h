#ifndef DISPLAY_H
#define DISPLAY_H
#define F_CPU 1000000UL // 1 MHz Internal oscillator FIXME!!! Kolla detta!
/*
 * Lågnivåbibliotek för displayen
 *
 * De globala funktionerna kan användas var som helst, (nästan) när som helst.
 * Det enda de gör är uppdaterar en lokal buffer. För att verkligen uppdatera
 * displayen på roboten måste update() kallas.
 */

#define LCD_RS		PINC7
#define LCD_RW		PIND7
#define LCD_ENABLE 	PINC6
#define LCD_DATA 	PORTA
#define LCD_DATA_DIR	DDRA
#define LCD_CONTROL	PORTC
#define LCD_CONTROL_DIR	DDRC

/************************************************************************/
/* Funktioner som används globalt                                       */
/************************************************************************/

// Initierar displayen
void init_display(void);
// Skicka kommando till displayen
void send_command(char command);
// Skicka ett tecken till display-bufferten
void send_character(char character);
// Skicka en sträng av tecken till display-bufferten
void send_string(char *string);
// Flytta aktuell position i display-bufferten
void gotoyx(unsigned y, unsigned x);
// Rensa display-bufferten
void clear_screen();
// Skicka bufferten till displayen
void update();
// Registrera ett användarkonfigurerat tecken
void register_character(char* font, unsigned position);

/************************************************************************/
/* Funktioner som endast används internt i display.c                    */
/************************************************************************/
// Kolla om displayen är upptagen
void check_busy(void);
// Toggla in data från bussen till displayen
void finish_stuff(void);
// Initiera svenska tecken
void init_swedish(void);

#endif // DISPLAY_H
