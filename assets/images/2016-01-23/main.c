#include <msp430G2553.h>
#include "MAX6921.h"

//declare functions
void delay(unsigned int);
void printTime(void);
void addSec(void);
void addMin(void);
void addHour(void);
void flashLED(void);

// global variables
unsigned int hours = 12;
unsigned int minutes = 0;
unsigned int seconds = 0;
unsigned int pm = 0;

int main( void ){
  // Configure clock
  BCSCTL3 |= XCAP_2;            // enabling built in 6 pF capacitance for crystal

  // Configure the Watch dog timer for the RTC
  WDTCTL = WDT_ADLY_1000;       // watchdog interval timer mode ACLK
  IE1 = WDTIE;                  // enable watchdog timer interrupt
  setupMax6921();

  P1DIR |= BIT0+BIT2;
  P1DIR &= ~BIT1;
  P1OUT |= BIT1;				// P1.1 Pulled Up
  P1REN |= BIT1;				// P1.1 Resistor enabled
  P1IE |= BIT1;                 // P1.1 interrupt enabled
  P1IES |= BIT1;                // P1.1 Falling edge
  P1IFG &= ~BIT1;               // P1.1 IFG cleared
  P1SEL |= BIT2;				// P1.2 TA0

  P2DIR &= ~BIT0;
  P2OUT |= BIT0;				// P2.0 Pulled Up
  P2REN |= BIT0;				// P2.0 Resistor enabled
  P2IE |= BIT0;                 // P2.0 interrupt enabled
  P2IES |= BIT0;                // P2.0 Falling edge
  P2IFG &= ~BIT0;               // P2.0 IFG cleared

  // Configure timers for boost converter
  TA0CCR0 = 32-1;               // PWM Period ~500kHz
  TA0CCTL1 = OUTMOD_7;          // CCR1 reset/set
  TA0CCR1 = 8;                  // CCR1 PWM duty cycle
  TA0CTL = TASSEL_2 + MC_1;     // SMCLK, up mode

  _BIS_SR(GIE);                 // global interrupts enabled

  printTime();
}

// watchdog timer interrupt
 #pragma vector=WDT_VECTOR
 __interrupt void watchdog_timer(void){
   addSec();
 }
void delay(unsigned int ms){
 while (ms--){
        __delay_cycles(16000); // set for 1ms at 16MHz
    }
}

// Push Button Interrupt
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    delay(1);          // Debounce delay
    if((P1IN & BIT1)== 0){   // Check if button still pressed
    	seconds = 0;
    	addHour();
    }
	P1IFG &= ~BIT1;  // P1.1 IFG cleared
}

#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
    delay(1);          // Debounce delay
    if((P2IN & BIT0)== 0){   // Check if button still pressed
    	seconds = 0;
    	addMin();
    }
	P2IFG &= ~BIT0;  // P1.1 IFG cleared
}

void printTime(){
	// takes hours minutes and seconds (6-digits) and prints them on the VFD
	// HH-MM-SS
	unsigned int h1,h2,m1,m2,s1,s2;
	for(;;){
	h1 = hours/10;
	h2 = hours%10;
	m1 = minutes/10;
	m2 = minutes%10;
	s1 = seconds/10;
	s2 = seconds%10;
		print(8,h1,0);
		print(7,h2,0);
		print(6,10,0);
		print(5,m1,0);
		print(4,m2,0);
		print(3,10,0);
		print(2,s1,0);
		print(1,s2,0);
		if(pm==1) print(0,0,1);
	}
}

void addSec(){
  if(seconds < 59){
    seconds++;
  }
  else{
    addMin();
    seconds = 0;
  }
}
void addMin(void){
  if(minutes < 59){
    minutes++;
  }
  else{
    addHour();
    minutes = 0;
  }
}
void addHour(void){
  if(hours < 12){
	  if(hours == 11){
		  hours ++;
		  pm^=BIT0; //toggle AM/PM
	  }
	  else{
		  hours++;
	  }
  }
  else{
	  hours = 1;
  }
}
void flashLED(void){
  // Toggle LED
  P1OUT |= BIT0;
  __delay_cycles(1000);
  P1OUT ^= BIT0;
}