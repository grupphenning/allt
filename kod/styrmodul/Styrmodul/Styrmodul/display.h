#ifndef DISPLAY_H
#define DISPLAY_H
#define F_CPU 1000000UL // 1 MHz Internal oscillator
/*
   Pin configuration:

   PINB0 	LED out
   PINB1	Button in
   PINB2	RS
   PINB3	R/W
   PINB4	Enable

   PINA0
   ...		Display data
   PINA7

*/

#define LCD_RS		PINC7
#define LCD_RW		PIND7 //finns ej
#define LCD_ENABLE 	PINC6
#define LCD_DATA 	PORTA
#define LCD_DATA_DIR	DDRA
#define LCD_CONTROL	PORTC
#define LCD_CONTROL_DIR	DDRC

/* For public use */
void init_display(void);
void send_command(char command);
void send_character(char character);
void send_string(char *string);
void gotoyx(unsigned y, unsigned x);
void clear_screen();
void update();

/* Internal */
void check_busy(void);
void finish_stuff(void);
void init_swedish(void);
void register_character(char* font, unsigned position);

#endif // DISPLAY_H
