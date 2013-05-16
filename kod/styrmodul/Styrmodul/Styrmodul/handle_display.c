/*
 * handle_display.c
 *
 * Created: 5/5/2013 1:26:17 PM
 *  Author: davek282
 */ 
void debug(char *str);

#include "handle_display.h"
#include <string.h>
#include "Styrmodul.h"
#include "sensor.h"
#include <avr/io.h>
#include "bitmacros.h"
uint8_t display_printf_string[100];

#define BUFFER_SIZE 100
#define MAX_SENSORS 7
void update_display_string()
{
	
// 	char tmp[100];
// 	clear_screen();
// 	sprintf(tmp, "L: %03d   R: %03d F: %03d ", sensor_buffer[IR_LEFT_FRONT], sensor_buffer[IR_RIGHT_FRONT], sensor_buffer[IR_FRONT]);
// 	send_string(tmp);
// 	update();
// 	return;
// 	
/*
	clear_screen();
	send_string(display_printf_string);
	update();
	return;
*/
    uint8_t *inp = display_printf_string;

    char tmpStr[BUFFER_SIZE];
    char *tmpp = tmpStr;
    while(inp < display_printf_string + BUFFER_SIZE)
    {
	    if(*inp != '%') // Normal-tecken, kopiera och gå vidare!
	    {
		    *tmpp = *inp;
		    if(*inp == '\0')
		    break;
		    inp++;
		    tmpp++;
		    continue;
	    } else
	    {
		    inp++;    // Bortom %-tecknet
		    uint8_t base;   // Nästa är d för decimal, x för hex
		    if(*inp == 'd')
				base = 10;
		    else if((*inp == 'x') || (*inp == 'X'))
				base = 16;
		    inp++;
		    uint8_t sensor = *inp; // Nästa är sensor-index
		    inp++;
		    if(sensor > MAX_SENSORS - 1)
				continue;
			if(sensor == 6)
			{
				sprintf(tmpp, "%6d", degrees_full);
				for(uint8_t i = 0; i < 6; i++)
					tmpp++;
			}
			else if(base == 10)
		    {
			    sprintf(tmpp, "%3d", sensor_buffer[sensor]);
			    //debug(tmpp);
				tmpp++; // Decimal-strängen är tre tecken
			    tmpp++;
			    tmpp++;
		    } else
		    {
			    sprintf(tmpp, "%02X", sensor_buffer[sensor]);
			    tmpp++; // Hex-strängen är två tecken
			    tmpp++;
		    }
		    continue;
	    }
    }
	clear_screen();
	send_string(tmpStr);
	update();

}



void init_default_printf_string()
{
	char default_string[] = {"V:%d\001,%d\004 F:%d\003 H:%d\002,%d\005" };
	strcpy(display_printf_string, default_string);
}