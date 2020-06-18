/*
 * MAX6921.h
 *
 *  Created on: Jan 5, 2016
 *      Author: Dan
 */

#ifndef VFDCLOCK_MAX6921_H_
#define VFDCLOCK_MAX6921_H_

#include <msp430G2553.h>

void setupMax6921(void);
void pulseClock(void);
void pinWrite(unsigned int, unsigned char);
unsigned int getNum(unsigned int);
unsigned int getDigit(unsigned int);
void print(unsigned int, unsigned int, unsigned int);

#endif /* VFDCLOCK_MAX6921_H_ */