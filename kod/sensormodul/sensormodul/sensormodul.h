#ifndef SENSORMODUL_H
#define SENSORMODUL_H

#include <inttypes.h>


/****************************************************
 Här är de kommandon som kan skickas av sensorenheten
 ****************************************************/
#define SENSOR_DEBUG	0x01
#define SENSOR_HEX		0x02
#define SENSOR			0x03
#define GYRO			0x04	
//FIXME: Lägg till fler!


/* Övriga definitioner */
#define GYRO PINA2
#define INTERUPT_REQUEST PINA7
#define TAPE_SENSOR PINA1
#define IR_SENSOR PINA0

void read_adc();
void read_ir(uint8_t sensor_no);
void read_gyro();
void init_adc();

#endif /* SENSORMODUL_H */