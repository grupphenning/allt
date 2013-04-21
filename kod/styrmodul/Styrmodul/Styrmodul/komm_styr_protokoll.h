//-----------------STYRKOMMANDON------------
#define COMM_BREAK				0x00
//#define CONTROL_COMMAND_PROT	0x01
#define COMM_DRIVE				0x02
#define COMM_BACK				0x03
#define COMM_STOP				0x04
#define COMM_LEFT				0x05
#define COMM_RIGHT				0x06
//#define DRIVE_TURN_PROT			0x07
#define COMM_DRIVE_LEFT			0x09
#define COMM_DRIVE_RIGHT		0x0A
#define COMM_TURN_90_DEGREES_LEFT 0x10
#define COMM_TURN_90_DEGREES_RIGHT 0x11
//-----------KALIBRERING AV SENSORER---------

//-------------GRIPKLOKOMMANDON--------------
#define COMM_CLAW_IN			0x0B
#define COMM_CLAW_OUT			0x0C

//----------SÄTT PD-KONSTANTER---------------
#define COMM_SET_PID			0x08

//----------DEBUG-----------------------------
#define COMM_DISPLAY			0x0D
#define COMM_CLEAR_DISPLAY		0x0E
#define COMM_TOGGLE_SENSORS		0x0F
