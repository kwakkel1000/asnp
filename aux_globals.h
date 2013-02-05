#pragma once

#define F_CPU 16000000UL                //Your clock speed in Hz (16Mhz here)

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <avr/pgmspace.h>

#include <stdlib.h>

void * operator new(size_t size);
void operator delete(void * ptr);

//custom headsers

//delay functions

//-----------------delays---------------------------------------------------------
#define LOOP_CYCLES 8                           //Number of cycles that the loop takes

#define fcpu_delay_us(num) delay_int(num/(LOOP_CYCLES*(1/(F_CPU/1000000.0))))
#define fcpu_delay_ms(num) delay_int(num/(LOOP_CYCLES*(1/(F_CPU/1000.0))))

void delay_int(unsigned long delay);
//--------------------------------------------------------------------------------


#define PORT_ON( port_letter, number )                  port_letter |= (1<<number)
#define PORT_OFF( port_letter, number )                 port_letter &= ~(1<<number)
#define PORT_ALL_ON( port_letter, number )              port_letter |= (number)
#define PORT_ALL_OFF( port_letter, number )             port_letter &= ~(number)
#define FLIP_PORT( port_letter, number )                port_letter ^= (1<<number)
#define PORT_IS_ON( port_letter, number )               ( port_letter & (1<<number) )
#define PORT_IS_OFF( port_letter, number )              !( port_letter & (1<<number) )

