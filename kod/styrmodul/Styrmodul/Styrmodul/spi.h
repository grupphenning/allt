/*
 * spi.h
 *
 * Created: 5/5/2013 12:26:19 PM
 *  Author: davek282
 */ 


#ifndef SPI_H_
#define SPI_H_
#include <avr/io.h>
void spi_init();
void send_byte_to_senor(uint8_t byte);
void send_byte_to_comm(uint8_t byte);
void debug(char *str);
void do_spi(uint8_t *has_comm_data_out, uint8_t *has_sensor_data_out, uint8_t *comm_data_out, uint8_t *sensor_data_out);

#endif /* SPI_H_ */