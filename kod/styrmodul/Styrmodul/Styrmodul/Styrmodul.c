/*
 * Styrmodul.c
 *
 * Created: 4/2/2013 2:20:45 PM
 *  Author: klawi021 ET AL!
 */
void debug(char *str, ...);

#define F_CPU 8000000UL
#include "Styrmodul.h"
#include "pid.h"
#include <stdlib.h>
#include <string.h>
#include "spi.h"
#include "pwm.h"
#include "sensor.h"
#include "comm.h"

uint8_t turn_dir;
uint8_t speed = 150;
uint8_t SPEED_OFFSET = 0;


int main(void)
{
	//Sväng inte
	turn = 0;
	
	//Initiera spi, pwm och display
	spi_init();
	pwm_init();
	init_display();
	update();
	
	claw_out();
	_delay_ms(500);
	claw_in();	
	
	//Aktivera global interrupts
	sei();
	
	//Initiera regulator
	clear_pid();	
	init_pid(80, -80);
	//update_k_values(40, 12, 22);
	update_k_values(40, 170, 20);
	
	// Pekare till aktuell position i bufferten
	tmp_sensor_buffer_p = 0x00;
	
	// Flagga som avgör huruvida vi är i början av meddelande	
	sensor_start = 1;
	
	// Anger aktuell längd av meddelandet				
	tmp_sensor_buffer_len = 0x00;
	
	//Initiera standardsträng på display		
	init_default_printf_string();
	clear_screen();
	update();
	
	
	while(1)
	{
		uint8_t has_comm_data, has_sensor_data, comm_data, sensor_data;
	
		do_spi(&has_comm_data, &has_sensor_data, &comm_data, &sensor_data);
		
		//Undersök och hantera meddelanden från slavarna
		if(has_comm_data) decode_comm(comm_data);
		if(has_sensor_data) decode_sensor(sensor_data);
		
		//Vid manuell sväng eller 180 grader måste make_turn anropas 		
		if(!autonomous || turning_180)
		{
			if(turn_dir) 
			{
				make_turn_flag = 1;
				make_turn(turn_dir);
				
				if(!make_turn_flag) 
				{
					turn_dir = 0;
					stop_motors();
				}				
			}
		}			
		
		//Kör regulatorn
		if (regulator_enable)
		{
			regulator(sensor_buffer[IR_RIGHT_FRONT] - sensor_buffer[IR_RIGHT_BACK], 
					  sensor_buffer[IR_LEFT_FRONT] - sensor_buffer[IR_LEFT_BACK], 
					  sensor_buffer[IR_RIGHT_FRONT] - sensor_buffer[IR_LEFT_FRONT], 
					  sensor_buffer[IR_RIGHT_BACK] - sensor_buffer[IR_LEFT_BACK]);
			regulator_enable = 0;
		}	
	}
}

//Skickar sensordata från IR-sensorerna till fjärrmodulen
void send_sensor_buffer_to_remote(void)
{
	unsigned i;
	send_byte_to_comm('s');
	send_byte_to_comm(3);
	for(i = 0; i < 3; ++i) send_byte_to_comm(sensor_buffer[i]);
}

//kör i 50 Hz! Ändra ej frekvensen, då denna även
//används till gripklon, som måste köras i 50 Hz!
ISR(TIMER1_COMPA_vect)
{
	static uint8_t integral_counter;
	if(integral_counter++ % 8 == 0) if(!turn) send_byte_to_sensor(STOP_TURN);
	
	// Refresha motor-output. Detta får äntligen styrningen att fungera pålitligt.
	if(dirbits & 1) {
		clearbit(PORT_DIR, LEFT_DIR);
		setbit(PORT_DIR, LEFT_DIR);
	}
	else {
		setbit(PORT_DIR, LEFT_DIR);
		clearbit(PORT_DIR, LEFT_DIR);
	}

	if(dirbits & 2) {
		clearbit(PORT_DIR, RIGHT_DIR);
		setbit(PORT_DIR, RIGHT_DIR);
	}
	else {
		setbit(PORT_DIR, RIGHT_DIR);
		clearbit(PORT_DIR, RIGHT_DIR);
	}

	uint8_t l, r;
	l = LEFT_AMOUNT;
	r = RIGHT_AMOUNT;

	if(l) {
		LEFT_AMOUNT = 0;
		LEFT_AMOUNT = l;
	}
	if(r) {
		RIGHT_AMOUNT = 0;
		RIGHT_AMOUNT = r;
	}		
}