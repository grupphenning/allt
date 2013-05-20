/*
 * handle_display.h
 *
 * Created: 5/5/2013 1:27:14 PM
 *  Author: davek282
 */ 


#ifndef HANDLE_DISPLAY_H_
#define HANDLE_DISPLAY_H_

#include <stdint.h>


void init_default_printf_string();
void update_display_string();
extern char *crossing_decision_string;

extern uint8_t display_printf_string[];
extern uint8_t last_command[];



#endif /* HANDLE_DISPLAY_H_ */