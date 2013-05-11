#ifndef SENSORMODUL_H
#define SENSORMODUL_H

#include <inttypes.h>


/****************************************************
 Här är de kommandon som kan skickas av sensorenheten
 ****************************************************/
#define SENSOR_DEBUG				0x01
#define SENSOR_HEX					0x02
#define SENSOR						0x03
#define GYRO_SENSOR					0x04
#define SENSOR_IR			        0x05
#define SENSOR_TAPE                 0x06
#define SENSOR_FOLLOW_TAPE          0x07
#define SENSOR_FOLLOW_TAPE_END      0x08
#define SENSOR_GYRO_INTEGRAL		0x27

#define TURN_RIGHT					0x10
#define TURN_LEFT					0x11	
//FIXME: Lägg till fler!
#define AUTONOMOUS_MODE				0x12
#define START_TURN					0x13
#define STOP_TURN					0x14


/* Övriga definitioner */
#define GYRO PINA2
#define INTERUPT_REQUEST PINA7
#define TAPE_SENSOR PINA1
#define IR_SENSOR PINA0

#define REFLEX_SENSITIVITY 0x70

void read_adc();
void read_ir(uint8_t sensor_no);
uint8_t read_gyro();
void init_adc();

#endif /* SENSORMODUL_H */