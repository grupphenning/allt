/*
 * Styrmodul.c
 *
 * Created: 4/2/2013 2:20:45 PM
 *  Author: klawi021 ET AL!
 */
void debug(char *str);

#define F_CPU 8000000UL
#include "Styrmodul.h"
#include "pid.h"
#include <stdlib.h>
#include <string.h>
#include "spi.h"
#include "pwm.h"
#include "sensor.h"
#include "comm.h"

uint8_t speed = 255;


int main(void)
{
	
	//don't turn!
	turn = 0;
	//OSCCAL = 0x70;
	//display ska ut
//	DDRA = 0xFF;


	
	spi_init();
	pwm_init();
	sei();		//aktivera global interrupts
	init_display();
	update();
	
	clear_pid();
	init_pid(50, -50);
	update_k_values(100, 0, 170);
	
	
	sensor_buffer_pointer = 0x00;	// Pekare till aktuell position i bufferten
	sensor_start = 1;				// Flagga som avgör huruvida vi är i början av meddelande
	sensor_packet_length = 0x00;			// Anger aktuell längd av meddelandet
	init_default_printf_string();
	
	clear_screen();
	update();
	while(1)
	{
			uint8_t has_comm_data, has_sensor_data, comm_data, sensor_data;
		
		do_spi(&has_comm_data, &has_sensor_data, &comm_data, &sensor_data);
		
		if(has_comm_data) decode_comm(comm_data);
		if(has_sensor_data) decode_sensor(sensor_data);
		
// 		if(turn)
// 			tank_turn_left(speed);
// 		else
// 			stop_motors();
			//drive_forwards(speed);
		
//		if (follow_end_tape)
//		{
			//regulate_end_tape_2(spi_data_from_sensor);
			//regulate_end_tape(reflex_sensors_currently_seeing_tape(spi_data_from_sensor));
// 			char temp[32];
// 			sprintf(temp,"%03d ", spi_data_from_sensor[1]);
// 			send_string(temp);
// 			update();
// 			
			//reflex_sensors_currently_seeing_tape(spi_data_from_sensor);
//		}
// 		if (follow_end_tape)
// 		{
// 			regulate_end_tape(spi_data_from_sensor);
// 		}
		if (regulator_enable && regulator_flag)
		{
			regulator(sensor_buffer[IR_RIGHT_FRONT] - sensor_buffer[IR_RIGHT_BACK], 
					  sensor_buffer[IR_LEFT_FRONT] - sensor_buffer[IR_LEFT_BACK], 
					  sensor_buffer[IR_RIGHT_FRONT] - sensor_buffer[IR_LEFT_FRONT], 
					  sensor_buffer[IR_RIGHT_BACK] - sensor_buffer[IR_LEFT_BACK]);
			regulator_enable = 0;
		}	
		
	}
}


void send_sensor_buffer_to_remote(void)
{
	unsigned i;
	send_byte_to_comm('s');
	send_byte_to_comm(32);
	for(i = 0; i < 32; ++i) send_byte_to_comm(sensor_buffer[i]);
}





/*
..................................................|         /  _________________     __              __    __              __.........................
..................................................|        /  |  ______________  |  |  \            /  |  |  \            /  |........................
..................................................|       /   | |              | |  |   \          /   |  |   \          /   |........................
..................................................|      /    | |              | |  |    \        /    |  |    \        /    |.......................
..................................................|     /     | |              | |  |     \      /     |  |     \      /     |........................
..................................................|    /      | |   O     =    | |  |      \    /      |  |      \    /      |.......................
..................................................|   /       | |              | |  |       \  /       |  |       \  /       |.......................
..................................................|  /        | |     /        | |  |        \/        |  |        \/        |........................ 
..................................................|  \        | |    /_        | |  |                  |  |                  |........................
..................................................|   \       | |              | |  |                  |  |                  |........................
..................................................|    \      | |              | |  |                  |  |                  |........................
..................................................|     \     | |              | |  |                  |  |                  |........................
..................................................|      \    | |   \_____/    | |  |                  |  |                  |........................
..................................................|       \   | |              | |  |                  |  |                  |........................
..................................................|        \  | |______________| |  |                  |  |                  |........................
..................................................|         \ |__________________|  |                  |  |                  |........................
*/


//kör i 50 Hz! Ändra ej frekvensen, då denna även
//används till gripklon, som måste köras i 50 Hz!
ISR(TIMER1_COMPA_vect)
{
	if(turn)
	{
		ninety_timer++;		
	}

	//en sekund har gått
	if(ninety_timer == 17)
	{
		turn = 0;
		//tank_turn_left(255);
		ninety_timer=0;
	}

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

//overflow på timer0, ställ in frekvens med
//OCR0A, och CSxx-flaggorna i TCCR0B
ISR(TIMER0_OVF_vect)
{
	
	
}



