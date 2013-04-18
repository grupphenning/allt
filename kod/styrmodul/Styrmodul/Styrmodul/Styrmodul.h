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

<<<<<<< HEAD
void regulator_timer_init();
=======
#define IR_LEFT_FRONT	1
#define IR_RIGHT_FRONT  2
#define IR_LEFT_BACK	4
#define IR_RIGHT_BACK   5
#define IR_FRONT		3
#define GYRO			6
#define REFLEX1			7
#define REFLEX2			8
#define REFLEX3			9
#define REFLEX4			10
#define REFLEX5			11
#define REFLEX6			12
#define REFLEX7			13
#define REFLEX8			14
#define REFLEX9			15
#define REFLEX10		16
#define REFLEX11		17


>>>>>>> 402a9ca94911b78c9f245d32fa8dcb64ea290c25
