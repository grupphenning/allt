/*
 * comm.c
 *
 * Created: 5/5/2013 5:05:01 PM
 *  Author: davek282
 */ 
void debug(char *str);

#include <avr/interrupt.h>
#include "bitmacros.h"
#include "Styrmodul.h"
#include "handle_display.h"
#include "sensor.h"
#include "comm.h"

void decode_comm(uint8_t command)
{
	static uint8_t pid = 0;		// Hantera PID-argument
	static uint8_t constant_p;	// Akutellt PID-argument
	static uint8_t constant_i;	// Dito...
	static uint8_t constant_d;	// Dito...

	static uint8_t display_printf_p = 0; // Används vid uppdatering av printf-strängen
	static uint8_t display = 0;	// Dislayen uppdateras!
	static uint8_t drive = 0;	// Förra kommandot väntar på ett hastighetsargument
	static uint8_t set_speed = 0;

// 	char temp[32];
// 	sprintf(temp,"%02X ", command);
// 	send_string(temp);
// 	update();
	
	if(pid)
	{
// 		char tmp[30];
// 		sprintf(tmp, "%02X ",  pid);
// 		send_string(tmp);
// 		update();
		if(pid == 4){
			constant_p = command;
		}			
		else if(pid == 3){
			constant_i = command;
		}
		else if(pid == 2) {
			constant_d = command << 8;
		}
		else // pid == 1
		{
			constant_d |= command;
			update_k_values(constant_p, constant_i, constant_d);
		}
		--pid;
	} 
	else if(set_speed) {
		speed = command;
		set_speed = 0;
	}
	else if(display)
	{
		/* Slutet på printf-strängen? */
		if(command == 0x00)
		{
			update_display_string();
			display_printf_p = 0;
		} 
		else
		{
			if(display_printf_p == 0)
				memset(display_printf_string, 0, 100);
			display_printf_string[display_printf_p++] = command;
		}		
		display = 0;
		return;
	}
	else if(command == COMM_DRIVE)
	{
		drive_forwards(speed);
	} else if(command == COMM_BACK)
	{
		drive_backwards(speed);
	} else if(command == COMM_STOP)
	{
		disable_pid();
		stop_motors();
	} else if(command == COMM_LEFT)
	{
		tank_turn_left(speed);
	} else if(command == COMM_RIGHT)
	{
		tank_turn_right(speed);
	} else if(command == COMM_DRIVE_LEFT)	// Ignorera argumentet tills vidare, vet inte hur vi ska lösa det...
	{
		turn_left(speed);
	}	else if(command == COMM_DRIVE_RIGHT)	// Ignorera argumentet även här, tills vidare...
	{
		turn_right(speed);
	} else if(command == COMM_CLAW_OUT)
	{
		claw_out();
	} else if(command == COMM_CLAW_IN)
	{
		claw_in();
	}
	else if(command == COMM_STOP)
	{
		disable_pid();
		stop_motors();
	}
	else if(command == COMM_SET_PID)
	{
		pid = 4;
	}
	else if(command == COMM_ENABLE_PID)
	{
		autonomous = 1;
		send_byte_to_sensor(AUTONOMOUS_MODE);
//		crossing_buffer_p = 0;
		drive_forwards(speed);
		enable_pid();
		enable_crossings();
		setbit(PORT_DIR, LEFT_DIR);		//Kör framåt under regleringen.
		setbit(PORT_DIR, RIGHT_DIR);	
	}
	else if(command == COMM_DISABLE_PID)
	{
		autonomous = 0;
		disable_crossings();
		disable_pid();
	}
	else if(command == COMM_CLEAR_DISPLAY)
	{
		clear_screen();
		update();
	}		
	else if(command == COMM_DISPLAY)
	{
		display = 1;
	}
	else if(command == COMM_TOGGLE_SENSORS)
	{
		if(display_auto_update)
			display_auto_update = 0;
		else
			display_auto_update = 1;
/*				
#if 1
		if(bitset(EIMSK, INT0))
		{
			update_display_string();
			clearbit(EIMSK, INT0);	// Avaktivera avbrottsförfrågan från sensorenheten
		}			
		else
			setbit(EIMSK, INT0);	// Akvitera avbrottsförfrågan från sensorenheten
#else
is_returning_home = 1;
#endif
*/
		}
	else if(command == COMM_TURN_90_DEGREES_LEFT)
	{
		send_string("PERPENDICULAR LEFT");
		update();
	}
	else if(command == COMM_TURN_90_DEGREES_RIGHT)
	{
		send_string("PERPENDICULAR RIGHT");
		update();
	}
	else if(command == COMM_CALIBRATE_SENSORS)
	{
		calibrate_sensors = 1;
	}
	else if(command == COMM_SET_SPEED) {
		set_speed = 1;
	}	
	else	
	{
		char tmp[30];
		sprintf(tmp, "Err%02X ", command);
		send_string(tmp);
		update();
	}
}
