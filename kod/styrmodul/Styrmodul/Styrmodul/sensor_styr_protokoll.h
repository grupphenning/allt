/* Sensor-styrprotokoll vid avbrott från sensor*/

//------------------Typ av sensor---------------
#define TYPE_OF_SENSOR 0b11000000
#define REFLEX 0b01000000
#define IR 0b00000000
#define GYRO 0b10000000

//------------------Reflex----------------------
#define CROSSING_RIGHT_PROT 0b01000000
#define CROSSING_LEFT_PROT 0b01001000
#define CROSSING_FORWARD_PROT 0b01010000
#define GOAL_PROT 0b01100000

//------------------IR--------------------------
// ska börja på 00

//------------------Gyro------------------------
// ska börja på 10


/* Sensor-styrprotokoll vid sampling från styr */
