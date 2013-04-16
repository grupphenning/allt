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
#define INTERPOLATION_POINTS 12
uint8_t test;
volatile uint8_t spi_data_from_comm;
volatile uint8_t spi_data_from_sensor;
volatile uint8_t comm_interrupt_occoured = 0;
volatile uint8_t sensor_interrupt_occoured = 0;
//uint8_t amount = 255;
#define SPEED 255
uint8_t ninety_timer, turn;
uint8_t left = 1;


uint8_t sensor_buffer[SENSOR_BUFFER_SIZE];		// Buffer som håller data från sensorenheten
uint8_t sensor_buffer_pointer;			// Pekare till aktuell position i bufferten
uint8_t sensor_start;					// Flagga som avgör huruvida vi är i början av meddelande
uint8_t sensor_packet_length;					// Anger aktuell längd av meddelandet

uint8_t ir_voltage_array[INTERPOLATION_POINTS] = {2.74, 2.32, 1.64, 1.31, 1.08, 0.93, 0.74, 0.61, 0.52, 0.45, 0.41, 0.38};		// Innehåller de spänningsvärden som ses i grafen på https://docs.isy.liu.se/twiki/pub/VanHeden/DataSheets/gp2y0a21.pdf, sid. 4.
uint8_t ir_centimeter_array[INTERPOLATION_POINTS] = {8, 10, 15, 20, 25, 30, 40, 50, 60, 70, 80, 90};			// Innehåller de centimetervärden som ses i grafen på https://docs.isy.liu.se/twiki/pub/VanHeden/DataSheets/gp2y0a21.pdf, sid. 4.

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
	drive_forwards(85);	
	sei();		//aktivera global interrupts
	
	_delay_ms(200);
	while(1)
	{
// 		if(turn)
// 			tank_turn_left(207);
// 		else
// 			stop_motors();
		
		
		if(comm_interrupt_occoured)
		{
			comm_interrupt_occoured = 0;
			decode_comm();
		} 
		if(sensor_interrupt_occoured)
		{
			sensor_interrupt_occoured = 0;
			decode_sensor();
		}
	}
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
	setbit(SPCR, SPR0);

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

void spi_get_data_from_comm(uint8_t message_byte)
{
	clearbit(PORTB, PORTB3);	//Väljer komm
	SPDR = message_byte;		//Lägger in meddelande i SPDR, startar överföringen
	while(!(SPSR & (1 << SPIF)));
	setbit(PORTB, PORTB3);		//Sätter komm till sleepmode
	spi_data_from_comm = SPDR;
}

void spi_get_data_from_sensor(uint8_t message_byte)
{
	clearbit(PORTB, PORTB2);	//Väljer sensor
	SPDR = message_byte;		//Lägger in meddelande i SPDR, startar överföringen
	while(!(SPSR & (1 << SPIF)));
	setbit(PORTB, PORTB2);		//Sätter sensor till sleepmode
	spi_data_from_sensor = SPDR;
}

void spi_send_byte(uint8_t byte)
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
	CLAW_AMOUNT = 78*4;
}

void claw_in()
{
	CLAW_AMOUNT = 15*4;
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

/*//-----------------AVBRYT-------------------

//-----------------STYRKOMMANDON------------
uint8_t break_prot = 0b00000000;
uint8_t control_command_prot = 0b00100000;
uint8_t drive_prot = 0b00100000;
uint8_t back_prot = 0b00100100;
uint8_t stop_prot = 0b00101000;
uint8_t tank_turn_left_prot = 0b00101100;
uint8_t tank_turn_right_prot = 0b00110000;
uint8_t drive_turn_prot = 0b00110100;

uint8_t drive_turn_left_request = 0b00111000;
uint8_t drive_turn_right_request = 0b00111100;
//-----------KALIBRERING AV SENSORER---------

//-------------GRIPKLOKOMMANDON--------------
uint8_t claw_in_prot = 0b01100000;
uint8_t claw_out_prot = 0b01100100;
//----------SÄTT PD-KONSTANTER---------------
*/

// Kommunikationsenheten skickar en avbrottsförfrågan
ISR(INT1_vect)
{
	comm_interrupt_occoured = 1;
	spi_get_data_from_comm(0x00);	//Sparar undan data från comm
}

// Sensorenheten skickar en avbrottsförfrågan
ISR(INT0_vect)
{
	sensor_interrupt_occoured = 1;
	spi_get_data_from_sensor(0x00);
}

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

void decode_comm()
{
	uint8_t command = spi_data_from_comm;
	uint8_t byte = command & 0b11100000;
	
	if (command == BREAK_PROT)
	{
		// Någon som vet vilken "Avbryt"-funktion som avses i designspecen!?!?!?
		// Kör iaf den avbrytfunktion som avses i designspecen!!!!!!
		//send_string("break");
		//update();
	}
	else if (byte == CONTROL_COMMAND_PROT)
	{
		if (command == DRIVE_PROT)
		{
			drive_forwards(SPEED);
		}
		else if (command == BACK_PROT)
		{
			drive_backwards(SPEED);
		}
		else if (command == STOP_PROT)
		{
			stop_motors();
		}
		else if (command == TANK_TURN_LEFT_PROT)
		{
			tank_turn_left(SPEED);
//			send_string("Rotera vänster");
//			update();
		}
		else if (command == TANK_TURN_RIGHT_PROT)
		{
			tank_turn_right(SPEED);
//			send_string("Rotera höger");
//			update();
		}
		/*else if (command == drive_turn_prot)
		{
			spi_get_data_from_comm(drive_turn_left_request);
			spi_get_data_from_comm(drive_turn_left_request);
			turn_left(spi_data_from_comm);
			spi_get_data_from_comm(drive_turn_right_request);
			spi_get_data_from_comm(drive_turn_right_request);
			turn_right(spi_data_from_comm);
		}*/
	}	
	else if (command == CLAW_IN_PROT)
	{
		claw_in();
	}
	else if (command == CLAW_OUT_PROT)
	{
		claw_out();
	}
	else 
	{
		send_string("SYNTAX ERROR!");
		update();
	}
}


void decode_sensor()
{
	uint8_t data = spi_data_from_sensor;
	
	/* Första byten i ett meddelande är storleken */
	if(sensor_start)
	{
		sensor_packet_length = spi_data_from_sensor;
		sensor_start = 0;
		return;
	}

	/* Annars, lägg in inkommande byten i bufferten */
	sensor_buffer[sensor_buffer_pointer++] = spi_data_from_sensor;

	/* Om det är fler byte kvar att ta emot, vänta på dem! */
	if(sensor_buffer_pointer != sensor_packet_length)
		return;
	/* Aha, vi har tagit emot hela meddelandet! Tolka detta! */
	
	
	char tmpstr[50];
	clear_screen();
	sprintf(tmpstr, "%02X", sensor_packet_length);
	send_string(tmpstr);
	send_string(" ");
	sprintf(tmpstr, "%02X", sensor_buffer[0]);
	send_string(tmpstr);
	send_string(" ");
	sprintf(tmpstr, "%02X", sensor_buffer[1]);
	send_string(tmpstr);
	update();

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
		case SENSOR_FRONT:
			obstacle_check();
		default:
			// Unimplemented command
		break;
	}

	
	/* Återställ pekare, för att visa att nästa byte är början av meddelande */
	sensor_buffer_pointer = 0x00;
	sensor_packet_length = 0;
	sensor_start = 1;
	

//	uint8_t sensor_type = command & TYPE_OF_SENSOR;
// 	if( sensor_type == REFLEX )
// 	{
// 		if(command == CROSSING_RIGHT_PROT)	
// 		{
// 			tank_turn_right(SPEED);
// 			send_string( "h ");
// 			update();
// 		}
// 		else if (command == CROSSING_LEFT_PROT)
// 		{
// 			tank_turn_left(SPEED);	
// 			send_string("v ");
// 			update();
// 		}
// 		else if (command == CROSSING_FORWARD_PROT)
// 		{
// 			drive_forwards(SPEED);
// 		}
// 		
// 
// 	}
/*	else //SKRIV UT DATA*/


}




void sensor_debug_message()
{
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

void obstacle_check()
{
	if (sensor_buffer[1] >= 0x60)
	{
		stop_motors();
	}
}




// uint8_t interpret_ir(uint8_t value)
// {
// 	uint8_t i = 0;
// 	
// 	// Om värdet är närmast interpolationspunkt nr. i av alla punkter, returnera denna punkts centimetervärde.
// 	for ( !((value >= ( (ir_voltage_array[i] - ir_voltage_array[i+1]) >> 1 ) ) & (value <= ( (ir_voltage_array[i-1] - ir_voltage_array[i]) >> 1) )) )
// 	{
// 		i++;
// 	}
// 	return uint8_t ir_centimeter_array[i];
// }