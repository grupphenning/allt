#include <avr/io.h>
#include "bitmacros.h"
#include <avr/delay.h>
#include "display.h"
#include <avr/interrupt.h>
#include "komm_styr_protokoll.h"
#include "sensor_styr_protokoll.h"


#define LEFT_DIR PB1
#define RIGHT_DIR PB0
#define PORT_DIR PORTB

#define LEFT_AMOUNT OCR2A
#define RIGHT_AMOUNT OCR2B

#define CLAW_AMOUNT OCR1A

#define LEFT_PWM PD7
#define RIGHT_PWM PD6
#define CLAW PD5
#define PORT_PWM PORTD

#define DISPLAY PORTA
#define DISPLAY_POWER PA2
#define DISPLAY_BLINK PA0
#define DISPLAY_CURSOR PA1
#define DISPLAY_RS PC7
#define DISPLAY_ENABLE PC6

#define IR_LEFT_FRONT	1
#define IR_RIGHT_FRONT  2
#define IR_FRONT		3
#define IR_LEFT_BACK	4
#define IR_RIGHT_BACK   5

#define REFLEX1			6
#define REFLEX2			7
#define REFLEX3			8
#define REFLEX4			9
#define REFLEX5			10
#define REFLEX6			11
#define REFLEX7			12
#define REFLEX8			13
#define REFLEX9			14
#define REFLEX10		15
#define REFLEX11		16

#define REFLEX_SENSITIVITY 0x70



//KORNINGSLÄNGDDATA
#define IR_FRONT_TO_MIDDLE_LENGTH		11
#define OFFSET							10	 
#define DISTANCE_TO_ALLEY_END			120
#define MAXIMUM_IR_DISTANCE				150
#define SEGMENT_LENGTH					80	
#define IR_SIDE_TO_MIDDLE_LENGTH		7	

void send_byte_to_comm(uint8_t byte);

uint8_t interpret_big_ir(uint8_t value);
uint8_t interpret_small_ir(uint8_t value);
void regulate_end_tape(uint8_t* values);
void regulate_end_tape_2(uint8_t* values);
void regulate_end_tape_3();
uint8_t * reflex_sensors_currently_seeing_tape(uint8_t * values);
void init_default_printf_string();
void analyze_ir_sensors();
void crossing_turn(char dir, uint8_t stop_distance);
uint8_t is_on_tape(uint8_t tape_number);
extern uint8_t dirbits;
