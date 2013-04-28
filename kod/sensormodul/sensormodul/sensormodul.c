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
volatile uint8_t test_data[18];9
volatile uint8_t data_index=1;
volatile uint8_t adc_interrupt = 0;
volatile uint8_t has_data_from_spi = 0;


int main(void)
{
	setbit(DDRA, PINA7); //Sätter avbrottpinne mot styr som output
	clearbit(DDRA,PINA0); //Sätter IR/Reflexdata som input
	setbit(DDRA,PINA1); //Sätter reflex-enable som output
	clearbit(DDRA,PINA2); // Gyro som input
	
	DDRD = 0xFF; //A,B,C,D till muxarna
	setbit(DDRC, PINC0);
	init_spi();
	init_adc();
	init_gyro();
	init_sensor_timer();
	

//	_delay_ms(500);	// Ja, vänta! Annars hamnar SPI-protokollet i osynk!
	
    while(1)
    {
		if(has_data_from_spi)
		{
			switch(spi_data_to_master)
			{
				case TURN_RIGHT:
					begin_turning(90);
					break;
				case TURN_LEFT:
					begin_turning(-90);
					break;
				default:
					break; 
			}
			has_data_from_spi = 0;
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

uint8_t gyro_init_value;						//Gyrots initialvärde
uint16_t full_turn, gyro_int;


// Börja snurra. positiv degrees = medurs. Har slutat då is_turning blir 0.
void begin_turning(int16_t degrees)
//>>>>>>> f840052e7a203feb98207f4cf649ee4656753730 //wtf??
{
	full_turn = 55*abs(degrees);	//Sväng x antal grader
	gyro_int = 0;
	init_gyro_timer();

}

/*
 * Initerar gyrot genom att utföra ett fåtal avläsningar,
 * kastar dessa som skräpvärden och sedan väljer ett som initialvärde till gyrot
 */
void init_gyro()
{
	static uint8_t gyro_reads_number = 0;
	
	while(gyro_reads_number<3)
	{
		read_gyro();
		gyro_reads_number++;
	}
	gyro_init_value = read_gyro();
}


//Anropas i timer under pågående sväng
void timed_gyro_turn()
{
	gyro_int += 3*abs(gyro_init_value - (int)read_gyro());			//Maxhastighet 300grader/s,
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
	read_adc();
	
	/*MJUKVARUFILTER*/
	second_to_last[sensor_no] = last[sensor_no];
	last[sensor_no] = test_data[index];
	
	test_data[index] = second_to_last[sensor_no] < last[sensor_no] ? second_to_last[sensor_no] : last[sensor_no];
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

	read_adc();
}

/*
 * Läs adc och lägg resultatet till den array som är ämnad att sändas till master
 */
void read_adc()
{
	setbit(ADCSRA,ADSC); //start_reading
	//while(!adc_interrupt); //Wait for interupt to occur
	while(bitclear(ADCSRA, ADIF));
	test_data[data_index++] = ADCH;
	//adc_interrupt = 0;
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
	//ICR1 = 1250;
	
	//62,5 Hz
	ICR1 = 150;
	
}

//Sensor timer! (25 Hz)
ISR(TIMER1_OVF_vect)
{
	read_all_sensors();
}

//Gyro timer! (XX Hz)
ISR(TIMER2_OVF_vect)
{
	timed_gyro_turn();
}

/*
 * Avaktivera timern kopplad till gyrot
 */
void disable_gyro_timer()
{
	clearbit(TIMSK,TOIE2);
}

// KOLLA PÅ DETTA, SNÄLLA LILLA HK!!!

/*
 * Initeietra timer kopplad till gyrot
 */

// HÅÅÅÅÅÅÅÅÅKÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅÅ!!!!!!!!!!
void init_gyro_timer()
{
	setbit(TCCR2, WGM20);
	setbit(TCCR2, WGM21);
	
	//set prescaler på fck/256
	setbit(TCCR2, CS22);
	
	//aktivera interrupt på overflow
	setbit(TIMSK, TOIE2);
	
	//TEST
	//OCR2A = 150;
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

void SPI_read_byte()
{
	spi_data_from_master = SPDR;
}

ISR(SPI_STC_vect)
{
	SPI_read_byte();
	has_data_from_spi = 1;
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
	SPDR = byte;
	// Skapa master-interrupt
	PORTA ^= (1 << PORTA7);
	while(!(SPSR & (1 << SPIF)));
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
	while(i < len)
	{
		/* FIXME!!!
		   Jag gillar verkligen inte den här! Kan man inte kolla
		   att en byte har skickats innan man skickar nästa istället
		   för att ha en ful delay? Eller vad är det som gör att det inte
		   funkar utan delay?
		 */
		//_delay_ms(1);
		send_to_master_real(data[i++]);
	}
}
