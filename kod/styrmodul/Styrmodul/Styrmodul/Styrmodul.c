/*
 * Styrmodul.c
 *
 * Created: 4/2/2013 2:20:45 PM
 *  Author: klawi021 ET AL!
 */

#define F_CPU 8000000UL
#include "Styrmodul.h"
#include "pid.h"
#include "../../../sensormodul/sensormodul/sensormodul.h"
#define SENSOR_BUFFER_SIZE 256
//#define INTERPOLATION_POINTS 12
uint8_t test;

#define BUF_SZ 256

// Buffrar för interrupt-koden

volatile uint8_t spi_data_from_comm[BUF_SZ];
uint8_t spi_comm_read;
volatile uint16_t spi_comm_write;

volatile uint8_t spi_data_from_sensor[BUF_SZ];
uint8_t spi_sensor_read;
volatile uint16_t spi_sensor_write;

#define SPEED 255
uint8_t ninety_timer, turn, pid_timer;
uint8_t left = 1;

uint8_t gyro_init_value;						//Gyrots initialvärde
uint8_t regulator_enable = 0;					//Flagga för att indikera 40 ms åt regulatorn.

// Denna ancänds bara av inte-interrupt-koden
uint8_t sensor_buffer[SENSOR_BUFFER_SIZE];		// Buffer som håller data från sensorenheten
uint8_t sensor_buffer_pointer;			// Pekare till aktuell position i bufferten
uint8_t sensor_start;					// Flagga som avgör huruvida vi är i början av meddelande
uint8_t sensor_packet_length;					// Anger aktuell längd av meddelandet
// Innehåller de spänningsvärden som ses i grafen på https://docs.isy.liu.se/twiki/pub/VanHeden/DataSheets/gp2y0a21.pdf, sid. 4.
uint8_t small_ir_voltage_array[122] = {140, 139, 138, 137, 136, 135, 134, 133, 132, 131, 130, 129, 128, 127, 
									   126, 125, 124, 123, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113, 
									   112, 111, 110, 109, 108, 107, 106, 105, 104, 103, 102, 101, 100, 99,
									   98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 
									   81, 80, 79, 78, 77, 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66, 65,
									   64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49, 48, 
									   47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33, 32, 31, 
									   30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19};	
uint8_t small_ir_centimeter_array[122] = {8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 9, 10, 10, 10, 10, 10,
										  10, 10, 10, 10, 11, 11, 11, 11, 11, 11, 11, 12, 12, 12, 12, 12, 12, 13, 
										  13, 13, 13, 13, 13, 13, 14, 14, 14, 14, 14, 14, 14, 15, 15, 15, 15, 15,
										  16, 16, 16, 16, 17, 17, 17, 18, 18, 18, 19, 19, 19, 19, 20, 20, 20, 21,
										  21, 22, 22, 23, 23, 23, 24, 24, 25, 25, 26, 26, 27, 28, 28, 29, 29, 30, 
										  31, 32, 33, 34, 36, 37, 38, 39, 40, 41, 43, 44, 46, 47, 49, 50, 53, 55,
										  58, 60, 63, 65, 68, 70, 75, 80, 85, 90};	
uint8_t big_ir_voltage_array[117] =  {138, 137, 136, 135, 134, 133, 132, 131, 130, 129, 128, 127, 126, 125, 124, 
									  123, 122, 121, 120, 119, 118, 117, 116, 115, 114, 113, 112, 111, 110, 109, 
									  108, 107, 106, 105, 104, 103, 102, 101, 100, 99, 98, 97, 96, 95, 94, 93, 
									  92, 91, 90, 89, 88, 87, 86, 85, 84, 83, 82, 81, 80, 79, 78, 77, 76, 75, 
									  74, 73, 72, 71, 70, 69, 68, 67, 66, 65, 64, 63, 62, 61, 60, 59, 58, 57, 
									  56, 55, 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 
									  38, 37, 36, 35, 34, 33, 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22};	
uint8_t big_ir_centimeter_array[117] = {16, 16, 17, 17, 18, 18, 19, 19, 20, 20, 20, 21, 21, 21, 22, 22, 23, 23, 23, 24, 24,
										24, 25, 25, 25, 26, 26, 27, 27, 27, 28, 28, 28, 29, 29, 30, 30, 30, 31, 31, 32, 32, 
										33, 33, 33, 34, 34, 35, 35, 35, 36, 36, 37, 37, 38, 38, 38, 39, 39, 40, 40, 41, 41, 
										42, 42, 43, 43, 44, 44, 45, 46, 47, 48, 49, 50, 51, 53, 54, 55, 56, 58, 59, 60, 60, 
										61, 62, 63, 63, 64, 64, 65, 67, 68, 70, 73, 75, 78, 80, 82, 84, 86, 88, 90, 93, 97, 
										100, 105, 110, 113, 117, 120, 125, 130, 135, 140, 145, 150};
// Innehåller de centimetervärden som ses i grafen på https://docs.isy.liu.se/twiki/pub/VanHeden/DataSheets/gp2y0a21.pdf, sid. 4.


uint8_t follow_end_tape = 0;

int main(void)
{
	//don't turn!
	turn = 0;
	//OSCCAL = 0x70;
	//display ska ut
//	DDRA = 0xFF;

	init_display();
	clear_screen();
	update();
	spi_init();
	pwm_init();
	sei();		//aktivera global interrupts
	
	clear_pid();
	init_pid(40, 255, -255);
	update_k_values(1, 1, 1);
	
	while(1)
	{
		if (follow_end_tape)
		{
			//regulate_end_tape(spi_data_from_sensor[]);
		}
		
		if(spi_comm_write != spi_comm_read)
		{
			decode_comm(spi_data_from_comm[spi_comm_read]);
			++spi_comm_read;
			spi_comm_read %= BUF_SZ;
		}
	
		if(spi_sensor_write != spi_sensor_read)
		{
			uint8_t turn_amount;
			
			decode_sensor(spi_data_from_sensor[spi_sensor_read]);
			++spi_sensor_read;
			spi_sensor_read %= BUF_SZ;
		}
		
		if (regulator_enable && regulator_flag)
		{
			static int16_t temp_input = 0,temp_output = 0;
			static int16_t old_temp_output;
			temp_input = (sensor_buffer[IR_RIGHT_BACK] + sensor_buffer[IR_RIGHT_FRONT] - sensor_buffer[IR_LEFT_BACK] - sensor_buffer[IR_LEFT_FRONT])/2;
			temp_output = regulator(temp_input); //borde skrivas om så den ger ut ett åttabitarsvärde? Ja
			
			if(temp_output == 0)
			{
				LEFT_AMOUNT = SPEED;
				RIGHT_AMOUNT = SPEED;
			}
			
			if(temp_output > 0)
			{
				RIGHT_AMOUNT = RIGHT_AMOUNT - (uint8_t)abs(temp_output);
				LEFT_AMOUNT = SPEED;
			}
				
			if (temp_output < 0)
			{
				LEFT_AMOUNT = LEFT_AMOUNT - (uint8_t)abs(temp_output);
				RIGHT_AMOUNT = SPEED;
			}
			
		regulator_enable = 0;
		
		}	
	}
}

/*
 * Skickar en sträng till fjärrenheten (via komm) som dumpar den på skärmen
 */
void send_string_remote(char *str)
{
	while(*str)
		send_byte_to_comm(*str++);
}	

void pid_timer_init()
{
	//sätt "Fast PWM mode", med OCRA (OCR0A?) som toppvärde!
	setbit(TCCR0A, WGM00);
	setbit(TCCR0A, WGM01);
	setbit(TCCR0B, WGM02);
	
	//sätt klockskalning, fck = f/1024
	setbit(TCCR0B, CS00);
	setbit(TCCR0B, CS02);
	
	//aktivera interrupts, skickas på overflow
	setbit(TIMSK0, TOIE0);
	
	//8 bit-register
	//frekvensen blir då 8000000/(1024*255) = 30.63 Hz
	//vilket är mingränsen!
	OCR0A = 255;
}

void pwm_init()
{	
	//_delay_ms(1000);
	
	///////////////////////////////////spi_send_byte(0xAA);
	
	//DDRB = 0xFF;
	//DDRC = 0xFF;
	//DDRD = 0xFF;
	
	//sätt riktning på displaystyrpinnar!
	setbit(DDRC, PC6);
	setbit(DDRC, PC7);
	
	
	//sätt riktning på motorer + gripklo
	setbit(DDRB, PB0);
	setbit(DDRB, PB1);
	setbit(DDRD, PD7);
	setbit(DDRD, PD6);
	setbit(DDRD, PD5);
	
	
	//pwm-styrning för gripklon, pin OC1A, register OCR1A
	TCCR1A = 0;
	setbit(TCCR1A, COM1A1);
	setbit(TCCR1A, WGM11);
	
	TCCR1B = 0;
	setbit(TCCR1B, WGM11);
	setbit(TCCR1B, WGM12);
	setbit(TCCR1B, WGM13);
	
	//sätt klockan på fclk/64!
	setbit(TCCR1B, CS10);
	setbit(TCCR1B, CS11);
	//setbit(TCCR1B, CS12);
	
	
	//TCCR1A = (1 << COM1A1) | (1 << WGM11);
	//TCCR1B = (1 << WGM11) | (1 << WGM13) | (1 << WGM12) | (1 << CS12) | (1 << CS10);
	TIMSK1 = (1 << OCIE1A);  // Enable Interrupt TimerCounter1 Compare Match A (TIMER1_COMPA_vect)
	//ICR1 = 390;
	//ICR1 = 625;
	//ICR1 = 313;
	ICR1 = 625*4;
	//sätt OCR1A också!
	//CLAW_AMOUNT = 10*4;
	//_delay_ms(1000);
	//CLAW_AMOUNT = 78*4;
	
	//pwm-styrning för motorerna, pinne OC2A, register OCR2A för vänster, pinne OC2B, register OCR2B för höger.
	//PB1 DIR höger, PB0 vänster
	TCCR2A=0;
	setbit(TCCR2A, COM2A1);
	setbit(TCCR2A, COM2B1);
	setbit(TCCR2A, WGM20);
	setbit(TCCR2A, WGM21);
	OCR2A = 0; 
	OCR2B = 0;
	
	//TCCR2A = (1 << COM2A1) | (1 << COM2B1) | (1 << WGM21) | (1 << WGM20);
	
	//sätter på pwm!
	TCCR2B=0;
	//fclk/64, ger 488 Hz ut på PWM
	setbit(TCCR2B, CS20);
	setbit(TCCR2B, CS21);
	
	
	
	//TCCR2B = (1 << CS20);
	////////////////////////////////////////////TIMSK2 = (1 << OCIE2A);
	//fullt ös på OCR=0xff, inget på 0x00
	
	
}

void spi_init()
{
	clearbit(DDRD, PIND3);		// Avbrott från kommunikationsenheten är input
	clearbit(DDRD, PIND2);		// Samma för sensorenheten
	setbit(PORTD, PORTD3);		// Slå på internt pull up-motstånd (1 är neutralt läge, 0 är avbrottsförfrågan!)
	setbit(PORTD, PORTD2);		// Samma för sensor
	
	setbit(DDRB, PINB3);	// Slave Select  (SS) för kommunikationsenheten är output!
	setbit(DDRB, PINB2);	// Schutzstaffel (SS) för sensor, också output
	setbit(DDRB, PINB5);	//MOSI output!
	clearbit(DDRB, PINB6);	//MISO input!
	setbit(DDRB, PINB7);	//SCK output!
	
	setbit(PORTB, PORTB3);	// Välj ej komm!  
	setbit(PORTB, PORTB2);	// Välj ej sensor!
	
	clearbit(DDRB, PINB4);	//SS master input och hög
	setbit(PORTB, PINB4);
	
	test = SPSR;
	
	
	//Sätt SPCR-registret, inställningar om master/slave, spi enable, data order, klockdelning
	SPCR = 0;
	//setbit(SPCR, SPIE);
	setbit(SPCR, SPE);
	setbit(SPCR, MSTR);
	//setbit(SPCR, SPR0);
	setbit(SPCR, SPI2X);

	//aktivera interrupt på INT0 och INT1
	setbit(EIMSK, INT0);	// Akvitera avbrottsförfrågan från sensorenheten
	setbit(EIMSK, INT1);	// Akvitera avbrottsförfrågan från kommunikationsenheten

	//aktivera interrupt-request på "any change"
	setbit(EICRA, ISC00);
	setbit(EICRA, ISC10);
	
	sensor_buffer_pointer = 0x00;	// Pekare till aktuell position i bufferten
	sensor_start = 1;				// Flagga som avgör huruvida vi är i början av meddelande
	sensor_packet_length = 0x00;			// Anger aktuell längd av meddelandet

}

void send_byte_to_sensor(uint8_t byte)
{
	clearbit(PORTB, PORTB2); //Välj Komm-enheten måste ändras till allmän slav!
	SPDR = byte;
	
	//SPDR = 0xaa;
	/* Wait for transmission complete */
	while(!(SPSR & (1 << SPIF)));
	setbit(PORTB, PORTB2); //Sätt slave till sleepmode
	//test = SPDR;
	
	//PORTD = test;
}

void send_byte_to_comm(uint8_t byte)
{
	clearbit(PORTB, PORTB3); //Välj Komm-enheten måste ändras till allmän slav!
	SPDR = byte;
	
	//SPDR = 0xaa;
	/* Wait for transmission complete */
	while(!(SPSR & (1 << SPIF)));
	setbit(PORTB, PORTB3); //Sätt slave till sleepmode
	//test = SPDR;
	
	//PORTD = test;
}

void drive_forwards(uint8_t amount)
{
	LEFT_AMOUNT = amount;
	RIGHT_AMOUNT = amount;
	
	//sätt ettor (framåt) på DIR-pinnarna
	setbit(PORT_DIR, LEFT_DIR);
	setbit(PORT_DIR, RIGHT_DIR);
}

void drive_backwards(uint8_t amount)
{
	LEFT_AMOUNT = amount;
	RIGHT_AMOUNT = amount;
	
	//sätt nollor (bakåt) på DIR-pinnarna
	clearbit(PORT_DIR, LEFT_DIR);
	clearbit(PORT_DIR, RIGHT_DIR);
}

//sväng vänster!
void turn_left(uint8_t amount)
{
	LEFT_AMOUNT = amount;
}

//sväng höger!
void turn_right(uint8_t amount)
{
	RIGHT_AMOUNT = amount;
}

//stanna allt!
void stop_motors()
{
	//sätter ingen klocka
	//clearbit(TCCR2B, CS20);
	//clearbit(TCCR2B, CS21);
	//clearbit(TCCR2B, CS22);
	//LEFT_PWM = 0;
	//RIGHT_PWM = 0;
	LEFT_AMOUNT = 0;
	RIGHT_AMOUNT = 0;
}

//sväng vänster som en stridsvagn!
void tank_turn_left(uint8_t amount)
{
	LEFT_AMOUNT = amount;
	RIGHT_AMOUNT = amount;
	
	clearbit(PORT_DIR, LEFT_DIR);
	setbit(PORT_DIR, RIGHT_DIR);
}


//sväng höger som en stridsvagn!
void tank_turn_right(uint8_t amount)
{
	LEFT_AMOUNT = amount;
	RIGHT_AMOUNT = amount;
	
	setbit(PORT_DIR, LEFT_DIR);
	clearbit(PORT_DIR, RIGHT_DIR);
}

void claw_out()
{
	CLAW_AMOUNT = 70*4;
}

void claw_in()
{
	CLAW_AMOUNT = 50*4;
}


/*
..................................................|         /  _________________     __              __    __              __.........................
..................................................|        /  |  ______________  |  |  \            /  |  |  \            /  |........................
..................................................|       /   | |              | |  |   \          /   |  |   \          /   |........................
..................................................|      /    | |              | |  |    \        /    |  |    \        /    |.......................
..................................................|     /     | |              | |  |     \      /     |  |     \      /     |........................
..................................................|    /      | |   O     =    | |  |      \    /      |  |      \    /      |.......................
..................................................|   /       | |              | |  |       \  /       |  |       \  /       |.......................
..................................................|  /        | |     /        | |  |        \/        |  |        \/        |........................ 
..................................................|  \        | |    /_        | |  |                  |  |                  |........................
..................................................|   \       | |              | |  |                  |  |                  |........................
..................................................|    \      | |              | |  |                  |  |                  |........................
..................................................|     \     | |              | |  |                  |  |                  |........................
..................................................|      \    | |   \_____/    | |  |                  |  |                  |........................
..................................................|       \   | |              | |  |                  |  |                  |........................
..................................................|        \  | |______________| |  |                  |  |                  |........................
..................................................|         \ |__________________|  |                  |  |                  |........................
*/


// Kommunikationsenheten skickar en avbrottsförfrågan
ISR(INT1_vect)
{
	clearbit(PORTB, PORTB3);	//Väljer komm
	SPDR = 0;		//Lägger in meddelande i SPDR, startar överföringen
	while(!(SPSR & (1 << SPIF)));
	spi_data_from_comm[spi_comm_write] = SPDR;
	spi_comm_write = (spi_comm_write + 1) % BUF_SZ;
	setbit(PORTB, PORTB3);		//Sätter komm till sleepmode
}

// Sensorenheten skickar en avbrottsförfrågan
ISR(INT0_vect)
{
	clearbit(PORTB, PORTB2);	//Väljer sensor
	SPDR = 0;		//Lägger in meddelande i SPDR, startar överföringen
	while(!(SPSR & (1 << SPIF)));
	spi_data_from_sensor[spi_sensor_write] = SPDR;
	spi_sensor_write = (spi_sensor_write + 1) % BUF_SZ;
	setbit(PORTB, PORTB2);		//Sätter sensor till sleepmode
}

//kör i 50 Hz! Ändra ej frekvensen, då denna även
//används till gripklon, som måste köras i 50 Hz!
ISR(TIMER1_COMPA_vect)
{
	ninety_timer++;
	
	//en sekund har gått
	if(ninety_timer == 20)
	{
		turn ^= 0xff;
		//tank_turn_left(60);
		ninety_timer=0;
	}
	
	
}

//overflow på timer0, ställ in frekvens med
//OCR0A, och CSxx-flaggorna i TCCR0B
ISR(TIMER0_OVF_vect)
{
	
	
}


uint8_t display_printf_string[100];
void decode_comm(uint8_t command)
{
	static uint8_t pid = 0;		// Hantera PID-argument
	static uint8_t constant_p;	// Akutellt PID-argument
	static uint8_t constant_i;	// Dito...
	static uint8_t constant_d;	// Dito...

	static uint8_t display_printf_p = 0; // Används vid uppdatering av printf-strängen
	static uint8_t display = 0;	// Dislayen uppdateras!
	static uint8_t drive = 0;	// Förra kommandot väntar på ett hastighetsargument

// 	char temp[32];
// 	sprintf(temp,"%02X ", command);
// 	send_string(temp);
// 	update();
	
	if(pid)
	{
// 		char tmp[30];
// 		sprintf(tmp, "%02X ",  pid);
// 		send_string(tmp);
// 		update();
		if(pid == 3){
			constant_p = command;
		}			
		else if(pid == 2){
			constant_i = command;
		}
		else // pid == 1
		{
			constant_d = command;
			update_k_values(constant_p, constant_i, constant_d);
		}
		--pid;
	} 
	else if(display)
	{
		/* Slutet på printf-strängen? */
		if(command == 0x00)
		{
			clear_screen();
			display_printf_p = 0;
		} else
		{
			display_printf_string[display_printf_p++] = command;
		}			
		display = 0;
		return;
	} else if(drive)
	{
		if(drive == COMM_DRIVE)
		{
			drive = 0;
			drive_forwards(command);
		} else if(drive == COMM_BACK)
		{
			drive = 0;
			drive_backwards(command);
		} else if(drive == COMM_STOP)
		{
			drive = 0;
			disable_pid();
			stop_motors();
		} else if(drive == COMM_LEFT)
		{
			drive = 0;
			tank_turn_left(command);
		} else if(drive == COMM_RIGHT)
		{
			drive = 0;
			tank_turn_right(command);
		} else if(drive == COMM_DRIVE_LEFT)	// Ignorera argumentet tills vidare, vet inte hur vi ska lösa det...
		{
			drive = 0;
			LEFT_AMOUNT = 60;
			RIGHT_AMOUNT = 255;
		
			setbit(PORT_DIR, LEFT_DIR);
			setbit(PORT_DIR, RIGHT_DIR);
		}	else if(drive == COMM_DRIVE_RIGHT)	// Ignorera argumentet även här, tills vidare...
		{
			drive = 0;
			LEFT_AMOUNT = 255;
			RIGHT_AMOUNT = 60;

			setbit(PORT_DIR, LEFT_DIR);
			setbit(PORT_DIR, RIGHT_DIR);
		} else if(drive == COMM_CLAW_OUT)
		{
			drive = 0;
			claw_out();
		} else if(drive == COMM_CLAW_IN)
		{
			drive = 0;
			claw_in();
		}
		return;
	} // if(drive)
	else if(command == COMM_DRIVE || command == COMM_BACK || command == COMM_LEFT || command == COMM_RIGHT ||
	command == COMM_DRIVE_LEFT || command == COMM_DRIVE_RIGHT || command == COMM_CLAW_OUT || command == COMM_CLAW_IN)
	{
		drive = command;
		return;
	}
	else if(command == COMM_STOP)
	{
		disable_pid();
		stop_motors();
	}
	else if(command == COMM_SET_PID)
	{
		pid = 3;
	}
	else if(command == COMM_ENABLE_PID)
	{
		enable_pid();
		setbit(PORT_DIR, LEFT_DIR);		//Kör framåt under regleringen.
		setbit(PORT_DIR, RIGHT_DIR);	
	}
	else if(command == COMM_DISABLE_PID)
	{
		disable_pid();
	}
	else if(command == COMM_CLEAR_DISPLAY)
	{
		clear_screen();
		update();
	}		
	else if(command == COMM_DISPLAY)
	{
		display = 1;
	}
	else if(command == COMM_TOGGLE_SENSORS)
	{
		if(bitset(EIMSK, INT0))
			clearbit(EIMSK, INT0);	// Avaktivera avbrottsförfrågan från sensorenheten
		else
			setbit(EIMSK, INT0);	// Akvitera avbrottsförfrågan från sensorenheten
	}
	else if(command == COMM_TURN_90_DEGREES_LEFT)
	{
		send_string("PERPENDICULAR LEFT");
		update();
	}
	else if(command == COMM_TURN_90_DEGREES_RIGHT)
	{
		send_string("PERPENDICULAR RIGHT");
		update();
	}				
	else	
	{
		char tmp[30];
		sprintf(tmp, "Err%02X ", command);
		send_string(tmp);
		update();
	}
}

// Börja snurra. positiv degrees = medurs. Har slutat då is_turning blir 0.
void begin_turning(int16_t degrees)
{
	if(degrees < 0) 
	{
		tank_turn_left(150);
		send_byte_to_sensor(TURN_LEFT);
	}
	else
	{
		tank_turn_right(150);
		send_byte_to_sensor(TURN_RIGHT);
	}		

}

void decode_sensor(uint8_t data)
{
	
	static uint8_t sensor_transmission_number = 0;

	/* Första byten i ett meddelande är storleken */
	if(sensor_start)
	{
		sensor_packet_length = data;
		sensor_start = 0;
		return;
	}

	/* Annars, lägg in inkommande byten i bufferten */
	sensor_buffer[sensor_buffer_pointer++] = data;

	/* Om det är fler byte kvar att ta emot, vänta på dem! */
	if(sensor_buffer_pointer != sensor_packet_length)
		return;
	/* Aha, vi har tagit emot hela meddelandet! Tolka detta! */
	

/**********************************************************************
 * Här hanteras kommandon från sensorenheten
 **********************************************************************/
	switch(sensor_buffer[0]) {
		case SENSOR_DEBUG:
			sensor_debug_message();
			break;
		case SENSOR_HEX:
			sensor_debug_hex();
			break;
		case GYRO_SENSOR:
			stop_motors();
			break;	
		case SENSOR: {
			//Omvandla sensorvärden från spänningar till centimeter.
			// 	interpret_big_ir(sensor_buffer[IR_FRONT]);
			/*	interpret_big_ir(sensor_buffer[IR_LEFT_FRONT]);*/
			// 	interpret_big_ir(sensor_buffer[IR_RIGHT_FRONT]);
			// 	interpret_small_ir(sensor_buffer[IR_LEFT_BACK]);
			// 	interpret_small_ir(sensor_buffer[IR_RIGHT_BACK]);
			sensor_buffer[IR_FRONT] = interpret_big_ir(sensor_buffer[IR_FRONT]);
			sensor_buffer[IR_LEFT_FRONT] = interpret_big_ir(sensor_buffer[IR_LEFT_FRONT]);
			sensor_buffer[IR_RIGHT_FRONT] = interpret_big_ir(sensor_buffer[IR_RIGHT_FRONT]);
			sensor_buffer[IR_LEFT_BACK] = interpret_small_ir(sensor_buffer[IR_LEFT_BACK]);
			sensor_buffer[IR_RIGHT_BACK] = interpret_small_ir(sensor_buffer[IR_RIGHT_BACK]);
			
// 			//First time? Calibrate gyro
// 			if(sensor_transmission_number<5)
// 			{
// 				gyro_init_value = sensor_buffer[GYRO];
// 				sensor_transmission_number++;
// 			}
// 			
// 			if(is_turning) 
// 			{
// 				gyro_int += 3*abs(gyro_init_value - (int)sensor_buffer[GYRO]); //Maxhastighet 300grader/s,
// 				if(gyro_int >= full_turn)									   //maxvärde-nollnivå ung 100.
// 				{
// 					stop_motors();
// 					is_turning = 0;
// 				}
// 			}
			
			decode_tape_sensor_data();
			analyze_ir_sensors();

			break;
		} 
		default:
				// Unimplemented command
			break;
		}

	
	/* Återställ pekare, för att visa att nästa byte är början av meddelande */
	sensor_buffer_pointer = 0x00;
	sensor_packet_length = 0;
	sensor_start = 1;
	
	regulator_enable = 1;		//Här har det gått ~40 ms dvs starta regleringen.
	
	static uint8_t a=0;
	if((a++ & 0b10000))
	{
		a=0;
		update_display_string();
	}
}


#define BUFFER_SIZE 100
#define MAX_SENSORS 15
void update_display_string()
{
	char tmp[100];
	clear_screen();
	sprintf(tmp, "Left: %02X        Right: %02X ", sensor_buffer[IR_LEFT_FRONT], sensor_buffer[IR_RIGHT_FRONT]);
	sprintf(tmp, "Left:  %03d      Right: %03d ", sensor_buffer[IR_LEFT_FRONT], sensor_buffer[IR_RIGHT_FRONT]);
	send_string(tmp);
	update();
	return;


// 	clear_screen();
// 	send_string(display_printf_string);
// 	update();
// 	return;
    uint8_t *inp = display_printf_string;

    char tmpStr[BUFFER_SIZE];
    char *tmpp = tmpStr;
    while(inp < display_printf_string + BUFFER_SIZE)
    {
	    if(*inp != '%') // Normal-tecken, kopiera och gå vidare!
	    {
		    *tmpp = *inp;
		    if(*inp == '\0')
		    break;
		    inp++;
		    tmpp++;
		    continue;
	    } else
	    {
		    inp++;    // Bortom %-tecknet
		    uint8_t base;   // Nästa är d för decimal, x för hex
		    if(*inp == 'd')
		    base = 10;
		    else if((*inp == 'x') || (*inp == 'X'))
		    base = 16;
		    inp++;
		    uint8_t sensor = *inp; // Nästa är sensor-index
		    inp++;
		    if(sensor > MAX_SENSORS)
		    continue;
		    if(base == 10)
		    {
			    sprintf(tmpp, "%3d", sensor_buffer[sensor]);
			    tmpp++; // Decimal-strängen är tre tecken
			    tmpp++;
			    tmpp++;
		    } else
		    {
			    sprintf(tmpp, "%02X", sensor_buffer[sensor]);
			    tmpp++; // Hex-strängen är två tecken
			    tmpp++;
		    }
		    continue;
	    }
    }
	clear_screen();
	send_string(tmpStr);
	update();

}

void decode_tape_sensor_data()
{
	static uint8_t tape_count=0;
	static uint8_t no_tape_count = 0;
	static uint8_t is_over_tape = 0;
	static uint8_t is_in_tape_segment = 0;
	static char first_tape;
	static char second_tape;
	
	if (sensor_buffer[REFLEX1]>REFLEX_SENSITIVITY) //Tejpbitsstart hittad
	{
		is_over_tape = 1;
		no_tape_count = 0;
		tape_count++;
	}
	
	else if (is_over_tape && sensor_buffer[REFLEX1]<REFLEX_SENSITIVITY) //Tejpbit avslutad
	{
		is_over_tape = 0;
		if(is_in_tape_segment) //Andra tejpbiten avslutad
		{
			is_in_tape_segment = 0;
			second_tape = tape_count < 5 ? 's': 'l';
			decode_tape_segment(first_tape, second_tape); //Kör tejpsegmentsavkodning
		}
		else // Första tejpbit avslutad
		{
			is_in_tape_segment = 1;
			first_tape = tape_count < 5 ? 's': 'l';
		}
		tape_count=0;
	}
	
	else if(sensor_buffer[REFLEX1]<REFLEX_SENSITIVITY) //Utanför tejp
	{
		//checka om den bara sett en tejpbit, alltså är den vid mål
		if(first_tape == 's' && no_tape_count > 7 && is_in_tape_segment)
		{
			decode_tape_segment(first_tape, 0);
		}
		is_in_tape_segment = no_tape_count++ > 7 ? 0 : is_in_tape_segment; //vilken jävla oneliner!
		
		
		//overflowskydd
		if(no_tape_count == 255)
			no_tape_count = 255;
			
	}
	
}

void decode_tape_segment(char first, char second)
{
	_delay_ms(250);
	if(first == 's' && second == 'l')
	{
		//turn left
		stop_motors();
		_delay_ms(250);
		//begin_turning(-90);
		tank_turn_left(SPEED);
	}
	else if (first == 'l' && second == 's')
	{
		//turn right
		stop_motors();
		_delay_ms(2500);
		//begin_turning(90);
		tank_turn_right(SPEED);
	}
	else if (first == 's' && second == 's')
	{
		//keep going
		stop_motors();
		_delay_ms(250);
		drive_forwards(SPEED);
	}
	//första var smal, andra fanns ej, vi är vid mål!
	else if (first == 's' && !second)
	{
		follow_end_tape = 1;
		
		uint8_t i = 0;
		for (i=0;i<10;i++)
		{
			tank_turn_right(SPEED);
			_delay_ms(500);
			tank_turn_left(SPEED);
			_delay_ms(500);
		}
	}
	
	else
	{
		
		//ERROR nu, kanske hantering av målgång här senare ?
	}


}


void sensor_debug_message()
{
	clear_screen();
	send_string(sensor_buffer+1);
	update();
}

void sensor_debug_hex()
{
	uint8_t pos = 1;	// Gå förbi DEBUG_HEX-kommandot
	char tmpstr[10];
	while(pos != sensor_packet_length)
	{
		sprintf(tmpstr, "%02X", sensor_buffer[pos++]);
		send_string(tmpstr);
	}
	update();
}

//void interpret_small_ir(uint8_t value) //wtf??
uint8_t interpret_small_ir(uint8_t value)
{
 	uint8_t i = 0;
	
	if (value<19)
	{
		return 90;
	} 
	else if (value>140)
	{
		return 8;
	}
 	while ( !(value == small_ir_voltage_array[i] ) )
 	{
 		i++;
 	}
 	value = small_ir_centimeter_array[i];
 	return small_ir_centimeter_array[i];
}

//void interpret_big_ir(uint8_t value) //wtf??
uint8_t interpret_big_ir(uint8_t value)
{
	uint8_t i = 0;
	if (value<22)
	{
		return 150;
	}
	else if (value>138)
	{
		return 16;
	}
	while ( !(value == big_ir_voltage_array[i]) )
	{
		i++;
	}
	value = big_ir_centimeter_array[i];
	return big_ir_centimeter_array[i];
}

void analyze_ir_sensors()
{
	//Turn left, alley at front
	if(sensor_buffer[IR_LEFT_FRONT]>=MAXIMUM_IR_DISTANCE && sensor_buffer[IR_FRONT] >=MAXIMUM_IR_DISTANCE)
	{
		turn_left_alley_front();
	}
	//Turn Right, allay at front
	else if(sensor_buffer[IR_RIGHT_FRONT]>=MAXIMUM_IR_DISTANCE && sensor_buffer[IR_FRONT] >=MAXIMUM_IR_DISTANCE)
	{
		turn_right_alley_front();
	}
	
}

void turn_left_alley_front()
{
	//stäng av reglering
	//Kör till mitten på korsning(front = 120-(halva robotens längd))
	
	while(sensor_buffer[IR_FRONT] > DISTANCE_TO_ALLEY_END - IR_FRONT_TO_MIDDLE_LENGTH);
	
	stop_motors();
	_delay_ms(250);
	tank_turn_left(SPEED);
	//FIXA svängfunktion!
	
}

void turn_right_alley_front()
{
	//stäng av reglering
	//Kör till mitten på korsning(front = 120-(halva robotens längd))
	
	while(sensor_buffer[IR_FRONT] > DISTANCE_TO_ALLEY_END - IR_FRONT_TO_MIDDLE_LENGTH);
	
	stop_motors();
	_delay_ms(250);
	tank_right_left(SPEED);
	//FIXA svängfunktion!
	
}


/********************************************************
				Linjeföljning test
********************************************************/ 

int8_t get_position_from_reflex()
{
	
}