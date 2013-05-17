/*
 * tapeh.h
 *
 * Created: 5/5/2013 4:35:08 PM
 *  Author: davek282
 */ 


#ifndef TAPEH_H_
#define TAPEH_H_


void regulate_end_tape();
void disable_crossings();
void enable_crossings();
void make_turn(char dir);
void drive_to_crossing_end(uint8_t stop_distance);
void add_to_crossing_buffer(char c);
void analyze_ir_sensors();
void handle_crossing();
uint8_t interpret_big_ir(uint8_t value);
uint8_t interpret_small_ir(uint8_t value);
void sensor_debug_hex();
void sensor_debug_message();
void decode_sensor(uint8_t data);

extern volatile uint8_t turn;
extern uint8_t tmp_sensor_buffer_p;
extern uint8_t tape_crossings;
extern uint8_t sensor_start;
extern uint8_t tmp_sensor_buffer_len;
extern uint8_t regulator_enable;
extern volatile uint8_t ninety_timer;
extern uint8_t autonomous;
extern uint8_t turning_180;
extern uint8_t display_auto_update;
extern uint8_t calibrate_sensors;
extern uint8_t make_turn_flag;
extern uint8_t listening_to_gyro;
extern int16_t degrees_full;


#endif /* TAPEH_H_ */