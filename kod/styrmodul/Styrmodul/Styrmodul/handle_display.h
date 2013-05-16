#ifndef HANDLE_DISPLAY_H_
#define HANDLE_DISPLAY_H_
/*
 * handle_display.h
 *
 * Created: 5/5/2013 1:27:14 PM
 *  Author: davek282
 *
 * Hanterar den formatterade display-strängen
 *
 * En formatterad sträng (likt sprintf, men inte riktigt) används för att skriva ut data på Hennings display.
 * De enda platshållare som är implementerade är %d, %x och %X, vilka motsvarar (precis som för sprintf) decimal
 * form, hexadecimal form (med gemener), respektive hexadecimal form (med VERSALER). Argumentet som skrivs ut
 * uttrycks i binärform (alltså _inte_ ASCII-form) och anger den sensor som ska skrivas ut.
 *
 * Ett exempel:
 * "Value: %d\004 from sensor 4"
 * eller
 * "Val: 0x%X\004"
 *
 * På displayen visas då:
 * "Value: 157 from sensor 4"
 * eller
 * "Val: 0xEF"
 *
 * Decimaluttryck upptar tre till fem positioner (beroende på huruvida sensordatat är en eller två byte långa),
 * hexadecimala tar två alltid två positioner.
 */ 


#define BUFFER_SIZE 100 // Storlek på bufferten
#define MAX_SENSORS 7 // Antalet sensorer vars data kan skrivas ut


#include <stdint.h>

/*
 * Initierar den första formatterade display-strängen
 */
void init_default_printf_string();

/*
 * Uppdaterar den formatterade display-strängen
 */
void update_display_string();

/*
 * Strängen själv
 */
extern uint8_t display_printf_string[];


#endif /* HANDLE_DISPLAY_H_ */