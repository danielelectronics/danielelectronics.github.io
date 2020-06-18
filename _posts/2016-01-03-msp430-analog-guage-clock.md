---
featured: true
layout: post
title: "MSP430 Analog Gauge Clock"
date: 2016-01-03 11:11:11
tags: electronics programming microcontroller
image: "/assets/images/2016-01-03/1.jpg"
description: "A clock that displays the time using analog ammeters, made with the MSP430 microcontroller. This was a final project for a class, but it was a lot of fun, and has a really cool steam-punk look to it."
---

Overview
--------

After seeing an analog meter clock years ago on Hackaday ([link](http://hackaday.com/2012/08/24/volt-meter-clock-also-displays-the-temperature/)), I decided it was time I built my own, using the MSP430G2553 microcontroller that I was using the embedded systems class I was taking. I wanted to use the clock as the final project for the class, so I decided to implement the real time clock using the microcontroller itself, instead of pairing it with a serial real time clock module. The whole project consists of just the microcontroller, a watch crystal, some resistors and trimmer pots, and three analog ammeters. I completed the project with the help of my buddy Doug and ended up getting an A on the project. If you don’t want to read the entire post, you can download at the entire source code [here](/assets/images/2016-01-03/main.c).

Implementing the Real Time Clock
--------------------------------

Like I said before, this was a project for my embedded systems class, so I wanted to do everything using only the microcontroller we used in class, the MSP430G2553 that comes with the TI Launchpad. I implemented the real time clock (RTC) using a 32.768 kHz watch crystal (the one the comes with the launchpad) and the watchdog timer. I decided to use the watchdog timer so I could use the other two timers on the ‘G2553 for pulse width modulating the ammeters that are used to indicate the time.

To get the RTC working, first the clock source must be configured. I used the LFXT1CLK clock source with the crystal and set the loading capacitance to 6pF. I experimented with an oscilloscope to see which loading capacitance worked best because I wasn’t sure what the parasitic capacitance of the strip-board circuit was. TI has a great application not on selecting crystals and loading capacitors. Here’s a [link](http://www.ti.com/lit/an/slaa322d/slaa322d.pdf) (PDF warning) if you want to give it a read. Then, I set the watchdog timer to a 1000ms period and enabled its interrupt as shown in the code below.

{% highlight c linenos %}
// Configure clock
// enabling built in 6 pF capacitance for crystal
BCSCTL3 |= XCAP_1;            

// Configure the Watch dog timer for the RTC
WDTCTL = WDT_ADLY_1000;       // watchdog interval timer mode ACLK
IE1 = WDTIE;                  // enable watchdog timer interrupt
{% endhighlight %}

If we look inside msp430g2553.h we can see what the WDT_ADLY_1000 is actually doing to the watchdog control register.

{% highlight c linenos %}
// WDT is clocked by fACLK (assumed 32KHz)
#define WDT_ADLY_1000       (WDTPW+WDTTMSEL+WDTCNTCL+WDTSSEL) // 1000ms
{% endhighlight %}

We can see that it is setting up the watchdog timer to trigger an interrupt every second, based on an input from ACLK, which is exactly what we want.

The interrupt service routine is shown below.

{% highlight C linenos %}
// watchdog timer interrupt
#pragma vector=WDT_VECTOR __interrupt  
void watchdog_timer(void){
  addSec();
}
{% endhighlight %}


The flashLED() function simple flashes the led (P1.0) on the Launchpad for debugging purposes. The addSec() and setPWM() functions are more important. Lets look at the addSec(), addMin() and addHour() functions.

{% highlight c linenos %}
void addSec(){
  if(seconds &amp;amp;lt; 649){
    seconds+=11;;
  }
  else{
    addMin();
    seconds = 0;
  }
}
void addMin(void){
  if(minutes &amp;amp;lt; 649){
    minutes+=11;
  }
  else{
    addHour();
    minutes = 0;
  }
}
void addHour(void){
  if (hours &amp;amp;lt; 660){
    hours +=60;
  }
  else hours = 0 ;
}
{% endhighlight %}

Seconds and Minutes increment by 11 until 649, and hours increments by 60 until 660. The code is written this way to make setting the Pulse Width Modulation duty cycles easier.

Probing the crystal reveals a 32.768 kHz sine wave, and probing P1.0 shows the led flashing at 1 Hz.

![Image2](/assets/images/2016-01-03/2.jpg)
![Image2](/assets/images/2016-01-03/3.jpg)

Pulse Width Modulation
----------------------

Because this clock uses ammeters to display time, the amount of current flowing through the meter is proportional to the deflection of the needle. To control the deflection of the needle we need to be able to control the current through the meter. This is done using pulse width modulation (PWM) and resistors.

Timers A0 and A1 are used to generate the PWM signal for the meters.

{% highlight c linenos %}
// Configure timers for PWM, timer A1 and A2 will be used
TA0CTL = TASSEL_1 + MC_1;
TA1CTL = TASSEL_1 + MC_1;
TA0CCR0 = 660-1;              // Periods for both timers
TA1CCR0 = 660-1;
TA0CCTL0 = OUTMOD_7;          // Reset/Set
TA0CCTL1 = OUTMOD_7;
TA1CCTL0 = OUTMOD_7;
TA1CCTL1 = OUTMOD_7;
TA1CCTL2 = OUTMOD_7;
setPWM();
{% endhighlight %}

TASSEL_1 sets the timer source to ACLK and MC_1 puts the timer in mode count: up mode. OUTMOD_7 puts the capture and compare register in set/reset mode, to generate the PWM signal.
The setPWM() function sets the capture and compare registers’s value to the current time, which sets the duty cycle of the PWM signal.

{% highlight c linenos %}
void setPWM(void){
  TA0CCR1 = seconds;
  TA1CCR1 = minutes;
  TA1CCR2 = hours;
}
{% endhighlight %}

This graphic from the MSP430 User Guide (giant PDF warning) does a great job of explaining how PWM signals are generated using the timers and the capture and compare registers.

![Image4](/assets/images/2016-01-03/4.png)

Setting the clock was done using an interrupt on the “set” push button which initiates polling of the “minute” and “hour” push buttons. I’m not going to go into the details of how to use push buttons and interrupts with the MSP430, there are plenty of tutorials out there for the beginner at [TI’s Launchpad Tutorials page](https://training.ti.com/getting-started-msp430g2553-value-line-launchpad-workshop-series).

Hardware
--------

Enough with the software, lets look at the hardware. Problem is, there really isn’t much to this project besides the microcontroller. The power supply is the classic LM317 circuit set to about 3 volts, as shown in the schematic below.

![Image_Regulator](/assets/images/2016-01-03/5.png)

Choosing the divider resistors controls the feedback which sets the output voltage. The equation taken from the LM317 datasheet from TI is shown below.

![Image_calculation](/assets/images/2016-01-03/6.png)

I measured an output voltage closer to 3.03 V. The output voltage is fairly stable but could change as the clock is running 24/7. For this reason trimmer resistor were added in series with the meters so that the meters can be tuned to match the divisions exactly. The complete schematic is shown below.

![Image_schematic](/assets/images/2016-01-03/7.png)

The ammeters I used for this project were 1 mA analog panel meters purchased from [Amazon](https://www.amazon.com/uxcell-Mounted-Current-Amperemeter-Measuring/dp/B0087YAZNS/ref=sr_1_5?ie=UTF8&qid=1451871624&sr=8-5&keywords=ammeter+analog+1mA). My buddy Doug is an autoCAD wizard; he scanned in the old meter face plates to get the arc correct, and drew in new divisions for hours, minutes, and seconds. The whole thing only took him a few minutes. Here’s a [link to a PDF](/assets/images/analog-meters.pdf) of the custom face plates.

The body of the clock is 28 gauge steel. I decided not to paint it so it would have a retro industrial style. The surface rust only adds to the cool in my opinion. I used the metal shop at my university to cut, drill and bend the sheet metal into the clock face, and I assembled the circuit on a piece of strip board.

![Image_back](/assets/images/2016-01-03/8.jpg)

All the parts except for the ammeters and the sheet metal came from my junk bins.

Here’s a picture of the circuit built on stripboard (with a random chip acting as a placeholder, I didn’t have a DIP-20 package so I used a DIP-28). Once again, the full source code
can be found [here](/assets/images/2016-01-03/main.c).

![Image_Circuit](/assets/images/2016-01-03/9.png)

Update
------

[My project got featured on Hackaday!!!](https://hackaday.com/2016/01/05/current-meter-shows-current-time/) So exciting!!! Turns out one of the users spotted a big mistake. In the original video, the second hand only makes it to 55 before resetting. Turns out, it was actually getting to 59 seconds as any useful clock would, but the trimmer pots were out of wack so the meter never got to full deflection. Tuning the trimmer pots was a pain in the butt, so I added a couple lines of code to the time-setting interrupt. If P1.0 is pulled low, the microcontroller maxes out each meter until P1.0 goes high again. Now, to tune the trimmer pots, all I have to do is put a jumper on P1.0, and tune the trimmer pots until each meter’s needle points right to 60 (or 11 for the hours meter). The new code is shown below.

{% highlight c linenos %}
if((P1IN & BIT0)==0){
// Enter a calibration mode where each meter is maxed out.
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
{% endhighlight %}

The video and the source code have been updated.

<iframe height="500" src="https://www.youtube.com/embed/Us5rB6u8hQ8" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>
