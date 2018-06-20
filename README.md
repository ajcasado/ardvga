# ardvga
vga output for arduino boards using sinclair zx spectrum video model

This can output 16 color vga with 128x96px resolution from an arduino with an atmega328p at 16MHz (using ~1.5KB of sram).

With an atmega1284 at ~>26MHz you can obtain at least 256x192px with 16 colors as in the unforgettable ZX Spectrum from Sinclair Research Ltd.

Two different colors in every 8x8 pixel square, as used to be in the ZXSpectrum. (Yes, you can recreate the attribute clash!!!!).

The pixels are driven out by the serial port (USART) in MSPI mode (thanks to Nick Gammon). http://gammon.com.au/forum/?id=11608

The Vsync and Hsync signals are timed using Timer2 and dejittered using the sleep trick from Nick Gammon/Michael Rosen.http://forum.arduino.cc/index.php?topic=102181.msg1971465#msg1971465

The pixel output is used as the S input to a 74xx298 multiplexer to select the four bits of the paper/ink color. (Inspired in the Harlequin ZX Clone).

A 6 resistor DAC is used to recreate the 1bit RGB+BRIGHT color pallete of the ZX.(in the Harlquin clone transistors are used for a more ZX-accurate output of 15 colors, but with 6 resistors we get 16 colors).

Other modes can easily be achieved (8x4, 8x2 and 8x1 attribute modes).


This is a personal project, the code is ugly and incomplete and bad commented.


If there is any interest I can publish the schematics, and comment out the code.
