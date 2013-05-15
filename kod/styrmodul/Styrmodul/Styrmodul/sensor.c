/*
 * tape.c
 *
 * Created: 5/5/2013 4:34:58 PM
 *  Author: davek282
 */ 
void debug(char *str);

#include <avr/interrupt.h>
#include "bitmacros.h"
#include "Styrmodul.h"
#include "sensor.h"

#include "../../../sensormodul/sensormodul/sensormodul.h"

extern uint8_t is_returning_home;

uint8_t autonomous;
uint8_t is_returning_home;
char crossing_buffer[256];
uint8_t crossing_buffer_p;

volatile uint8_t ninety_timer, turn, pid_timer;
uint8_t left = 1;

uint8_t gyro_init_value;						//Gyrots initialvärde
uint8_t regulator_enable = 0;					//Flagga för att indikera 40 ms åt regulatorn.

int16_t turn_full;

// Denna ancänds bara av inte-interrupt-koden
uint8_t sensor_buffer[SENSOR_BUFFER_SIZE];		// Buffer som håller data från sensorenheten
uint8_t tape_command;
uint8_t tmp_sensor_buffer[256];					// Tape-argument från sensorenheten
uint8_t tmp_sensor_buffer_p;					// Pekare till aktuell position i bufferten
uint8_t sensor_start;							// Flagga som avgör huruvida vi är i början av meddelande
uint8_t tmp_sensor_buffer_len;					// Anger aktuell längd av meddelandet
// Innehåller de spänningsvärden som ses i grafen på https://docs.isy.liu.se/twiki/pub/VanHeden/DataSheets/gp2y0a21.pdf, sid. 4.
uint8_t small_ir_voltage_array[122] = {140, 139, 138, 137, 136, 135, 134, 133, 132, 131, 130, 129, 128, 127,
									   126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113,
									   112, 111, 110, 109, 108, 107, 106, 105, 104, 103, 102, 101, 100, 99,
									   98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82,
									   81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65,
									   64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48,
									   47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31,
									   30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19};	
uint8_t small_ir_centimeter_array[122] = {8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10,
										  10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 13, 
										  13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15,
										  16, 16, 16, 16, 17, 17, 17, 18, 18, 18, 19, 19, 19, 19, 20, 20, 20, 21,
										  21, 22, 22, 23, 23, 23, 24, 24, 25, 25, 26, 26, 27, 28, 28, 29, 29, 30, 
										  31, 32, 33, 34, 36, 37, 38, 39, 40, 41, 43, 44, 46, 47, 49, 50, 53, 55,
										  58, 60, 63, 65, 68, 70, 75, 80, 85, 90};	
uint8_t big_ir_voltage_array[117] =  {138, 137, 136, 135, 134, 133, 132, 131, 130, 129, 128, 127, 126, 125, 124, 
									  123, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113, 112, 111, 110, 109, 
									  108, 107, 106, 105, 104, 103, 102, 101, 100, 99, 98, 97, 96, 95, 94, 93, 
									  92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 
									  74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 
									  56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 
									  38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22};	
uint8_t big_ir_centimeter_array[117] = {16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 20, 21, 21, 21, 22, 22, 23, 23, 23, 24, 24,
										24, 25, 25, 25, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 30, 30, 30, 31, 31, 32, 32, 
										33, 33, 33, 34, 34, 35, 35, 35, 36, 36, 37, 37, 38, 38, 38, 39, 39, 40, 40, 41, 41, 
										42, 42, 43, 43, 44, 44, 45, 46, 47, 48, 49, 50, 51, 53, 54, 55, 56, 58, 59, 60, 60, 
										61, 62, 63, 63, 64, 64, 65, 67, 68, 70, 73, 75, 78, 80, 82, 84, 86, 88, 90, 93, 97, 
										100, 105, 110, 113, 117, 120, 125, 130, 135, 140, 145, 150};
// Innehåller de centimetervärden som ses i grafen på https://docs.isy.liu.se/twiki/pub/VanHeden/DataSheets/gp2y0a21.pdf, sid. 4.


uint8_t follow_end_tape = 1;


//Kalibrering
uint8_t calibrate_sensors = 0;

//Korsningsgrejer
uint8_t crossings = 0;
uint8_t tape_crossings = 0;
uint8_t has_detected_crossing = 0;
char crossing_direction;
uint8_t crossing_stop_value;
uint8_t make_turn_flag = 0;
uint8_t drive_from_crossing = 0;
uint8_t first_time_in_tape_crossing = 1;
uint8_t listening_to_gyro = 1;

/* Flagga som anger huruvida displayen ska uppdateras varje gång nya sensorvärden kommer in från sensorenheten */
uint8_t display_auto_update = 1;	// Påslagen som default
uint8_t counter = 0;




uint8_t *reflex_sensors_currently_seeing_tape(uint8_t * values)
{
	uint8_t i, offset=5;
	uint8_t return_values[11];
	for (i = 0;i < 11;i++)
	{
		if(values[i+offset] > REFLEX_SENSITIVITY)
			return_values[i] = 1;
		else
			return_values[i] = 0;
	}
	
	return return_values;
}


uint8_t is_empty(uint8_t * values, uint8_t len)
{
	uint8_t i;
	for (i = 0;i<len;i++)
	{
		if(values[i + 5] != 0)
			return 0;
	}
	
	return 1;
}


void regulate_end_tape_4()
{
	speed = 50;
	static uint8_t hihi = 0;
	if(hihi++ > 100)
	{
		hihi = 0;
		send_string("1");
		update();
	}
	
	int8_t pos = tape_command; //-5 för längst till vänster, 5 för höger, 0 i mitten!
	if(pos > 0)
	{
		RIGHT_AMOUNT = speed + 30 + pos*5;
		if(pos < 7)
		LEFT_AMOUNT = speed - 20 - pos*5;
		else
		LEFT_AMOUNT = 0;
		setbit(PORT_DIR, LEFT_DIR);
		setbit(PORT_DIR, RIGHT_DIR);
	}
	
	else if(pos < 0)
	{
		LEFT_AMOUNT = speed + 30 + abs(pos)*5;
		if(abs(pos) < 7)
		RIGHT_AMOUNT = speed - 20 - abs(pos)*5;
		else
		RIGHT_AMOUNT = 0;
		setbit(PORT_DIR, LEFT_DIR);
		setbit(PORT_DIR, RIGHT_DIR);
	}
	
	else //if(pos == 0)
	{ // == 0
		drive_forwards(speed);
	}
}




void decode_tape_sensor_data()
{
	static uint8_t tape_count=0;
	static uint8_t no_tape_count = 0;
	static uint8_t is_over_tape = 0;
	static uint8_t is_in_tape_segment = 0;
	static char first_tape;
	static char second_tape;
	
	
	
	static uint8_t used_sensor = REFLEX6;
	
	
	if(make_turn_flag && !crossings)
	{
		disable_pid();
		decode_tape_segment(first_tape, second_tape);
	}
	else if (sensor_buffer[used_sensor]>REFLEX_SENSITIVITY) //Tejpbitsstart hittad
	{
		crossings= 0;
		is_over_tape = 1;
		no_tape_count = 0;
		tape_count++;
	}
	
	else if (is_over_tape && sensor_buffer[used_sensor]<REFLEX_SENSITIVITY) //Tejpbit avslutad
	{
		is_over_tape = 0;
		if(is_in_tape_segment) //Andra tejpbiten avslutad
		{
			is_in_tape_segment = 0;
			second_tape = tape_count <= 5 ? 's': 'l';
			make_turn_flag = 1;
			
		}
		else // Första tejpbit avslutad
		{
			is_in_tape_segment = 1;
			first_tape = tape_count <= 5 ? 's': 'l';
		}
		tape_count=0;
	}
	
	else if(sensor_buffer[used_sensor]<REFLEX_SENSITIVITY) //Utanför tejp
	{
		//checka om den bara sett en tejpbit, alltså är den vid mål
		if(first_tape == 's' && no_tape_count > 7 && is_in_tape_segment)
		{
			decode_tape_segment(first_tape, 0);
		}
		is_in_tape_segment = no_tape_count++ > 7 ? 0 : is_in_tape_segment; //vilken jävla oneliner!
		
		
		//overflowskydd
		if(no_tape_count == 255)
		no_tape_count = 255;
		
	}
	
}


void decode_tape_segment(char first, char second)
{
	char turn_dir;
	if(first == 's' && second == 'l') turn_dir ='l'; //Turn left
	else if (first == 'l' && second == 's') turn_dir ='r'; //Turn right
	else if (first == 's' && second == 's') turn_dir = 'f'; //Turn front
	else if (first == 's' && !second) follow_end_tape = 1; //första var smal, andra fanns ej, vi är vid mål!
	
	switch(turn_dir)
	{
		case 'l':
		if(sensor_buffer[IR_LEFT_BACK] >= SEGMENT_LENGTH) make_turn(turn_dir);
		break;
		case 'r':
		if(sensor_buffer[IR_RIGHT_BACK] >= SEGMENT_LENGTH) make_turn(turn_dir);
		break;
		case 'f':
		make_turn('f');
		break;
		default:
		break;
	}
	if(drive_from_crossing)
	{
		if (sensor_buffer[IR_LEFT_BACK] <= SEGMENT_LENGTH && sensor_buffer[IR_RIGHT_BACK] <= SEGMENT_LENGTH)
		{
			stop_motors();
			drive_from_crossing = 0;
			enable_crossings();
			enable_pid();
		}
	}
	
}

// Global så att även handle_display.h kan läsa den
int16_t degrees_full;
void decode_sensor(uint8_t data)
{
	static uint16_t d = 0;
	static uint8_t sensor_transmission_number = 0;
	
	//Tilläggsvariabler
	static uint8_t left_back=0, left_front=0, right_back=0, right_front = 0; 
	static uint8_t reading_tape = 0;

	/* Första byten i ett meddelande är storleken */
	if(sensor_start)
	{
		tmp_sensor_buffer_len = data;
		sensor_start = 0;
		return;
	}

	/* Annars, lägg in inkommande byten i bufferten */
	tmp_sensor_buffer[tmp_sensor_buffer_p++] = data;

	/* Om det är fler byte kvar att ta emot, vänta på dem! */
	if(tmp_sensor_buffer_p != tmp_sensor_buffer_len + 1)
		return;
	/* Aha, vi har tagit emot hela meddelandet! Tolka detta! */
	sensor_start = 1;

	/* Kolla kontrollsumma */
	uint8_t sum = 0;
	unsigned i;
	for(i = 0; i < tmp_sensor_buffer_len; ++i) {
		sum += tmp_sensor_buffer[i];
	}
	if(sum != tmp_sensor_buffer[tmp_sensor_buffer_len]) {
		debug("CS");
		//sensor_start = 1;
		//return;
	}
/**********************************************************************
 * Här hanteras kommandon från sensorenheten
 **********************************************************************/
	switch(tmp_sensor_buffer[0]) {
		case SENSOR_DEBUG:
			//sensor_debug_message();
			break;
		case SENSOR_HEX:
			//sensor_debug_hex();
			break;
		case GYRO_SENSOR:
			stop_motors();
			break;	
		case SENSOR_GYRO_INTEGRAL:
			
			if(!listening_to_gyro)
			{
				break;
			}
			
			degrees_full = tmp_sensor_buffer[2] | (tmp_sensor_buffer[1] << 8);
			if(abs(degrees_full) > abs(turn_full)) {
				turn = 0;
				listening_to_gyro = 0;
				if (autonomous) make_turn(crossing_direction);
			}
			
			if(1||d++ % 64 == 0)
			{
				char integral_string[32];
				sprintf(integral_string, "Integral: %d", tmp_sensor_buffer[2] | (tmp_sensor_buffer[1] << 8));
				debug(integral_string);
			}
			break;
		case SENSOR_IR:
			memcpy(sensor_buffer, tmp_sensor_buffer, tmp_sensor_buffer_len);
			
			//Omvandla sensorvärden från spänningar till centimeter.
			sensor_buffer[IR_FRONT] = interpret_big_ir(sensor_buffer[IR_FRONT]);
			sensor_buffer[IR_LEFT_FRONT] = interpret_big_ir(sensor_buffer[IR_LEFT_FRONT])+left_front;
			sensor_buffer[IR_RIGHT_FRONT] = interpret_big_ir(sensor_buffer[IR_RIGHT_FRONT])+right_front;
			sensor_buffer[IR_LEFT_BACK] = interpret_small_ir(sensor_buffer[IR_LEFT_BACK])+left_back;
			sensor_buffer[IR_RIGHT_BACK] = interpret_small_ir(sensor_buffer[IR_RIGHT_BACK])+right_back;

			if(calibrate_sensors)
			{
				int8_t left_diff = sensor_buffer[IR_LEFT_FRONT]-sensor_buffer[IR_LEFT_BACK];
				int8_t right_diff = sensor_buffer[IR_RIGHT_FRONT]-sensor_buffer[IR_RIGHT_BACK];
				//Left
				if(left_diff>0)
				{
					left_back = left_diff;
				}
				else if(left_diff<0)
				{
					left_front = abs(left_diff);
				}
				//Right
				if(right_diff>0)
				{
					right_back = right_diff;
				}
				else if(right_diff<0)
				{
					right_front = abs(right_diff);
				}
				
				calibrate_sensors = 0;			
			}
		
			//Wait a few times before using data
			if(sensor_transmission_number<4)
			{
				sensor_transmission_number++;
				return;
			}


			if(autonomous) {
				// Om vi ska köra hemåt
				if(is_returning_home)
				{
					static uint8_t turn_first = 1;
					char c;
					// Analysera sensordata för att ta reda på när vi hittat en korsning.
					if (!has_detected_crossing && !make_turn_flag && !drive_from_crossing &&
					(sensor_buffer[IR_LEFT_FRONT] >= SEGMENT_LENGTH || sensor_buffer[IR_RIGHT_FRONT] >= SEGMENT_LENGTH))
					{
						analyze_ir_sensors();
					}
					// Korsning funnen
					if(has_detected_crossing)
					{
						drive_to_crossing_end(crossing_stop_value);
					}
					if(make_turn_flag && turn_first) 
					{
						// Sväng i motsatt riktning gentemot buffern, samt räkna ner bufferpekaren
						c = crossing_buffer[--crossing_buffer_p];
						c =
							c == 'l' ? 'r'
							: c == 'r' ? 'l'
							: c;
						turn_first = 0;
					}					
					if(make_turn_flag)
					{
						// Kolla om svängen är klar
						make_turn(c);
						if(make_turn_flag == 0 && crossing_buffer_p == 0)
						{
							// TODO kör ut sista sträckan från labyrinten
							is_returning_home = 0;
							//stop_motors();
						}
					}
					if(!make_turn_flag) turn_first = 1;
				}
				else 
				{	
					
					if(crossings||tape_crossings)
					{
						/*Hantera korsningar*/ /*Hantera tejp-korsningar Bra flaggor satta!*/
						handle_crossing();
					}
					
  					if (follow_end_tape)
  					{
  						//regulate_end_tape_3();
  					}

					//Temp för debug av hemkörning
					if(!turn && sensor_buffer[IR_FRONT] < SEGMENT_LENGTH/2 && sensor_buffer[IR_LEFT_FRONT] < SEGMENT_LENGTH/2 && sensor_buffer[IR_RIGHT_FRONT]< SEGMENT_LENGTH/2)
					{
						//stop_motors();
						/*_delay_ms(250);
						turn = 1;
						tank_turn_left(speed);
						stop_motors();
						_delay_ms(250);
						turn = 1;
						tank_turn_left(speed);*/
						//_delay_ms(5000);
						//is_returning_home = 1;
					}
				}
			}
			//update_display_string();
			break;
			
			case SENSOR_TAPE:
				tape_command = tmp_sensor_buffer[1];
			
				if(autonomous)
				{
					if(!first_time_in_tape_crossing)
						break;
					
					if(tape_command == 'f' || tape_command == 'r' || tape_command == 'l')
					{
						first_time_in_tape_crossing = 0;
						crossing_direction = tape_command;
						tape_crossings = 1;
						make_turn_flag = 1;
						has_detected_crossing = 0;
						crossings = 0;
						stop_motors();	
					}
					else if (tape_command == 'g')
					{
						follow_end_tape = 1;
						claw_out();
					}
					
                    //make_turning_decision();
				}
				
                    break;
            case SENSOR_FOLLOW_TAPE:
				tape_command = tmp_sensor_buffer[1];
				if(autonomous)
				{
                    regulate_end_tape_4();
				}				
                    break;
            case SENSOR_FOLLOW_TAPE_END:
                    {
						
						if(autonomous)
						{
                            stop_motors();
							claw_in();
                            /*
                            OBS!!! 
                            HÄR SKALL KLO STÄNGAS OCH 180-GRADERSSVÄNGEN INITIERAS!!!
                            Kommer den att ha åkt för långt då?<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
                            */
               
                            //is_returning_home = 1;        // OBS! SKALL GÖRAS EFTER ATT 180-GRADERSSVÄNGEN UTFÖRTS!!!
                            follow_end_tape = 0;
							speed = 0;
						}							        
                    }	
					break;
			case SENSOR_TAPE_DEBUG:
					{
						char tmp[20];
						sprintf(tmp, "Tejp: %d ", tmp_sensor_buffer[1]);
						debug(tmp);					
					}						
		default:
				// Unimplemented command
			break;
		}

	
	/* Återställ pekare, för att visa att nästa byte är början av meddelande */
	tmp_sensor_buffer_p = 0x00;
	tmp_sensor_buffer_len = 0;
	
	static uint8_t a=0;
	if((a++ & 0b10000) && display_auto_update)
	{
		a=0;
		update_display_string();
	}

	static uint16_t c = 0;
	if(c++ % 512 == 0)
		send_sensor_buffer_to_remote();
	
}


void sensor_debug_message()
{
	clear_screen();
	send_string(tmp_sensor_buffer+1);
	update();
}

void sensor_debug_hex()
{
	uint8_t pos = 1;	// Gå förbi DEBUG_HEX-kommandot
	char tmpstr[10];
	while(pos != tmp_sensor_buffer_len)
	{
		sprintf(tmpstr, "%02X", tmp_sensor_buffer[pos++]);
		send_string(tmpstr);
	}
	update();
}

//void interpret_small_ir(uint8_t value) //wtf??
uint8_t interpret_small_ir(uint8_t value)
{
 	uint8_t i = 0;
	
	if (value<19)
	{
		return 90;
	} 
	else if (value>140)
	{
		return 8;
	}
 	while ( !(value == small_ir_voltage_array[i] ) )
 	{
 		i++;
 	}
 	value = small_ir_centimeter_array[i];
 	return small_ir_centimeter_array[i];
}

//void interpret_big_ir(uint8_t value) //wtf??
uint8_t interpret_big_ir(uint8_t value)
{
	uint8_t i = 0;
	if (value<22)
	{
		return 150;
	}
	else if (value>138)
	{
		return 16;
	}
	while ( !(value == big_ir_voltage_array[i]) )
	{
		i++;
	}
	value = big_ir_centimeter_array[i];
	return big_ir_centimeter_array[i];
}

void handle_crossing()
{
	//Om ej i korsning och får sensordata som indikerar korsning. Analysera korsningstyp, sätt direction och has_detected_crossing_flagga
	//KOMMENTERA IN DENNA FÖR KORSNINGAR
	if (!has_detected_crossing && !make_turn_flag && !drive_from_crossing && 
		(sensor_buffer[IR_LEFT_BACK] >= SEGMENT_LENGTH || sensor_buffer[IR_RIGHT_BACK] >= SEGMENT_LENGTH))
	{
		analyze_ir_sensors();
	}
			
	//Om korsning detekterad. Kör till korsningens rotationscentrum, och sätt is_turning flagga
	else if(has_detected_crossing)
	{
		//disable_pid();
		drive_to_crossing_end(crossing_stop_value);
	}
	//Sväng tills villkor för svängning uppfyllt, sätt  is_turning flagga, nollställ begin turning
	else if (make_turn_flag)
	{
		make_turn(crossing_direction);
	}
	//Kör ut från korsningen till regleringen är redo att slås på igen, nollställ drive_from_crossing flagga
	else if(drive_from_crossing)
	{
		if (sensor_buffer[IR_LEFT_BACK] <= SEGMENT_LENGTH-30 && sensor_buffer[IR_RIGHT_BACK] <= SEGMENT_LENGTH - 30)
		{
			drive_from_crossing = 0;
			crossings = 1;
			tape_crossings = 0;
			first_time_in_tape_crossing = 1;
			listening_to_gyro = 1;
		}
	}
	else
	{
		regulator_enable = 1;		//Här har det gått ~40 ms dvs starta regleringen.
	}
}
/* 
 * ANALYZE_IR_SENSORS()
 * Vid upptäckt av någon form av korsning anropas denna.
 * Funktionen väljer rätt korsningstyp utifrån sensordata.
 * Flaggor som talar om vilken korsning ddet är sätts.
 */
void analyze_ir_sensors()
{
	uint8_t sample_limit = 1;
	static uint8_t decision_tlar_count = 0, decision_tral_count = 0, decision_tlaf_count = 0, 
			       decision_traf_count = 0, decision_tfal_count = 0, decision_tfar_count = 0,
				   decision_tr_count = 0, decision_tl_count = 0; 
	
	//Turn right
	if (sensor_buffer[IR_RIGHT_FRONT] >= MAXIMUM_IR_DISTANCE &&
		sensor_buffer[IR_FRONT] <= SEGMENT_LENGTH &&
		sensor_buffer[IR_LEFT_FRONT] <= SEGMENT_LENGTH)
	{
		if(++decision_tr_count == sample_limit) has_detected_crossing = 1;
		crossing_direction = 'r';
		crossing_stop_value = SEGMENT_LENGTH/2-IR_FRONT_TO_MIDDLE_LENGTH +OFFSET;
	}
	//Turn left
	else if (sensor_buffer[IR_LEFT_FRONT] >= MAXIMUM_IR_DISTANCE &&
			 sensor_buffer[IR_FRONT] <= SEGMENT_LENGTH &&
			 sensor_buffer[IR_RIGHT_FRONT] <= SEGMENT_LENGTH)
	{
		if(++decision_tl_count == sample_limit) has_detected_crossing = 1;
		crossing_direction = 'l';
		crossing_stop_value = SEGMENT_LENGTH/2-IR_FRONT_TO_MIDDLE_LENGTH +OFFSET;
	}
	//Turn left, alley right												
	else if(sensor_buffer[IR_LEFT_FRONT] >= MAXIMUM_IR_DISTANCE &&
		sensor_buffer[IR_FRONT] <= SEGMENT_LENGTH &&
		sensor_buffer[IR_RIGHT_FRONT] >= SEGMENT_LENGTH &&
		sensor_buffer[IR_RIGHT_FRONT] <= MAXIMUM_IR_DISTANCE)
	{
		if(++decision_tlar_count == sample_limit) has_detected_crossing = 1;
		crossing_direction = 'l';
		crossing_stop_value = SEGMENT_LENGTH/2-IR_FRONT_TO_MIDDLE_LENGTH +OFFSET;
	}	
	//turn right alley to left															
	else if(sensor_buffer[IR_LEFT_FRONT] >= SEGMENT_LENGTH &&
		sensor_buffer[IR_LEFT_FRONT] <= MAXIMUM_IR_DISTANCE &&
		sensor_buffer[IR_FRONT] <= SEGMENT_LENGTH &&
		sensor_buffer[IR_RIGHT_FRONT] >= MAXIMUM_IR_DISTANCE)
	{
		if(++decision_tral_count == sample_limit) has_detected_crossing = 1;
		crossing_direction = 'r';
		crossing_stop_value = SEGMENT_LENGTH/2-IR_FRONT_TO_MIDDLE_LENGTH+OFFSET;
	}
	//Turn left, alley at front
	else if(sensor_buffer[IR_LEFT_FRONT]>=MAXIMUM_IR_DISTANCE &&
		sensor_buffer[IR_FRONT] >= DISTANCE_TO_ALLEY_END &&
		sensor_buffer[IR_FRONT] <= MAXIMUM_IR_DISTANCE &&
		sensor_buffer[IR_RIGHT_FRONT] <= SEGMENT_LENGTH)
	{
		if(++decision_tlaf_count == sample_limit) has_detected_crossing = 1;
		crossing_direction = 'l';
		crossing_stop_value = DISTANCE_TO_ALLEY_END - IR_FRONT_TO_MIDDLE_LENGTH + OFFSET;
	}
	//Turn Right, alley at front
	else if(sensor_buffer[IR_LEFT_FRONT]<=SEGMENT_LENGTH &&
		sensor_buffer[IR_FRONT] >= DISTANCE_TO_ALLEY_END &&
		sensor_buffer[IR_FRONT] <= MAXIMUM_IR_DISTANCE &&
		sensor_buffer[IR_RIGHT_FRONT] >=MAXIMUM_IR_DISTANCE)
	{
		if(++decision_traf_count == sample_limit) has_detected_crossing = 1;
		crossing_direction = 'r';
		crossing_stop_value = DISTANCE_TO_ALLEY_END - IR_FRONT_TO_MIDDLE_LENGTH + OFFSET;
	}
	//turn front alley left
	else if(sensor_buffer[IR_LEFT_FRONT] >= SEGMENT_LENGTH &&
		sensor_buffer[IR_LEFT_FRONT] < MAXIMUM_IR_DISTANCE &&
		sensor_buffer[IR_FRONT] >= MAXIMUM_IR_DISTANCE &&
		sensor_buffer[IR_RIGHT_FRONT] <= SEGMENT_LENGTH)
	{

		if(++decision_tfal_count == sample_limit) has_detected_crossing = 1;
		crossing_direction = 'f';
		crossing_stop_value = 0;
	}
	//turn front alley right
	else if(sensor_buffer[IR_LEFT_FRONT] <= SEGMENT_LENGTH &&
		sensor_buffer[IR_FRONT] >= MAXIMUM_IR_DISTANCE &&
		sensor_buffer[IR_RIGHT_FRONT] >= SEGMENT_LENGTH &&
		sensor_buffer[IR_RIGHT_FRONT] <=MAXIMUM_IR_DISTANCE)
	{
		if(++decision_tfar_count == sample_limit) has_detected_crossing = 1;
		crossing_direction = 'f';
		crossing_stop_value = 0;
	}
	if(has_detected_crossing)
	{
		decision_tfal_count = decision_tlaf_count = decision_tl_count = decision_tr_count = decision_tlar_count = decision_tral_count = decision_traf_count = decision_tfar_count = 0;
	}	
}


void add_to_crossing_buffer(char c)
{
	crossing_buffer[crossing_buffer_p++] = c;
}

/*
 * CROSSING_TURN(STOP_DISTANCE)
 * Körs då korning, samt korningstyp identifierats.
 * Utför sväng då roboten kört tillräckligt långt in i korsningen.
 */
void drive_to_crossing_end(uint8_t stop_distance)
{

	if ((sensor_buffer[IR_FRONT] <= stop_distance) || stop_distance == 0)
	{
		make_turn_flag = 1;
		stop_motors();
		has_detected_crossing = 0;
	}
	
}

void make_turn(char dir)
{
	uint16_t gyro_const = 1300;
	static uint8_t first = 1;
	//uint8_t turn_delay = 50;
	// Sväng färdig
	if (!turn && !first)
	{
		send_byte_to_sensor(STOP_TURN);
		stop_motors();
		spi_delay_ms(500);
		drive_forwards(speed);
		first = 1;
		make_turn_flag = 0;
		drive_from_crossing = 1;
		if(!is_returning_home) add_to_crossing_buffer(dir);
	}
	else
	{
		switch(dir)
		{
			case 'l':
				if(first)
				{
					turn = 1;
					//sätt igång timern!
// 					setbit(TCCR3B, CS30);
// 					setbit(TCCR3B, CS32);
					send_byte_to_sensor(START_TURN);
					turn_full = -gyro_const;
					
					spi_delay_ms(1000);
					first = 0;
					tank_turn_left(speed);
				}
			break;
			
			case 'r':
				if(first)
				{
					turn = 1;
					
					//sätt igång timern!
// 					setbit(TCCR3B, CS30);
// 					setbit(TCCR3B, CS32);
					send_byte_to_sensor(START_TURN);
					turn_full = gyro_const+100;
					
					spi_delay_ms(1000);
					first = 0;
					tank_turn_right(speed);
				}
			break;
			
			case 'f':
// 				stop_motors();
// 				_delay_ms(10000);
				 
				first = 0;
				break;
			
			default: break;
		}
	}

}
void enable_crossings()
{
	crossings = 1;
	has_detected_crossing = 0;
	make_turn_flag = 0;
	drive_from_crossing = 0;
}

void disable_crossings()
{
	crossings = 0;
}

//Används ej!
// void turn_left90()
// {
// 	static uint8_t front = 0;
// 	
// 	if(is_turning && abs(front - sensor_buffer[IR_RIGHT_FRONT]) <= 1  && sensor_buffer[IR_FRONT] >=150)
// 	{
// 		is_turning = 0;
// 		drive_from_crossing = 1;
// 		
// 		stop_motors();
// 		_delay_ms(250);
// 		drive_forwards(speed);
// 	}
// 	else if(!is_turning) 
// 	{
// 		front = sensor_buffer[IR_FRONT];
// 		is_turning = 1;
// 		stop_motors();
// 		_delay_ms(250);
// 		tank_turn_left(speed);
// 	}  
// 	
// }
//Används ej!
/*void turn_right90()
{
	static uint8_t front = 0;
	static uint8_t is_turning = 0;
	
	if(is_turning && front == sensor_buffer[IR_LEFT_FRONT] && sensor_buffer[IR_FRONT] >=150)
	{
		is_turning = 0;
		stop_motors();
		//Börjja köra sen
	}
	else if(!is_turning)
	{
		front = sensor_buffer[IR_FRONT];
		is_turning = 1;
		stop_motors();
		_delay_ms(500);
		tank_turn_right(speed);
	}
	
}*/


/********************************************************
				Linjeföljning test
********************************************************/ 

