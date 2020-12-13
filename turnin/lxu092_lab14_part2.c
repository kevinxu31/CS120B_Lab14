/*	Author: lab
 *  Partner(s) Name: Luofeng Xu
 *	Lab Section:
 *	Assignment: Lab 14  Exercise 2
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	Demo Link: Youtube URL>https://youtu.be/Wqz3DTqQJe0
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

#ifndef USART_1284_H
#define USART_1284_H

// USART Setup Values
#define F_CPU 8000000UL // Assume uC operates at 8MHz
#define BAUD_RATE 9600
#define BAUD_PRESCALE (((F_CPU / (BAUD_RATE * 16UL))) - 1)

////////////////////////////////////////////////////////////////////////////////
//Functionality - Initializes TX and RX on PORT D
//Parameter: usartNum specifies which USART is being initialized
//			 If usartNum != 1, default to USART0
//Returns: None
void initUSART(unsigned char usartNum)
{
	if (usartNum != 1) {
		// Turn on the reception circuitry of USART0
		// Turn on receiver and transmitter
		// Use 8-bit character sizes 
		UCSR0B |= (1 << RXEN0)  | (1 << TXEN0);
		UCSR0C |= (1 << UCSZ00) | (1 << UCSZ01);
		// Load lower 8-bits of the baud rate value into the low byte of the UBRR0 register
		UBRR0L = BAUD_PRESCALE;
		// Load upper 8-bits of the baud rate value into the high byte of the UBRR0 register
		UBRR0H = (BAUD_PRESCALE >> 8);
	}
	else {
		// Turn on the reception circuitry for USART1
		// Turn on receiver and transmitter
		// Use 8-bit character sizes
		UCSR1B |= (1 << RXEN1)  | (1 << TXEN1);
		UCSR1C |= (1 << UCSZ10) | (1 << UCSZ11);
		// Load lower 8-bits of the baud rate value into the low byte of the UBRR1 register
		UBRR1L = BAUD_PRESCALE;
		// Load upper 8-bits of the baud rate value into the high byte of the UBRR1 register
		UBRR1H = (BAUD_PRESCALE >> 8);
	}
}
////////////////////////////////////////////////////////////////////////////////
//Functionality - checks if USART is ready to send
//Parameter: usartNum specifies which USART is checked
//Returns: 1 if true else 0
unsigned char USART_IsSendReady(unsigned char usartNum)
{
	return (usartNum != 1) ? (UCSR0A & (1 << UDRE0)) : (UCSR1A & (1 << UDRE1));
}
////////////////////////////////////////////////////////////////////////////////
//Functionality - checks if USART has successfully transmitted data
//Parameter: usartNum specifies which USART is being checked
//Returns: 1 if true else 0
unsigned char USART_HasTransmitted(unsigned char usartNum)
{
	return (usartNum != 1) ? (UCSR0A & (1 << TXC0)) : (UCSR1A & (1 << TXC1));
}
////////////////////////////////////////////////////////////////////////////////
// **** WARNING: THIS FUNCTION BLOCKS MULTI-TASKING; USE WITH CAUTION!!! ****
//Functionality - checks if USART has recieved data
//Parameter: usartNum specifies which USART is checked
//Returns: 1 if true else 0
unsigned char USART_HasReceived(unsigned char usartNum)
{
	return (usartNum != 1) ? (UCSR0A & (1 << RXC0)) : (UCSR1A & (1 << RXC1));
}
////////////////////////////////////////////////////////////////////////////////
//Functionality - Flushes the data register
//Parameter: usartNum specifies which USART is flushed
//Returns: None
void USART_Flush(unsigned char usartNum)
{
	static unsigned char dummy;
	if (usartNum != 1) {
		while ( UCSR0A & (1 << RXC0) ) { dummy = UDR0; }
	}
	else {
		while ( UCSR1A & (1 << RXC1) ) { dummy = UDR1; }
	}
}
////////////////////////////////////////////////////////////////////////////////
// **** WARNING: THIS FUNCTION BLOCKS MULTI-TASKING; USE WITH CAUTION!!! ****
//Functionality - Sends an 8-bit char value
//Parameter: Takes a single unsigned char value
//			 usartNum specifies which USART will send the char
//Returns: None
void USART_Send(unsigned char sendMe, unsigned char usartNum)
{
	if (usartNum != 1) {
		while( !(UCSR0A & (1 << UDRE0)) );
		UDR0 = sendMe;
	}
	else {
		while( !(UCSR1A & (1 << UDRE1)) );
		UDR1 = sendMe;
	}
}
////////////////////////////////////////////////////////////////////////////////
// **** WARNING: THIS FUNCTION BLOCKS MULTI-TASKING; USE WITH CAUTION!!! ****
//Functionality - receives an 8-bit char value
//Parameter: usartNum specifies which USART is waiting to receive data
//Returns: Unsigned char data from the receive buffer
unsigned char USART_Receive(unsigned char usartNum)
{
	if (usartNum != 1) {
		while ( !(UCSR0A & (1 << RXC0)) ); // Wait for data to be received
		return UDR0; // Get and return received data from buffer
	}
	else {
		while ( !(UCSR1A & (1 << RXC1)) );
		return UDR1;
	}
}

#endif USART_1284_H

typedef struct task{
        int state;
        unsigned long period;
        unsigned long elapsedTime;
        int(*TickFct)(int);
}task;

task tasks[2];
const unsigned short tasksNum=2;
const unsigned long tasksPeriod=10;
volatile unsigned char TimerFlag = 0;
unsigned long _avr_timer_M = 1;
unsigned long _avr_timer_cntcurr = 0;
void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}
void TimerOn() {
	TCCR1B 	= 0x0B;
	OCR1A 	= 125;
	TIMSK1 	= 0x02;
	TCNT1 = 0;
	_avr_timer_cntcurr = _avr_timer_M;
	SREG |= 0x80;
}

void TimerOff() {
	TCCR1B 	= 0x00;
}

void TimerISR() {
	unsigned char i;
	for(i=0;i<tasksNum;++i){
		if(tasks[i].elapsedTime>=tasks[i].period){
			tasks[i].state=tasks[i].TickFct(tasks[i].state);
			tasks[i].elapsedTime=0;
		}
		tasks[i].elapsedTime+=tasksPeriod;
	}
}
ISR(TIMER1_COMPA_vect)
{
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0) {
		TimerISR();
		_avr_timer_cntcurr = _avr_timer_M;
	}
}

#define B0 (PINB&0x01)
enum Lead_LED{init,lead};
int Lead_Tick(int state){
	static unsigned char x=0;
	static unsigned char temp=0;
	switch(state){
		case init:
			if(B0==1){
				state=lead;
			}
			else{
				state=init;
			}
			break;
		case lead:
			if(B0==1){
				state=lead;
				PORTC=0x01;
			}
			else if(B0==0){
				state=init;
			}
			break;
		default:
			break;
	}
	switch(state){
		case init:
			PORTC=0;
			break;
		case lead:
			if(USART_IsSendReady(1)){
				x=!x;
				PORTA=x;				
				USART_Send(x,1);
			}
			break;
		default:
			break;
	}
	return state;
}

enum Follow_LED{initf,follow};
int Follow_Tick(int state1){
	static unsigned char temp=0;
        switch(state1){
                case initf:
			if(B0==0){
				state1=follow;
			}
			else{
				state1=initf;
			}
			break;
		case follow:
			if(B0==0){
				state1=follow;
			}
			else if(B0==1){
				state1=initf;
			}
			break;
		default:
			break;
	}
	switch(state1){
		case initf:
			PORTC=1;
			break;
		case follow:
			if(USART_HasReceived(0)){
				temp=USART_Receive(0);			
				PORTA=temp;
			}
		 	break;
                default:
                        break;
        }
        return state1;
}


int main(void) {
	initUSART(0);
	USART_Flush(0);
	initUSART(1);
	USART_Flush(1);
	DDRA=0xFF;PORTA=0x00;
	DDRB=0x00;PORTB=0xFF;
	DDRC=0xFF;PORTC=0x00;
	unsigned char i=0;
	tasks[i].state=init;
        tasks[i].period=1000;
        tasks[i].elapsedTime=0;
        tasks[i].TickFct=&Lead_Tick;
	i++;
	tasks[i].state=initf;
	tasks[i].period=20;
	tasks[i].elapsedTime=0;
	tasks[i].TickFct=&Follow_Tick;
	TimerSet(10);
	TimerOn();
    	while (1) {

    	}
    	return 1;
}
