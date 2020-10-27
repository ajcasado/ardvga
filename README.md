# ardvga
vga output for arduino boards using sinclair zx spectrum video model

This can output 16 color vga with 128x96px resolution from an arduino with an atmega328p at 16MHz (using ~1.5KB of sram).

With an atmega1284 at ~>26MHz you can obtain at least 256x192px with 16 colors as in the unforgettable ZX Spectrum from Sinclair Research Ltd.

Two different colors in every 8x8 pixel square, as used to be in the ZXSpectrum. (Yes, you can recreate the attribute clash!!!!).

The pixels are driven out by the serial port (USART) in MSPI mode (thanks to Nick Gammon). http://gammon.com.au/forum/?id=11608

The Vsync and Hsync signals are timed using Timer2 and dejittered using the sleep trick from Nick Gammon/Michael Rosen.http://forum.arduino.cc/index.php?topic=102181.msg1971465#msg1971465

The pixel output is used as the S input to a 74xx298 multiplexer to select the four bits of the paper/ink color. (Inspired in the Harlequin ZX Clone). http://www.zxdesign.info/colourselect.shtml

The flash bit of the ZX attribute byte format is not used, instead i use one bright bit for paper and other bright bit for ink.

A 6 resistor DAC is used to recreate the 1bit per channel RGB+BRIGHT color pallette of the ZX.(in the Harlequin clone transistors are used for a more ZX-accurate output of 15 colors).

Other modes can easily be achieved (8x4, 8x2 and 8x1 attribute modes).


This is a personal project, the code is ugly and incomplete and bad commented.


There are tested schematics for uno and nano boards based on atmega328p in the schematics folder.

 See in youtube :
 https://www.youtube.com/watch?v=gy4r95XHz44
 https://www.youtube.com/watch?v=BLAV3MRPJYY
 
 Last update, 1284p branch: 
 
 -Running on ATmega1284p with MCUdude MightyCore loaded.
 
 -Clock is 27Mhz.
 
 -Example runs at 256x192px. Can run at 328x240.
 
 -Set resolution in begin() call in chars (8x8px).
 
 -Define MODE_720 od MODE_640 in ardvga.h to select vga mode 720x400 or 640x480
 
 -Define EFFECT_SCANLINE or NO_EFFECT_SCANLINE to configure the scanline effect. With scanline activated runs faster.
 
VIDEO: https://youtu.be/EQStUlc8NUM
