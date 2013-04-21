#ifndef KOMMUNIKATIONSMODUL_H
#define KOMMUNIKATIONSMODUL_H

int main(void);
uint8_t SPI_SlaveReceive();
void SPI_read_byte();
void SPI_write_byte(uint8_t byte);
void init_spi();
void serial_send_byte(uint8_t val);
void USART_init();
void USART_Transmit(unsigned char data);
unsigned char USART_Receive(void);
void create_master_interrupt();
void decode_remote(uint8_t ch);
void send_to_master(uint8_t byte);


#endif /* KOMMUNIKATIONSMODUL_H */
