---
featured: false
layout: post
title: "Testing a DC-DC Converter Module"
date: 2016-02-09 11:11:11
tags: electronics
image: "/assets/images/2016-02-09/1.png"
description: "Testing an LM7805-pin-compatible buck converter, just for fun."
permalink: /2016/02/09/testing-a-dc-dc-converter-module/
---

Overview
--------

I needed a 5V rail for a project that would be sourcing current for a USB battery charger from a 12V wall adapter. At these kinds of loads (<= 1A) a linear voltage regulator like the LM7805 is impractical, as it has to drop a lot of voltage and dissipate a lot of power. That’s why I opted to use a buck converter instead.

I needed it quick so instead of going through the trouble of picking a switching controller, a MOSFET, an inductor, a diode, and a capacitor, and going through all of the calculations, I decided to try out a little DC-DC converter module from Digikey. That being said, there is a lot to learn from building a DC-DC converter circuit from scratch. Going through all the calculations and choosing the right controller, FET, inductor, etc. is a great way to learn how to find a part and how to address the trade-offs that engineering is all about. But I was in a rush, so I ordered some of [these modules](https://www.digikey.com/product-detail/en/V7805-1000R/102-2178-ND/2352137). I was browsing Adafruit and saw the little LM78xx pin-compatible buck converters which instantly reminded me of the mailbag video Dave put up on the eevblog where he tested a similar product. Not wanting to pay the 15 bucks for the [Adafruit modules](https://www.adafruit.com/products/1065) I headed over to Digikey and found the V7805-1000R buck converter module from CUI Inc. I ordered a couple along with the recommended input and output capacitors and waited for them to arrive so I could test their efficiency and see how close I got to their claimed 97% efficiency.

<figure class="sideBySide">
  <div>
    <img src="/assets/images/2016-02-09/2.jpg" alt="CUI inc module from Digikey">
    <p>CUI inc. module from Digikey.</p>
  </div>
  <div>
    <img src="/assets/images/2016-02-09/3.jpg" alt="Traco Power module available on Adafruit">
    <p>Traco Power module available on Adafruit.</p>
  </div>
</figure>

97% efficiency is awesome. Switch-mode converters are almost perfect from an efficiency point of view. The efficiency, in this case, is the ratio of the input power to the output power. At 100% efficiency, the input and output power would be equal, and the device itself wouldn’t have to dissipate any power.

Let's take the case of the LM7805 (which isn’t really fair as linear voltage regulators are a totally different animal, which have their own advantages over switching converters). In my application the input voltage is 12V and the output voltage is 5V at a 1A load. This means the regulator has to drop 7V at 1A, which is 7W. It’ll probably get a little toasty, and will probably need a heat sink. The efficiency is the output power (5W) divided by the input power (12W) times 100. The efficiency would be about 42% in this case.

The [datasheet](The datasheet for the V7805-100R shows the peak efficiency occurs at the minimum input voltage of 6.5V at a load of around 200mA.) for the V7805-100R shows the peak efficiency occurs at the minimum input voltage of 6.5V at a load of around 200mA.

![efficiency](/assets/images/2016-02-09/4.png)

My test will be at an input voltage of 12V, so the efficiency numbers probably won’t reach the 97% shown in the datasheet.

My test circuit will consist of a DC power supply set to 12V, a multimeter used as an ammeter on the input, another meter on the output used as an ammeter, and a third meter to measure the voltage at the input and output. The meters I used are as follows: 1. [Fluke 77 IV](http://assets.fluke.com/manuals/77iv____umeng0000.pdf), 2. a [BK Precision Test Bench 390a](https://bkpmedia.s3.amazonaws.com/downloads/datasheets/en-us/38xx_datasheet.pdf), and 3. a [Craftsmen 82141](https://www.manualslib.com/manual/477270/Craftsman-82141.html). The voltages were measured at the terminals of the device, and the DC supply was adjusted to compensate for the multimeter burden voltage.

<figure class="singleImage">
<div>
  <img src="/assets/images/2016-02-09/5.jpg" alt="Test Setup">
  <p>Left – multimeters used as ammeters, Right – DC-DC converter module circuit</p>
</div>
</figure>

The table below shows the data gathered.

| Vin (V) | Vout (V) | Iin (mA) | Iout (mA) | Pin (mW) | Pout (mW) | Efficiency (%) |
|---------|----------|----------|-----------|----------|-----------|----------------|
| 12.045  | 5.06     | 30       | 50        | 361.35   | 253       | 70.01522       |
| 12.1    | 5.07     | 50       | 100       | 605      | 507       | 83.80165       |
| 12.04   | 5.06     | 70       | 151       | 842.8    | 764.06    | 90.65733       |
| 12.08   | 5.09     | 100      | 201       | 1208     | 1023.09   | 84.69288       |
| 12.07   | 5.12     | 120      | 250       | 1448.4   | 1280      | 88.37338       |
| 12.03   | 5.11     | 150      | 314       | 1804.5   | 1604.54   | 88.91881       |
| 12.01   | 5.01     | 160      | 354       | 1921.6   | 1773.54   | 92.29496       |
| 12.02   | 5        | 190      | 405       | 2283.8   | 2025      | 88.66801       |
| 12      | 5        | 210      | 450       | 2520     | 2250      | 89.28571       |
| 12      | 5        | 230      | 504       | 2760     | 2520      | 91.30435       |
| 11.99   | 5        | 250      | 546       | 2997.5   | 2730      | 91.0759        |
| 11.99   | 5        | 280      | 603       | 3357.2   | 3015      | 89.80698       |
| 12.02   | 4.99     | 330      | 721       | 3966.6   | 3597.79   | 90.70211       |
| 12      | 4.99     | 400      | 863       | 4800     | 4306.37   | 89.71604       |
| 11.98   | 4.99     | 450      | 981       | 5391     | 4895.19   | 90.80301       |
| 12.01   | 4.99     | 510      | 1111      | 6125.1   | 5543.89   | 90.51101       |

The graph of efficiency versus load is shown below, the peak efficiency is around 92%. Not bad for a 12V input.

![Graph](/assets/images/2016-02-09/6.png)
