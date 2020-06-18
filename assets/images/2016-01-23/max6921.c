#include <msp430G2553.h>
#include "MAX6921.h"

//Define our pins
#define DATA BIT4  // P1.4
#define CLOCK BIT6 // P1.6
#define LATCH BIT5 // P1.5

void setupMax6921(void){
	P1DIR |= (DATA + CLOCK + LATCH);  // Setup pins as outputs
}
// Pulse the clock pin
void pulseClock(void){
  P1OUT |= CLOCK;
  P1OUT ^= CLOCK;
}

// Writes a value to the specified bitmask/pin. Use built in defines
// when calling this, as the shiftOut() function does.
// All nonzero values are treated as "high" and zero is "low"
void pinWrite(unsigned int bit, unsigned char val){
  if (val){
    P1OUT |= bit;
  } else {
    P1OUT &= ~bit;
  }
}

unsigned int getNum(unsigned int num){
	switch(num){
	//      	   0b--------ABCDEFGH
	case 0: return 0b0000000011111100;
	case 1: return 0b0000000001100000;
	case 2: return 0b0000000011011010;
	case 3: return 0b0000000011110010;
	case 4: return 0b0000000001100110;
	case 5: return 0b0000000010110110;
	case 6: return 0b0000000010111110;
	case 7: return 0b0000000011100000;
	case 8: return 0b0000000011111110;
	case 9: return 0b0000000011100110;
	default: return 0b0000000000000010;
	}
}


unsigned int getDigit(unsigned int digit){
	switch(digit){
	//             0b--------12345678;
	case 1: return 0b0000000010000000;
	case 2: return 0b0000000001000000;
	case 3: return 0b0000000000100000;
	case 4: return 0b0000000000010000;
	case 5: return 0b0000000000001000;
	case 6: return 0b0000000000000100;
	case 7: return 0b0000000000000010;
	case 8: return 0b0000000000000001;
	default: return 0b0000000000000000;

	}
}

void print(unsigned int digit, unsigned int num, unsigned int pm){
	//Set latch to low (should be already)
	  P1OUT &= ~LATCH;
	  unsigned int i;
	  unsigned int digitData;
	  unsigned int numData;
	  // If pm = 1; then we are drawing the PM dot
	  if(pm == 1){
		  digitData = 0;
		  numData = 1;
		  // Shift out the first 2 bits as zeros and the 3rd bit as a 1 (to indicate digit 9)
		  P1OUT &= ~DATA;
		  for(i = 0; i < 2; i++){
			  pulseClock();
		  	  }
		  P1OUT |= DATA;
		  pulseClock();
		  }
	  else{
		  numData = getNum(num);
		  digitData = getDigit(digit);
		  // Shift out the first 3 bits as zeros (2 not connected to IV-18, and we're not drawing PM dot)
		  P1OUT &= ~DATA;
		  for(i = 0; i < 3; i++){
			  pulseClock();
		  }
	  }
	 // Shift out digit bits
	 for(i=0; i<8; i++){
		 pinWrite(DATA, (digitData & (1 << i)));
		 pulseClock();
		 }
	 // Shift out the numner bits
	 for(i=0; i<8; i++){
	     pinWrite(DATA, (numData & (1 << i)));
	     pulseClock();
	 	 }
	 // shift out one more NC bit
	P1OUT &= ~DATA;
	pulseClock();
	// Pulse the latch pin to write the values into the storage register
	P1OUT |= LATCH;
	P1OUT &= ~LATCH;
}