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

