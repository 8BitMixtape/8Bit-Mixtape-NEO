# 8Bit Mixtape NEO
New Attiny85 based edition with Bootloader to program it via sound.

## First Working prototypes
See our first working prototype in the video: https://www.youtube.com/watch?v=9HnzJeK31R8

See more documentation of the developments on this repo's wiki: https://github.com/8BitMixtape/8bitMixTape-SoundProg2085/wiki


## Background
Inspired by the bootloader for atmega by Christoph Haberer and other implementations by Fredrik...
* http://www.hobby-roboter.de/forum/viewtopic.php?f=4&t=127
* http://www.fredrikolofsson.com/f0blog/?q=node%2F596

Not found anything yet for attiny, except that's it's a bit more challenging to make bootloaders.
* http://www.mikrocontroller.net/articles/Konzept_f%C3%BCr_einen_ATtiny-Bootloader_in_C

Can we maybe use the same bootloader to program via blinking lights?
* https://www.wayneandlayne.com/blinky_programmer/

# Bootloader in progress
see devel repo here: 
* Iyoks devel: https://github.com/ATtinyTeenageRiot/TinyAudioBoot
* forked and improved by Chris: https://github.com/ChrisMicro/TinyAudioBoot
* find all files, pre-compiled and examples by Chris: https://github.com/ChrisMicro/AttinySound

# Boards are being designed atm...

![Special Edition Gong Xi Fa Cai](https://github.com/8BitMixtape/8bitMixTape-SoundProg2085/blob/master/boards/images_schematics/85SoundProg_MixTape_V03-NEO.jpg)

![Making board in yogya](https://github.com/8BitMixtape/8bitMixTape-SoundProg2085/raw/master/boards/FirstPrototype_Neo03/AudioProgMixTape_MakingOf.png)

See more boards on the wiki: https://github.com/8BitMixtape/8bitMixTape-SoundProg2085/wiki/Boards

# Test your prototype

Notes from our first testing phase: https://github.com/8BitMixtape/8bitMixTape-SoundProg2085/wiki/Testing-the-prototype-Boards

# Discussing the schematics
how to free even more pins on that little attiny85...

## Version 0.8 Final - Changed Sound to PB1

![](https://raw.githubusercontent.com/wiki/8BitMixtape/8bitMixTape-SoundProg2085/Schematics_85SoundProg_MixTape_V08.png)

Parts:
* R1-4: 10K \(Voltage dividers\)
* R5: 10K \(Buttons to be tested\)
* R6: 4K7 \(Buttons to be tested\)
* R7: 100 \(NEO-Pixel data stabilizer\)
* R8: 100 \(Output Filter\)
* R9-10: LED resistor \(depends on color\)
* Pot1-2: 10K lin \(Potentiometers\)
* C1: 100nF \(SD-Prog de-coupling\)
* C2: 1uF \(Output Filter\)
* C3: 100nF \(Voltage stabilization\)
* C4: 100nF \(de-coupling output?\)

Pin Use:
* PB0\(pin5\): NEO-pixels
* PB1\(pin6\): Sound output \(parallel LED\)
* PB2\(pin7\): \(A1\) Potentiometer \(right\), CV input
* PB3\(pin2\): \(A3\) Buttons/Switch via Voltage-Dividers
* PB4\(pin3\): \(A2\) Potentiometer \(left\), SD-Prog Jack-3.5mm Input
* PB5\(pin1\): \(A0 \)kinda free pin, can be used for sync / gate signal and communication from the bootloader
* GND\(pin4\):
* Vcc\(pin8\): Power LED, stabilizing cap Vcc-GND. 

## Older versions

See wiki: https://github.com/8BitMixtape/8bitMixTape-SoundProg2085/wiki/Schematics-Discussion

# Basic stuff we need

Interface:
* 2 potentiometers
* 2 push-buttons
* 1 Sound output PWM (PB0 is preferred) is also CV output in case of Sequencer code
* Sound-progger input
* CV input
* Master-clock or gate output
* Sync-clock input
* 1 pin to communicate wiht the 8 NEO-pixels (is that cheating?)
* what else?

# The Beginning...

Late night geeking at Iyok's ego-lab...

![Late night geeking at Iyok's ego-lab](https://github.com/8BitMixtape/8bitMixTape-SoundProg2085/raw/master/boards/FirstPrototype_Neo03/IMG_20170127_191152.jpg)

After years of geeking around wiht all these versions of 8bit Mixtapes...

http://lifepatch.org/8-bit_Mixtape

or 

http://wiki.sgmk-ssam.ch/wiki/8bit_Mix_Tape

