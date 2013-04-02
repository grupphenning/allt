/*
 * styr.c
 *
 * Created: 4/2/2013 2:20:45 PM
 *  Author: klawi021
 */ 


#include <avr/io.h>

int main(void)
{
	/* Set */
	TCCR1A = (1 << COM1A1) | (1 << WGM11);
	TCCR1B = (1 << WGM11) | (1 << WGM13) | (1 << WGM12) | (1 << CS12) | (1 << CS10);
	TCCR2A = (1 << COM2A1) | (1 << COM2B1) | (1 << WGM21) | (1 << WGM20); 
	TCCR2B = (1 << CS20);
	
    while(1)
    {
        //TODO:: Please write your application code 
    }
}