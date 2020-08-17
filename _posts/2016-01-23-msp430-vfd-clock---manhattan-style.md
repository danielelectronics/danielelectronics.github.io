---
featured: true
layout: post
title: "MSP430 VFD Clock – Manhattan Style"
date: 2016-01-23 11:11:11
tags: electronics programming microcontroller
image: "/assets/images/2016-01-23/1.jpg"
description: "A clock that displays the time on an IV-18 vacuum florescent display tube, powered by the MSP430, and reusing some of the code from my MSP430 analog gauge clock."
permalink: /2016/01/23/msp430-vfd-clock-manhattan-style/
---

Overview
--------

I wanted to do a follow-up to my last clock build, the [MSP430 Analog Gauge Clock]({% post_url 2016-01-03-msp430-analog-gauge-clock %}), reusing some of the code from that project, and I had an IV-18 vacuum fluorescent display (VFD) tube that I bought on eBay. Also, I  wanted to finish the project before Christmas break was over. That didn’t happen. But I did manage to get the code written and most of the hardware built. I didn’t have the right parts on hand to build a boost converter to provide the 50V or so needed to drive the VFD, and school was about to start, so I decided to put that off until later, as adding an open-loop boost converter circuit using a PWM signal from my MSP430 would be pretty trivial. I’ve finally finished the project and decided to do a write-up.

I know that and IV-18 based clock isn’t anything original. In fact, [Adafruit used to sell a kit](https://www.adafruit.com/product/194) for this exact purpose. I don’t really care that it’s not original, because all vacuum tubes are cool, and doing all of the work yourself is really much cooler than using a kit. Also, I wanted to try my hand at Manhattan-style construction. Manhattan style circuits are built on a copper ground plane, with little islands for every node of the circuit. Because of the ground plane, and because it has fewer parasitics than stripboard or perfboard, Manhattan-style construction is often used for radiofrequency projects. My project isn’t a radio project, and the circuit probably would have worked just fine on a stripboard, but Manhattan-style construction gives a circuit a crude/retro look, which pairs nicely with a Russian vacuum tube.

If you wanna skip straight to the source code, the files are available here.
* [main.c](/assets/images/2016-01-23/main.c)
* [max6921.h](/assets/images/2016-01-23/max6921.h)
* [max6921.c](/assets/images/2016-01-23/max6921.c)

Driving the VFD
---------------

Originally I wanted to avoid using a VDF display driver. I wanted to use shift registers and decoders driven with my trusty MSP430G2553. Then I started reading the [datasheet of the IV-18](http://www.ladyada.net/media/icetube/IV-18.pdf). The tube needs +50V on each of the segment pins that need to be lit and +50V on the pin corresponding to the digit. This means that I would need high-voltage PNP transistors for each segment and each digit, for a total of 17 transistors. I could use an 8-bit shift register to drive the segments, but I would have needed 9 bits to control the characters (8 characters + the AM/PM indicator character). I could have used an 8-bit decoder and an extra pin on my microcontroller for the AM/PM. This would mean I’d need 7 total pins, three for the shift register, three for the decoder, and one for the AM/PM segment. This just wasn’t practical since I’d need to buy the high voltage transistors anyway. I decided to go with the Max6921 vacuum florescent display driver, which is basically just a 20-bit high voltage shift register. Here’s a link to the datasheet.

<figure class="singleImage">
  <div>
    <img src="/assets/images/2016-01-23/2.jpg" alt="IV-18 Tube">
    <p>FIV-18 Tube with Adafruit breakout board.</p>
  </div>
</figure>

As I mentioned before the microcontroller I’m using is the TI MSP430G2553. I bought several for my embedded systems class I took last semester so I have a bunch laying around. The core code for the real-time clock is the same as in my Analog Gauge Clock project. It uses 1 Hz interrupts generated from the Watchdog timer to keep time-based off a 32 kHz watch crystal.
Here are the important bits of that code:

{% highlight c linenos %}
// Configure clock
// enabling built-in 6 pF capacitance for crystal
BCSCTL3 |= XCAP_1;            

// Configure the Watch dog timer for the RTC
WDTCTL = WDT_ADLY_1000;       // watchdog interval timer mode ACLK
IE1 = WDTIE;                  // enable watchdog timer interrupt
{% endhighlight %}

If we look inside msp430g2553.h we can see what the WDT_ADLY_1000 is actually doing to the watchdog control register.

{% highlight c linenos %}
// WDT is clocked by fACLK (assumed 32KHz)
#define WDT_ADLY_1000 (WDTPW+WDTTMSEL+WDTCNTCL+WDTSSEL) // 1000ms
{% endhighlight %}

We can see that it is setting up the watchdog timer to trigger an interrupt every second, based on an input from ACLK, which is exactly what we want.

The interrupt service routine is shown below.

{% highlight c linenos %}
// watchdog timer interrupt
#pragma vector=WDT_VECTOR __interrupt
void watchdog_timer(void) {
  addSec();
}
{% endhighlight %}

The addSec() function simply increments a global variable called seconds, or, if seconds is rolling over, calls the addMin() function. There’s also an addHour() function which does the exact same thing.

There are two push button interrupts that are used to set the clock. The first interrupt resets the seconds variable and calls the addMin() function. The other interrupt does the same thing but calls the addHour() function. I used interrupts on two different ports with as little code in them as possible to minimize the time spent in the interrupt service routines. This is because the IV-18 tube is driven by the processor, and any time that the processor is not driving the display creates flicker.

The MAX6921 is driven with some shifting code that I wrote to display the time. I used the shift register [example by Andrew Morton](http://processors.wiki.ti.com/index.php/MSP430_Launchpad_Shift_Register) on the MSP430 Launchpad wiki to write the code. The most important function is called print() and the implementation is shown below. The implementation is pretty specific to my application, and, though it could be re-written, isn’t intended to be used as a MAX6921 or general-purpose shift register driver.

{% highlight c linenos %}
void print(unsigned int digit, unsigned int num, unsigned int pm){
    //Set latch to low (should be already)
      P1OUT &amp;= ~LATCH;
      unsigned int i;
      unsigned int digitData;
      unsigned int numData;
      // If pm = 1; then we are drawing the PM dot
      if(pm == 1){
          digitData = 0;
          numData = 1;
          // Shift out the first 2 bits as zeros and the 3rd bit as a
          // 1 (to indicate digit 9)
          P1OUT &amp;= ~DATA;
          for(i = 0; i &lt; 2; i++){
              pulseClock();
                }
          P1OUT |= DATA;
          pulseClock();
          }
      else{
          numData = getNum(num);
          digitData = getDigit(digit);
          // Shift out the first 3 bits as zeros (2 not connected to
          // IV-18, and we're not drawing PM dot)
          P1OUT &amp;= ~DATA;
          for(i = 0; i &lt; 3; i++){
              pulseClock();
          }
      }
     // Shift out digit bits
     for(i=0; i&lt;8; i++){
         pinWrite(DATA, (digitData &amp; (1 &lt;&lt; i)));
         pulseClock();
         }
     // Shift out the numner bits
     for(i=0; i&lt;8; i++){
         pinWrite(DATA, (numData &amp; (1 &lt;&lt; i)));
         pulseClock();
          }
     // shift out one more NC bit
    P1OUT &amp;= ~DATA;
    pulseClock();
    // Pulse the latch pin to write the values into the storage
    // register
    P1OUT |= LATCH;
    P1OUT &amp;= ~LATCH;
}
{% endhighlight %}

The getNum() and getDigit() functions are switch statements that return the binary representation for the number or digit I am trying to write to the VFD.

The main program continually loops the same function call, printTime() which relies on the print() function in the MAX6921.c file.

{% highlight c linenos %}
void printTime(){
    // takes hours minutes and seconds (6-digits) and prints them on
    // the VFD
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
{% endhighlight %}

To get each digit from the hours, minutes, and seconds variables separate, the function first divides by 10 do get the ten’s place digit, and them modulos (is that the verb form?) by 10 to get the one’s place digit.

The power supply is made of two LM317’s to provide the 3.3 V for the MSP430 and the 5 V for the MAX6921. The 50 V for the IV-18 is provided by an open-loop boost converter that steps the 12 V from the plug-pack up to 50 V. The boost converter uses a 330 uH inductor I had in my junk bin, a high voltage N-channel MOSFET ([IRLU3410PBF](http://www.irf.com/product-info/datasheets/data/irlr3410pbf.pdf)), a Schottky diode ([SR2010-TPCT](http://www.mccsemi.com/up_pdf/SR208-SR2010%28DO-41%29.PDF)), and a 100 V 100 uF capacitor. The boost converter is open-loop, so there’s no feedback to the MSP430 and no control law. The load is very consistent and will work across a big voltage range, so control isn’t necessary. The output of the boost converter was measured to be around 50 V. The PWM code is shown below.

{% highlight c linenos %}
// Configure timers for boost converter
TA0CCR0 = 32-1;               // PWM Period ~500kHz
TA0CCTL1 = OUTMOD_7;          // CCR1 reset/set
TA0CCR1 = 8;                  // CCR1 PWM duty cycle
TA0CTL = TASSEL_2 + MC_1;     // SMCLK, up mode
{% endhighlight %}

That’s about it for the code. I wired up the circuit Manhattan-style using a piece of copper-clad board, and a bunch of copper-clad “islands” I made using my dad’s air punch/flanger tool that are held down with super glue. I ended up using a breakout board from Adafruit for the IV-18 tube, which is [still available on their website](https://www.adafruit.com/products/343) at the time of writing so that I could plug the tube into a socket so that I can take the tube out to show people. The final results are shown below with a quick demo video.

<figure class="singleImage">
  <div>
    <img src="/assets/images/2016-01-23/3.jpg" alt="Circuit construction.">
    <p>Close up on the wiring with the tube out of its socket.</p>
  </div>
</figure>

<figure class="singleImage">
  <div>
    <img src="/assets/images/2016-01-23/4.jpg" alt="Final board.">
    <p>Final circuit with the tube.</p>
  </div>
</figure>

<iframe height="500" src="https://www.youtube.com/embed/FJc-KY1ck6U" frameborder="0" allow="accelerometer; autoplay; encrypted-media; gyroscope; picture-in-picture" allowfullscreen></iframe>
