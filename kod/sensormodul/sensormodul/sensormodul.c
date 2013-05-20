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
#include <stdarg.h>

uint8_t spi_data_from_master;
uint8_t spi_data_to_master;
uint8_t tape_sensor;
uint8_t read_mode; // Om read_mode == 1, integrera gyro, annars läs IR och tejp.
char tape_type;


void debug(char *fmt, ...)
{
	va_list args;
	char tmp[256];
	unsigned i;
	va_start(args, fmt);
	vsnprintf(tmp + 1, 255, fmt, args);
	va_end(args);

	tmp[0] = SENSOR_DEBUG;
	send_to_master(strlen(tmp) + 2, tmp);
}

//Gyrots initialvärde
int16_t gyro_init_value;
int16_t full_turn, gyro_int;

//Array med data från alla nio reflexsensorer.
volatile uint8_t tape_sensor_data[9];
//Array med data från alla fem IR-sensorer, med en extra plats för
// en flagga som säger vilken typ av data det är.
volatile uint8_t ir_sensor_data[5+1];
//Array med data som säger vilken typ av tejp som hittats,
//med en extra plats för en flagga som säger vilken typ av data det är.
volatile uint8_t decoded_tape_data[1+1];
//Array med data som säger vilken position roboten befinner sig på i
//linjeföljningen, med en extra plats för en flagga som säger
//vilken typ av data det är.
volatile uint8_t tape_position[1+1];
volatile uint8_t test_data[16+1];
//Anger vilket index datan börjar på i ir_sensor_data och tape_sensor_data.
volatile uint8_t data_index=1;
//Flagga som anger om adc:n skickat interrupt.
volatile uint8_t adc_interrupt;
//Anger om data finns att hämta på bussen.
volatile uint8_t has_data_from_spi;
volatile uint8_t read_and_send_ir_to_master;
//Flagga som anger om linjeföljningsmodet är aktiverat.
volatile uint8_t follow_end_tape;
//Flagga som anger om det autonoma modet är aktiverat.
volatile uint8_t autonomous;
volatile uint8_t to_read_gyro;
volatile uint8_t tape_interrupt_flag;
volatile uint8_t spi_transfer_complete;
//uint8_t no_tape_counter;

/*
Sparar ner data från bussen.
*/
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
	init_sensor_timer();
	
	setbit(DDRB, PORTB3);

	while(1)
	{
		if(read_mode) {
			if(read_and_send_ir_to_master)
			{
				gyro_int += abs(gyro_init_value - read_gyro());			//Maxhastighet 300grader/s,
				read_and_send_ir_to_master = 0;
				uint8_t values[3];
				values[0] = SENSOR_GYRO_INTEGRAL;
				values[1] = (gyro_int) >> 8;
				values[2] = (gyro_int);
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
				data_index = 1;
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
			else if(spi_data_from_master == SENSOR_DELAY) {
				_delay_ms(1000);
			}

			has_data_from_spi = 0;
		}
		
		if (!follow_end_tape)
		{
			read_one_tape(); //AD-omvandlar andra tejpsensorn
			if(tape_sensor)
			{
				decode_tape();
			}
			tape_interrupt_flag = 0;
		}
	}
}

/* =======================================GYRO=======================================*/

/*
* Initierar gyrot genom att utföra ett fåtal avläsningar,
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
	uint8_t i;
	gyro_init_value = 0;
	for(i = 0; i < 1; ++i) gyro_init_value += read_gyro();
	gyro_init_value += 1;
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
	
	//SKalning av klockfrekvens
	setbit(ADCSRA,ADPS2);
	setbit(ADCSRA,ADPS1);
	setbit(ADCSRA,ADPS0);
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


/*
Skickar (till styrmodulen) vilken typ av tejp som hittats.
*/
void send_decoded_tape()
{
	data_index = 1; //Borde inte behövas, dubbelkolla det!
	decoded_tape_data[0] = SENSOR_TAPE;
	decoded_tape_data[1] = tape_type;
	send_to_master(2, decoded_tape_data);
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
	setbit(ADCSRA,ADSC); //start_reading
	while(bitclear(ADCSRA, ADIF));
	return ADCH;
}

/*
* Läs av ir-sensor med nr sensor_no.
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
* Läs av tape-sensor med nr sensor_no.
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

/*
Läs enbart av en reflexsensor,
används för alla tejpbitar utom linjeföljningstejpen,
då nio av reflexsensorerna används.
*/
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
	while(bitclear(ADCSRA, ADIF));
	tape_sensor_data[data_index++] = ADCH;
}

/*
Läs AD-omvandlaren för enbart en tejpsensor.
*/
void read_adc_one_tape()
{
	setbit(ADCSRA,ADSC); //start_reading
	while(bitclear(ADCSRA, ADIF));
	tape_sensor = ADCH;
}

/*
Läser AD-omvandlaren för IR-sensorerna.
*/
void read_adc_ir()
{
	setbit(ADCSRA,ADSC); //start_reading
	while(bitclear(ADCSRA, ADIF));
	ir_sensor_data[data_index++] = ADCH;
}


void decode_tape()
{
	static uint16_t tape_count = 0;
	static uint16_t no_tape_count = 0;
	static uint8_t is_over_tape = 0;
	static uint8_t is_in_tape_segment = 0;
	static uint16_t first_tape_count = 0;
	static uint16_t second_tape_count = 0;
	static char first_tape;
	static char second_tape;
	
	if (tape_sensor > REFLEX_SENSITIVITY) //Tejpbitsstart hittad
	{
		is_over_tape = 1;
		no_tape_count = 0;
		tape_count++;
	//	debug("tejpr %d", tape_count);
		if(tape_count > 500)
		{
			//debug("lang maltejp");
			is_in_tape_segment = 0;
			follow_end_tape = 1;
			no_tape_count = 0;
			decode_tape_segment(first_tape_count, 0);
			tape_count = 0;
		}

	}
	
	else if (is_over_tape && tape_sensor < REFLEX_SENSITIVITY) //Tejpbit avslutad
	{
		static uint8_t no_tape_count_2;
		
		if(++no_tape_count_2 > 10)
		{
			no_tape_count_2 = 0;

			is_over_tape = 0;
			if(is_in_tape_segment && tape_count !=1) //Andra tejpbiten avslutad
			{
				is_in_tape_segment = 0;
				second_tape_count = tape_count;
				uint8_t tmp[5];
				decode_tape_segment(first_tape_count, second_tape_count); //Kör tejpsegmentsavkodning
			tape_count=0;
			}
			else if(tape_count != 1) // Första tejpbit avslutad		OBS: Fulhack som löste en sensorbugg.
			{
				is_in_tape_segment = 1;
				first_tape_count = tape_count;
			tape_count=0;
			}
		}			
	}
	
	else if(tape_sensor < REFLEX_SENSITIVITY) //Utanför tejp
	{
		no_tape_count++;
		//checka om den bara sett en tejpbit, alltså är den vid mål. Kontrollerar aldrig om det är en kort eller lång tejpbit!
		if(no_tape_count > 300 && is_in_tape_segment)
		{
			is_in_tape_segment = 0;
			follow_end_tape = 1;
			no_tape_count = 0;
			decode_tape_segment(first_tape_count, 0);
		}
		
		
		//overflowskydd
		if(no_tape_count == 65000)
		no_tape_count = 65000;
		
	}
	
}


void decode_tape_segment(uint16_t first, uint16_t second)
{
	uint16_t tape_border = 180;
	if (second == 0) //Bara en tejp, vi är vid mål!
	{
		tape_type = 'g';
		send_decoded_tape();
		return;
	}
	
	if(first <= tape_border && second > tape_border) //(first == 's' && second == 'l')
	{
		//turn left
		tape_type = 'l';
		send_decoded_tape();
	}
	else if (first <= tape_border && second <= tape_border) //(first == 's' && second == 's')
	{
		//keep going
		tape_type = 'f';
		send_decoded_tape();
	}
	else if (first > tape_border && second <= tape_border) //(first == 'l' && second == 's')
	{
		//turn right
		tape_type = 'r';
		send_decoded_tape();
	}
}

/*
Räknar ut positionen som roboten befinner sig på i linjeföljningen,
finns utförligt beskrivet i tekniska dokumentationen.
*/
void regulate_end_tape()
{
	int8_t pos_index; //-4 för längst till vänster, 4 för höger, 0 i mitten!
	uint8_t i, n_of_reflexes_on = 0;
	int8_t res=0;
	int8_t old_pos=0, pos=0;
	uint8_t reflex[9];
	uint8_t left_ir_sensor_value;
	uint8_t right_ir_sensor_value;
	static uint8_t vertical_end_tape_found = 0;
	
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
	
		
	if (n_of_reflexes_on < 4 &&  n_of_reflexes_on > 0 ) // den är på linjen, sätt flagga att den följt linje
	{
		vertical_end_tape_found = 1;
	}
	
static uint8_t floor_counter;
	// Normal linjeföljning
	if(n_of_reflexes_on != 0)
	{
		pos = res*2/n_of_reflexes_on;        //ojojoj
		tape_position[0] = SENSOR_FOLLOW_TAPE;
		tape_position[1] = pos;
		//Skicka positionen till master
		send_to_master(2, tape_position);
	}

	//utanför tejp
	else if ((n_of_reflexes_on == 0 /* || n_of_reflexes_on > 4*/) && vertical_end_tape_found) // Utanför tejp men den har kört linjeföljning. Henning är i mål!
	{
		//if(++floor_counter > 20)
		//{
			floor_counter = 0;
//			debug("hittat sluttejp");
			//Skickar att det är slut på linjeföljning till mastern
			tape_position[0] = SENSOR_FOLLOW_TAPE_END;
			tape_position[1] = 'e';
			send_to_master(2, tape_position);

			vertical_end_tape_found = 0;
			follow_end_tape = 0;
		//}			
	}
	else if (n_of_reflexes_on == 0 && !vertical_end_tape_found)// Utanför tejp men den har inte kört linjeföljning, ska regulera in mot tejpen.
	{
		read_ir(1);
		read_ir(2);
		left_ir_sensor_value = ir_sensor_data[1]; //Läggs in med index sensorindex + 1 från read_adc
		right_ir_sensor_value = ir_sensor_data[2];
		if (right_ir_sensor_value > left_ir_sensor_value) // Står till vänster om tejpen, sväng höger
		{
			pos = 8;
			tape_position[0] = SENSOR_FOLLOW_TAPE;
			tape_position[1] = pos;
			//Skicka positionen till master
			send_to_master(2, tape_position);
		}
		else // Står till höger om tejpen, sväng vänster
		{
			pos = -8;
			tape_position[0] = SENSOR_FOLLOW_TAPE;
			tape_position[1] = pos;
			//Skicka positionen till master
			send_to_master(2, tape_position);
		}
	}
}


/*=======================================TIMERS==================================*/

/*
* Initiera timer kopplad till sensoravläsning
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

/*
Avbrottsrutin för timern som skickar IR-data till styrenheten,
25-50 Hz ungefär.
*/
ISR(TIMER1_OVF_vect)
{
	read_and_send_ir_to_master = 1;
	to_read_gyro = 1;
	//tape_interupt_flag = 1;
}

//Gyro timer! (~30 Hz)
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
	clearbit(DDRB, PINB4);	// SS är input
	clearbit(DDRB, PINB5);	// MOSI är input
	setbit(DDRB, PINB6);	// MISO är output
	clearbit(DDRB, PINB7);	//CLK är input
	setbit(DDRA, PINA7);	// Avbrottsförfrågan är output
	setbit(PORTA, PINA7);	// 1 = normal, 0 = avbrottsförfrågan
	sei();
}

/*
Avbrottsrutin för SPI-bussen.
*/
ISR(SPI_STC_vect)
{
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
		sum += data[i];
		send_to_master_real(data[i++]);
	}
	send_to_master_real(sum);
}
