/*
This program was written by Daniel Johnson and Doug Jensen for the MSP430 Analog Gauge Clock Project
Inputs and outputs:
P1.1, P1.3, P1.4 are used as push button inputs
P1.3 is the set button, while this button is held down, the clock can be set by resetting the time to 12:00
P1.4 is the hours button - pressing this while holding down the set button adds hours to the time 
P1.1 is the minutes button - pressing this while holding down the set button adds minutes to the time 
There is no AM/PM designation on this clock. 

hours go from 0-660, increments of 60, Where 0 indicates 1:00 and 660 indicates 12:00
minutes go from 0-649, increments of 11, where 0=0 and 649=59
seconds go from 0-649, increments of 11, where 0=0 and 649=59

P1.2 is the PWM output for seconds
P2.1 is the PWM output for minutes
P2.4 is the PWM output for hours
*/

#include "msp430g2553.h"

unsigned int seconds, minutes, hours = 0;
void flashLED(void);
void addSec(void);
void addMin(void);
void addHour(void);
void delay(int);
void setPWM(void);
int main( void )
{
  // Configure clock
  BCSCTL3 |= XCAP_1;            // enabling built in 6 pF capacitance for crystal
  
  // Configure the Watch dog timer for the RTC
  WDTCTL = WDT_ADLY_1000;       // watchdog interval timer mode ACLK
  IE1 = WDTIE;                  // enable watchdog timer interrupt

  // Configure timers for PWM, timer A1 and A2 will be used
  TA0CTL = TASSEL_1 + MC_1;       
  TA1CTL = TASSEL_1 + MC_1; 
  TA0CCR0 = 660-1;              // Periods for both timers
  TA1CCR0 = 660-1;
  TA0CCTL0 = OUTMOD_7;          // Reset/Set
  TA0CCTL1 = OUTMOD_7;          // Reset/Set
  TA1CCTL0 = OUTMOD_7;          // Reset/Set
  TA1CCTL1 = OUTMOD_7;
  TA1CCTL2 = OUTMOD_7;
  setPWM();
  
  // Configure pins for PWM, and Pushbuttons
  P1DIR = BIT2+BIT6;
  P1SEL = BIT2;
  P1OUT = 0x00;
  P2OUT = 0x00;
  P2DIR = BIT1+BIT4;
  P2SEL |= BIT1+BIT4;
  P1OUT = BIT0+BIT1+BIT4+BIT3;  // P1.3 Pulled up
  P1REN |= BIT0+BIT1+BIT4+BIT3;      // Enable internal pull-up/down resistor on P1.3
  P1IE |= BIT3;                 // P1.3 interrupt enabled
  P1IES |= BIT3;                // P1.3 Falling edge
  P1IFG &= ~BIT3;               // P1.3 IFG cleared
  
  _BIS_SR(GIE);     // global interrupts enabled
}
// watchdog timer interrupt
#pragma vector=WDT_VECTOR
__interrupt void watchdog_timer(void)
{
  flashLED();
  addSec();
  setPWM();
}

// Push Button Interrupt
#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
    delay(100);          // Debounce delay
    if((P1IN & BIT3)== 0){   // Check if button still pressed
      P1OUT |= BIT0;
      // Reset clock to 1:00
      seconds = 0;
      minutes = 0;
      hours = 0; 
      while((P1IN & BIT3) == 0){
        // Poll minute and hour buttons
        if((P1IN & BIT1) == 0){
          delay(100); // debounce
          if((P1IN & BIT1) == 0){
            addMin(); 
          }
        }
        if((P1IN & BIT4) == 0){
          delay(100); // debounce
          if((P1IN & BIT4) == 0){
            addHour();
          }
        }
        if((P1IN & BIT0)==0){ // Enter a calibration mode where each meter is maxed out.
        	while((P1IN & BIT0)==0){
        		hours = 660;
        		minutes = 660;
        		seconds = 660;
        		setPWM();
        	}
        	hours = 0;
        	minutes = 0;
        	seconds = 0;
        	setPWM();
        }
        setPWM();
      } 
    }
   P1OUT &= ~BIT0;
   P1IFG &= ~BIT3;  // P1.3 IFG cleared
}

void addSec(){
  if(seconds<649){
    seconds+=11;;
  }
  else{
    addMin();
    seconds = 0;
  }
}
void addMin(void){
  if(minutes < 649){
    minutes+=11;
  }
  else{
    addHour();
    minutes = 0;
  }
}
void addHour(void){
  if (hours < 660){
    hours +=60;
  }
  else hours = 0 ;
}
void setPWM(void){
  TA0CCR1 = seconds;
  TA1CCR1 = minutes;
  TA1CCR2 = hours;
}
void flashLED(void){
  // Toggle LED
  P1OUT ^= BIT6;					
  __delay_cycles(3000);
  P1OUT ^= BIT6;
}
void delay(int time){
	int i =0;
  for(i=0; i<time; i++){
    __delay_cycles(1000);
  }
}