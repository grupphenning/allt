/*
 * sensormodul.c
 *
 * Created: 2013-04-09 11:03:56
 *  Author: eriha181
 */
//definiera klockhastighet
#define F_CPU 8000000UL
#include "sensormodul.h"
#include "bitmacros.h"

#include <avr/io.h>
#include <avr/delay.h>
#include <avr/interrupt.h>

uint8_t spi_data_from_master;
uint8_t spi_data_to_master;
uint8_t tape_sensor;
uint8_t read_mode; // Om read_mode == 1, integrera gyro, annars läs IR och tejp.
char tape_type;

int16_t gyro_init_value;						//Gyrots initialvärde
int16_t full_turn, gyro_int;

volatile uint8_t tape_sensor_data[9];
volatile uint8_t ir_sensor_data[5+1];
volatile uint8_t decoded_tape_data[1+1];
volatile uint8_t tape_position[1+1];
volatile uint8_t test_data[16+1];
volatile uint8_t data_index=1;
volatile uint8_t adc_interrupt;
volatile uint8_t has_data_from_spi;
volatile uint8_t read_and_send_ir_to_master;
volatile uint8_t follow_end_tape;
volatile uint8_t autonomous;
volatile uint8_t to_read_gyro;
volatile uint8_t tape_interupt_flag;
volatile uint8_t spi_transfer_complete;

void read_byte(uint8_t b)
{
	has_data_from_spi = 1;
	spi_data_from_master = b;
}

int main(void)
{
	setbit(DDRA, PINA7); //Sätter avbrottpinne mot styr som output
	clearbit(DDRA,PINA0); //Sätter IR/Reflexdata som input
	setbit(DDRA,PINA1); //Sätter reflex-enable som output
	clearbit(DDRA,PINA2); // Gyro som input
	
	DDRD = 0xFF; //A,B,C,D till muxarna
	setbit(DDRC, PINC0);
	
	_delay_ms(100);	// Ja, vänta! Annars hamnar SPI-protokollet i osynk!
	
	init_spi();
	init_adc();
	init_gyro();
	//init_gyro_timer();
	init_sensor_timer();
	
	setbit(DDRB, PORTB3);

	
	
    while(1)
    {
		if(read_mode) {
			if(read_and_send_ir_to_master)
			{
				gyro_int += gyro_init_value - ((int16_t)read_gyro() * 16);			//Maxhastighet 300grader/s,
				read_and_send_ir_to_master = 0;
				uint8_t values[3];
				values[0] = SENSOR_GYRO_INTEGRAL;
				values[1] = (gyro_int / 16) >> 8;
				values[2] = (gyro_int / 16);
				send_to_master(3, values);					
			}
			if(to_read_gyro)
			{
				to_read_gyro = 0;
			}				
		}
		else {			
			if (read_and_send_ir_to_master && !follow_end_tape)
			{
				read_and_send_ir_to_master = 0;
				read_and_send_ir();
			}
			if (follow_end_tape)
			{
				data_index = 0;
				for (uint8_t i = 1; i < 10; i++)
				{
					read_tape(i);
				}
				regulate_end_tape();
			}
		}			

		
		if(SPSR & (1 << SPIF)) read_byte(SPDR);
		
		clearbit(PORTB, PORTB3);
		if(has_data_from_spi)
		{
			if(spi_data_from_master == AUTONOMOUS_MODE)
			{
				setbit(PORTB, PORTB3);
				autonomous = 1;
				//resetta allt, vi vill inte börja snurra o hålla på
				data_index=1;
				adc_interrupt = 0;
				has_data_from_spi = 0;
				read_and_send_ir_to_master = 0;
				follow_end_tape = 0;
				tape_sensor = 0;
				tape_type = 0;
			}
			else if(spi_data_from_master == START_TURN) {
				gyro_int = 0;
				read_mode = 1;
			}
			else if(spi_data_from_master == STOP_TURN) {
				read_mode = 0;
				gyro_int = 0;
			}
			
// 			switch(spi_data_to_master)
// 			{
// 				case TURN_RIGHT:
// 					begin_turning(90);
// 					break;
// 				case TURN_LEFT:
// 					begin_turning(-90);
// 					break;
// 				default:
// 					break; 
// 			}

			has_data_from_spi = 0;
		}	
		
		if (!follow_end_tape)
		{
			read_one_tape(); //AD-omvandlar andra tejpsensorn
			if(tape_sensor)
			{
				decode_tape();
				//send_decoded_tape(); //Onödigt skicka när beslut ej tagits.
			}
			tape_interupt_flag = 0;			
		}		
    }
}

/* VAFAN ÄR DETTA??
<<<<<<< HEAD
void init_gyro_timer()
{
	
}


void read_all_sensors()
=======
*/

/* =======================================GYRO=======================================*/



// Börja snurra. positiv degrees = medurs. Har slutat då is_turning blir 0.
void begin_turning(int16_t degrees)
//>>>>>>> f840052e7a203feb98207f4cf649ee4656753730 //wtf??
{
	full_turn = 55*abs(degrees);	//Sväng x antal grader
	gyro_int = 0;
//	init_gyro_timer();

}

/*
 * Initerar gyrot genom att utföra ett fåtal avläsningar,
 * kastar dessa som skräpvärden och sedan väljer ett som initialvärde till gyrot
 */
void init_gyro()
{
	static uint8_t gyro_reads_number = 0;
	
	while(gyro_reads_number<5)
	{
		read_gyro();
		gyro_reads_number++;
	}
	//uint16_t sum = 0;
	uint8_t i;
	//for(i = 0; i < 16; ++i) sum += read_gyro();
	gyro_init_value = 0;
	for(i = 0; i < 16; ++i) gyro_init_value += read_gyro();
	gyro_init_value += 1;
	//gyro_init_value = sum;
}


// 
void timed_gyro_turn()
{
	gyro_int += 3 * abs(gyro_init_value - (uint8_t) read_gyro());			//Maxhastighet 300grader/s,
	if(gyro_int >= full_turn)										//maxvärde-nollnivå ung 100.
	{
		disable_gyro_timer();
		send_stop_turn_message();
	}
}


/* ====================================SENSORAVLÄSNING=======================================*/

/*
 * Initiera adc-omvandling
 */

void init_adc()
{
	//reference voltage
	setbit(ADCSRA,REFS0);
	
	//ADC enable
	setbit(ADCSRA,ADEN);
	
	//Left adjust measurment
	setbit(ADMUX, ADLAR);
	//	setbit(ADMUX, MUX3);		// 10x gain!
	
	//Set ADC in free running- mode
	//setbit(ADCSRA,ADATE);
	
	//Enable interupt
	//setbit(ADCSRA,ADIE);
	
	//SKalning av klockfrekvens
	setbit(ADCSRA,ADPS2);
	setbit(ADCSRA,ADPS1);
	setbit(ADCSRA,ADPS0);
}

/*
 * Läs av alla sensorer ( Ej gyro)
 */
void read_all_sensors()
{
	setbit(PORTC, PINC0);		// For debug!
	data_index = 1;
	test_data[0] = SENSOR;
	
	read_ir(0);
	read_ir(1);
	read_ir(2);
	read_ir(3);
	read_ir(4);
	
	read_tape(0);
	read_tape(1);
	read_tape(2);
	read_tape(3);
	read_tape(4);
	read_tape(5);
	read_tape(6);
	read_tape(7);
	read_tape(8);
	read_tape(9);
	read_tape(10);
	clearbit(PORTC, PINC0);		// For debug!!
	send_to_master(16, test_data);
	setbit(PORTC, PINC0);		// For debug!!
	clearbit(PORTC, PINC0);		// For debug!!
}

/*
* Läser och skickar data från ir-sensorerna
*/

void read_and_send_ir()
{
	data_index = 1;
	ir_sensor_data[0] = SENSOR_IR;
	for (uint8_t i=0; i<5; i++)
	{
		read_ir(i);
	}
	send_to_master(6, ir_sensor_data);
}

void send_decoded_tape()
{
	data_index = 1; //Borde inte behövas, dubbelkolla det!
	decoded_tape_data[0] = SENSOR_TAPE;
	decoded_tape_data[1] = tape_type;
	send_to_master(2, decoded_tape_data);
}


/* 
 * Läser bara av tejpsensorer, skicka rådata eller beräkna mittpunkt här?!
 */


void read_and_send_tape()
{
	data_index = 1;
	test_data[0] = SENSOR;
	uint8_t i;
	for(i = 0; i < 11; i++)
	{
		read_tape(i);
	}
	
	send_to_master(11, test_data);
}

/*
 * Läs av gyrot.
 * OBS!! Använder inte read_adc() likt andra sensoravläsningsfunktioner
 */


uint8_t read_gyro()
{
	clearbit(ADMUX,MUX0);
	setbit(ADMUX,MUX1);
	clearbit(ADMUX,MUX2);
	clearbit(ADMUX,MUX3);
	clearbit(ADMUX,MUX4);
	//read_adc();
	setbit(ADCSRA,ADSC); //start_reading
	//while(!adc_interrupt); //Wait for interupt to occur
	while(bitclear(ADCSRA, ADIF));
	return ADCH;
}


/*
* Läs av ir-sensor med nr sensor_no
*/
void read_ir(uint8_t sensor_no)
{
	unsigned index = data_index;
	static uint8_t last[20], second_to_last[20];

	clearbit(ADMUX,MUX0);
	clearbit(ADMUX,MUX1);
	clearbit(ADMUX,MUX2);
	clearbit(ADMUX,MUX3);
	clearbit(ADMUX,MUX4);
	PORTD = ((11+sensor_no)<<4); //Tell mux where to read from
	read_adc_ir();
	
	/*MJUKVARUFILTER*/
	second_to_last[sensor_no] = last[sensor_no];
	last[sensor_no] = ir_sensor_data[index];
	
	ir_sensor_data[index] = second_to_last[sensor_no] < last[sensor_no] ? second_to_last[sensor_no] : last[sensor_no];
}


/*
 * Läs av tape-sensor med nr sensor_no
 */
void read_tape(uint8_t sensor_no)
{
	clearbit(ADMUX,MUX0);
	clearbit(ADMUX,MUX1);
	clearbit(ADMUX,MUX2);
	clearbit(ADMUX,MUX3);
	clearbit(ADMUX,MUX4);
	setbit(PORTA, PINA1);
	PORTD = (sensor_no<<4)+sensor_no;

	read_adc_tape();
}

void read_one_tape()
{
	clearbit(ADMUX,MUX0);
	clearbit(ADMUX,MUX1);
	clearbit(ADMUX,MUX2);
	clearbit(ADMUX,MUX3);
	clearbit(ADMUX,MUX4);
	setbit(PORTA, PINA1);
	PORTD = 0x22; //Sensor nummer 2

	read_adc_one_tape();
}

/*
* Läs adc och lägg resultatet till den array som är ämnad att sändas till master
*/
void read_adc_tape()
{
	setbit(ADCSRA,ADSC); //start_reading
	//while(!adc_interrupt); //Wait for interrupt to occur
	while(bitclear(ADCSRA, ADIF));
	tape_sensor_data[data_index++] = ADCH;
	//adc_interrupt = 0;
}

void read_adc_one_tape()
{
	setbit(ADCSRA,ADSC); //start_reading
	while(bitclear(ADCSRA, ADIF));
	tape_sensor = ADCH;
}

void read_adc_ir()
{
	setbit(ADCSRA,ADSC); //start_reading
	//while(!adc_interrupt); //Wait for interupt to occur
	while(bitclear(ADCSRA, ADIF));
	ir_sensor_data[data_index++] = ADCH;
	//adc_interrupt = 0;
}


void decode_tape()
{
	static uint8_t tape_count=0;
	static uint8_t no_tape_count = 0;
	static uint8_t is_over_tape = 0;
	static uint8_t is_in_tape_segment = 0;
	static uint8_t first_tape_count = 0;
	static uint8_t second_tape_count = 0;
	static char first_tape;
	static char second_tape;
	
	if (tape_sensor > REFLEX_SENSITIVITY) //Tejpbitsstart hittad
	{
		//cli(); //Stänger av globala avbrott
		is_over_tape = 1;
		no_tape_count = 0;
		tape_count++;
	}
	
	else if (is_over_tape && tape_sensor < REFLEX_SENSITIVITY) //Tejpbit avslutad
	{
		is_over_tape = 0;
		
		if(is_in_tape_segment) //Andra tejpbiten avslutad
		{
			is_in_tape_segment = 0;
			//second_tape = tape_count < 5 ? 's': 'l';
			second_tape_count = tape_count;
			uint8_t tmp[5];
// 			tmp[0] = SENSOR_TAPE_DEBUG;
// 			tmp[1] = tape_count;
// 			send_to_master(2,tmp);
			decode_tape_segment(first_tape_count, second_tape_count); //Kör tejpsegmentsavkodning
		}
		else if(tape_count != 1) // Första tejpbit avslutad		OBS: Fulhack som löste en sensorbugg.
		{
			is_in_tape_segment = 1;
			//first_tape = tape_count < 5 ? 's': 'l';
			first_tape_count = tape_count;
// 			uint8_t tmp[5];
// 			tmp[0] = SENSOR_TAPE_DEBUG;
// 			tmp[1] = tape_count;		
//			send_to_master(2,tmp);
		}
		tape_count=0;
	}
	
	else if(tape_sensor < REFLEX_SENSITIVITY) //Utanför tejp
	{
		no_tape_count++;
		//checka om den bara sett en tejpbit, alltså är den vid mål. Kontrollerar aldrig om det är en kort eller lång tejpbit!
		if(no_tape_count > 2 * first_tape_count && is_in_tape_segment)
		{
			is_in_tape_segment = 0;
			follow_end_tape = 1;
			//decode_tape_segment(first_tape_count, 0); // Skickas vid början av linjeföljningen!
			
		}
		//is_in_tape_segment = no_tape_count++ > 7 ? 0 : is_in_tape_segment; //vilken jävla oneliner!
		
		
		//overflowskydd
		if(no_tape_count == 255)
		no_tape_count = 255;
		
	}
	
}


void decode_tape_segment(uint16_t first, uint8_t second)
{
	_delay_ms(250);
	if (second == 0)         //Bara en tejp, vi är vid mål!
	{
		tape_type = 'g';
		send_decoded_tape();
		return;	
// 		first = first*10;
// 		tape_ratio = first/second;
	}
	
	if(first <= 100 && second > 100) //(first == 's' && second == 'l')
	{
		//turn left
		tape_type = 'l';
		send_decoded_tape();
	}
	else if (first <= 100 && second <= 100) //(first == 's' && second == 's')
	{
		//keep going
		tape_type = 'f';
		send_decoded_tape();
	}
	else if (first > 100 && second <= 100) //(first == 'l' && second == 's')
	{
		//turn right
		tape_type = 'r';
		send_decoded_tape();
	}
}

void regulate_end_tape()
{
	int8_t pos_index; //-5 för längst till vänster, 5 för höger, 0 i mitten!
	uint8_t i, n_of_reflexes_on = 0;
	int8_t res=0;
	int8_t old_pos=0, pos=0;
	uint8_t reflex[9];
	
	for (i = 0; i < 9; i++)
	{
		pos_index = i-4;
		if(tape_sensor_data[i] > REFLEX_SENSITIVITY)
		{
			reflex[i] = 1;
			res += pos_index;
			n_of_reflexes_on += 1;
		}
		else
		reflex[i] = 0;
	}
	
	//div med 0
	if(n_of_reflexes_on != 0)
	{
		pos = res*2/n_of_reflexes_on;        //ojojoj
		tape_position[0] = SENSOR_FOLLOW_TAPE;
		tape_position[1] = pos;
		//Skicka positionen till master
		send_to_master(2, tape_position);
	}
	//utanför tejp, stopp!
	else
	{
		//Skickar att det är slut på linjeföljning till mastern
		tape_position[0] = SENSOR_FOLLOW_TAPE_END;
		tape_position[1] = 'e';
		send_to_master(2, tape_position);
		follow_end_tape = 0;
	}
}


/*=======================================TIMERS==================================*/

/*
 * Initiera timer kopplad till sensoravälsning
 */
void init_sensor_timer()
{
	setbit(TCCR1A, WGM11);
	setbit(TCCR1B, WGM12);
	setbit(TCCR1B, WGM13);
	//set prescaler på fck/256
	setbit(TCCR1B, CS12);
	
	//aktivera interrupt på overflow
	setbit(TIMSK, TOIE1);
	
	//25 hertz, ges av 8000000/(256*1250) = 25 Hz
	/*ICR1 = 1250;*/
	
	
	//50 Hertz, ges av 8000000/(256*625) = 50 Hz
	ICR1 = 625;
	
}

//Sensor timer! (25 Hz)
// ISR(TIMER1_OVF_vect)
// {
// 	read_all_sensors();
// }

ISR(TIMER1_OVF_vect)
{
	read_and_send_ir_to_master = 1;
	to_read_gyro = 1;
	//tape_interupt_flag = 1;
}

//Gyro timer! ( Hz)
ISR(TIMER2_OVF_vect)
{        
	to_read_gyro = 1;
}

/*
 * Avaktivera timern kopplad till gyrot
 */
void disable_gyro_timer()
{
	clearbit(TIMSK, TOIE2);
}

/*
 * Initeietra timer kopplad till gyrot (TIMER2)
 */
void init_gyro_timer()
{
	// WTF? Waveform?
//	setbit(TCCR2, WGM20);
	//sätt CTC mode med OCR2 som topp!
	setbit(TCCR2, WGM21);
	
	// Initvärde
	TCNT2 = 0x00;
	OCR2 = 0xff;
	
	//set prescaler på fck/1024
	setbit(TCCR2, CS20);
	setbit(TCCR2, CS21);
	setbit(TCCR2, CS22);
	

	//aktivera interrupt på overflow
	setbit(TIMSK, TOIE2);

}

/*=====================================SPI========================================*/
/*
 *Initiera spi-bussen
 */
void init_spi()
{
	setbit(SPCR, SPE);		//Enables spi
//	setbit(SPCR, SPIE);
	clearbit(DDRB, PINB4);	// SS är input
	clearbit(DDRB, PINB5);	// MOSI är input
	setbit(DDRB, PINB6);	// MISO är output
	clearbit(DDRB, PINB7);	//CLK är input
	setbit(DDRA, PINA7);	// Avbrottsförfrågan är output
	setbit(PORTA, PINA7);	// 1 = normal, 0 = avbrottsförfrågan
	sei();
}

ISR(SPI_STC_vect)
{
// 	uint8_t byte;
// 	byte = SPDR;
// 	if(byte) {
// 		has_data_from_spi = 1;
// 		spi_data_to_master = byte;
// 	}
	spi_transfer_complete = 1;
}

/*
 * Skickar en sträng till kontrollern som dumpar denna på displayen.
 */
void send_string_to_master(char *str)
{
	uint8_t len = strlen(str) + 1;	/* Längden av strängen + kommandot */
	char msg[34];
	strncpy(msg + 1, str, 33);
	msg[0] = SENSOR_DEBUG;
	msg[33] = 0;
	send_to_master(34, msg);
}

/*
 *Skicka en byte till master
 *OBS!! om du vill sända en byte använd inte denna! Använd send_to_master(1,byte)
 */
void send_to_master_real(uint8_t byte)
{
	volatile uint8_t a;
	if(SPSR & (1 << SPIF)) read_byte(SPDR);
	// Skapa master-interrupt
	spi_transfer_complete = 0;
	SPDR = byte;
	PORTA ^= (1 << PORTA7);
	while(!(SPSR & (1 << SPIF)));
	a = SPDR;
}

void send_stop_turn_message()
{
	send_to_master(1,GYRO);	//Kanske behöver lägga GYRO i en array ?
}




/*
 * Denna funktion tar argument enligt följande:
 * len		antalet byte att skicka
 * *data	en pekare till en uint8_t-array av den data som ska skickas.
 *			Första byten är ett kommando, resten är argument.
 *			Kommandon finns definierade i sensormodul.h
 *			Se exempel i main()
 *
 * Protokollet som faktiskt används för att skicka data till styrenheten 
 * över bussen är enligt följande:
 * Byte:
 *	1				längd av datapaket (max 255)
 *  2				Kommando
 *  3 -	(len-2)		eventuella argument
 */
void send_to_master(uint8_t len, uint8_t *data)
{
	
	send_to_master_real(len);
	int i = 0;
	uint8_t sum = 0;
	while(i < len)
	{
		/* FIXME!!!
		   Jag gillar verkligen inte den här! Kan man inte kolla
		   att en byte har skickats innan man skickar nästa istället
		   för att ha en ful delay? Eller vad är det som gör att det inte
		   funkar utan delay?
		 */
		//_delay_ms(1);
		sum += data[i];
		send_to_master_real(data[i++]);
	}
	send_to_master_real(sum);
}
