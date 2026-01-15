# Teletext In Vision Decoder Board

This repo is an attempt to open-source my teletext decoder.

It was originally designed, tested, and even a few were sold by me in 2019.

You can see the [original product page for it on my website](https://www.nathanmediaservices.co.uk/invision/).

You are now free to do whatever you like with this design and the firmware for it - except selling it. You are **expressly forbidden** to use these designs to create a product that you then go on to resell, unless I have given you written permission otherwise.

Disclaimer: the design and firmwares are offered completely as-is, with no warranty of any kind. I will no longer be offering support or assistance on this project, if you want to build one you must figure it out from the resources provided here. 

## Why are there so many different firmwares?

Because I have built various slightly different boards into different projects and cases for different purposes, and with different ancillaries. For example, there are code bases here which support LCD screens, IR remotes, thumbwheel selectors, serial remotes, etc. You'll probably need to try a few and modify them to suit your needs.

## PCB Versions
There are three total versions of the decoder PCB. The first version was a prototype, and features a number of mistakes - I therefore do not recommend using it. Unless you're a complete masochist.

The first revision fixes the mistakes on the prototype and is ready to use. 

The second revision swaps the Arduino Nano for a standalone ATMega328(p) chip. This makes it cheaper and easier for manufacture, but harder for programming/debugging - therefore I don't recommend this revision for the novice.

## Notes
Depending on how you configure the ATMega328(p) in revision 2, you may omit the crystal Y2 and associated tuning capacitors.

You may also omit R17 and R18 - these are I2C pullup resistors, and I have yet to find a need for more pullup than already onboard the microcontroller.

You need not install the output circuitry for the S-Video or RGB outputs if you don't need those - on the other hand, if you only intend to use the RGB output you can probably get rid of the entire PAL coder circuit and just bridge the chip's RGB directly to the outputs.

Really, you only actually *need* the teletext decoder, video input circuitry, and the microcontroller - the rest depends on what you're using it for! I have shipped some very bare boards in specific circumstances!

Similarly, the board is designed to take a 9-15v DC input and regulate it down to 5v for operation - however, the LM7805 should really have a heatsink and can be unreliable. Personally I always bridge the regulator out and supply 5v directly from an external supply.

If you are having trouble getting colour to work on the PAL CVBS or S-Video outputs, you need to check the colour subcarrier frequency. Your crystal may require slightly different tuning capacitors. However I have found most TVs to be very forgiving of the subcarrier frequency.

If you do decide to do something cool with this design, please consider posting photos of it to Bluesky and tagging "@nmsni.co.uk", I would love to see it!

Also... consider kicking a quid or two my way at [Ko-Fi](https://ko-fi.com/avrovulcanxh607). It took a fair bit of time to figure this out.
