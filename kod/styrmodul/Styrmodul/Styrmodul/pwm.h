/*
 * pwm.h
 *
 * Created: 5/5/2013 1:21:17 PM
 *  Author: davek282
 */ 


#ifndef PWM_H_
#define PWM_H_

void pwm_init();
void drive_forwards(uint8_t amount);
void drive_backwards(uint8_t amount);
void turn_left(uint8_t amount);
void turn_right(uint8_t amount);
void stop_motors();
void tank_turn_left(uint8_t amount);
void tank_turn_right(uint8_t amount);
void claw_in();
void claw_out();
void begin_turning(int16_t degrees);


#endif /* PWM_H_ */