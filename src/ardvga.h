/* ardvga.h - Library to drive a VGA display using the ZX Spectrum computer family color attribute model
 * Created by Antonio J. Casado Alías, XXX, 2017.
 * Released under the XXX License.
 * Based on the work of XXX
 *///Cambiar a ZXVga

#ifndef ardvga_h
#define ardvga_h

#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include "font.h"
#include "pitches.h"

//#include "font8x8_box.h"

#if defined(__AVR_ATmega644__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)

  #define MCU_1284

  #define UCPHA1 1 /*not defined in maniacbug files, search for better support for this chip*/
  #define pixel_ton() UCSR1B = bit (TXEN1)
  #define pixel_toff() UCSR1B = 0
  #define PIXEL_DR UDR0
  //#define _720 48149
  //#define _640 56281
  #define VGA_SYNC_CR DDRD
  #define VGA_SYNC_PORT PORTD
  #define VSYNC_PIN PD5
  #define VSYNC_PIN_UP_MASK B00100000 //calcular a partir de VSYNC_PIN
  #define VSYNC_PIN_DOWN_MASK B11011111 //calcular a partir de VSYNC_PIN
  #define HSYNC_PIN PD6
  #define VGA_PIXEL_CR DDRD
  #define VGA_PIXEL_PORT PORTD
  #define VGA_PIXEL_PIN PD3
  #define VGA_CLOCK_PIN PD4
  #define VGA_ATTRIBUTE_CR DDRC
  #define VGA_ATTRIBUTE_PORT PORTC
  #define SOUNDPORT PORTA //hacer opcional la compilacion del sonido
  #define SOUNDPORTCR DDRA
  #define SOUNDPIN PA0
  #define soundoff() SOUNDPORT &= ~(1<<SOUNDPIN) //check if is calculated at compile time
  #define soundon() SOUNDPORT |= (1<<SOUNDPIN)
  #define horizontalChars 32
  #define verticalChars 24

#endif

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega168P__) || defined(__AVR_ATmega328P__)

  #define MCU_328

  #define pixel_ton() UCSR0B = (1<<TXEN0)
  #define pixel_toff() UCSR0B = 0
  #define PIXEL_DR UDR0
  #define H_SYNC_CR DDRD
  #define H_SYNC_PORT PORTD
  #define HSYNC_PIN PD3
  #define V_SYNC_CR DDRB
  #define V_SYNC_PORT PORTB
  #define VSYNC_PIN PB0
  #define VSYNC_PIN_UP_MASK B00000001 // (1<<VSYNC_PIN)
  #define VSYNC_PIN_DOWN_MASK B11111110 // ~VSYNC_PIN_UP_MASK
  #define VGA_PIXEL_CR DDRD
  #define VGA_PIXEL_PORT PORTD
  #define VGA_PIXEL_PIN PD1
  #define VGA_CLOCK_PIN PD4
  #define VGA_ATTRIBUTE_CR DDRC
  #define VGA_ATTRIBUTE_PORT PORTC
  #define VGA_ATTRIBUTE_B_CR DDRD
  #define VGA_ATTRIBUTE_B_PORT PORTD
  #define VGA_ATTRIBUTE_B_PIN PIND
  #define VGA_ATTRIBUTE_B_MASK B11000000
  #define SOUNDPORT PORTD
  #define SOUNDPORTCR DDRD
  #define SOUNDPIN PD5
  #define soundoff() SOUNDPORT &= ~(1<<SOUNDPIN) //check if is calculated at compile time
  #define soundon() SOUNDPORT |= (1<<SOUNDPIN)
  //#define horizontalChars 17
  //#define verticalChars 11

#endif


/*  For the standard arduino AVR clock of 16MHZ i found that with my vga monitor (LG W2261)
 *  for HT=73(max~faster:720@61Hz,640@51Hz) -> max horizontalChars w/skipLine is 20 , max horizontalChars w/noSkipLine is 19
 *  for HT=69(opt~med):720@64Hz,640@54Hz) -> max horizontalChars w/skipLine is 18 , max horizontalChars w/noSkipLine is 17
 *  for HT=68(subopt~med):720@65Hz,640@55Hz) -> max horizontalChars w/skipLine is 16 , max horizontalChars w/noSkipLine is 15
 *  for HT=63(standard vga~slower):720@70Hz,640@60Hz) -> max horizontalChars w/skipLine is 15 , max horizontalChars w/noSkipLine is 14
 *  Check diferent timings for your monitor starting from 63 and set the higest value for hT that works to achieve the best performance,
 *  mine is 73, default is 63
 */
 //Se podría usar el puerto i2c del estandar vga para obtener un valor optimo

//#define STD_HT 63 // Using a 16bit timer maybe the limits can be pushed more
#define MAX_HT (((F_CPU/8)/27027)-1) /*experimental minimum synchronizable F_HORIZONTAL timing for LG W2261 using 8bit counter (i2c hack?)*/
#define STD_HT (((F_CPU/8)/31469)-1) /*F_HORIZONTAL industry standard*/ /*std_HT 63*/
#define HT 65
//#define HT STD_HT
//#define HT 73 //(70 max with schneider TV)
#define F_HORIZONTAL ((F_CPU/8)/(HT+1)) // 8 is the TIMER2 prescale
#define _720 0
#define _640 1

#define Black 0
#define Blue 1
#define Red 2
#define Magenta 3
#define Green 4
#define Cyan 5
#define Yellow 6
#define White 7
#define BLANK B00000000

#define inkBlue B001
#define inkRed B010
#define inkGreen B100
#define inkMagenta (inkBlue | inkRed)
#define inkCyan (inkBlue | inkGreen)
#define inkYellow (inkRed | inkGreen)
#define inkWhite (inkBlue | inkRed | inkGreen)
#define inkBlack B00000000
#define paperBlue inkBlue << 3
#define paperRed inkRed  << 3
#define paperMagenta inkMagenta  << 3
#define paperGreen inkGreen  << 3
#define paperCyan inkCyan  << 3
#define paperYellow inkYellow  << 3
#define paperWhite inkWhite  << 3
#define paperBlack B00000000
#define brightInk B10000000
#define brightPaper B01000000
#define noBright B00000000


#define MAX_ULONG 4294967295

#define horizontalPixels (ardvga::horizontalChars * 8)
#define verticalPixels (ardvga::verticalChars * 8)
#define horizontalTop (horizontalPixels - 1)
#define verticalTop (verticalPixels - 1)
#define vMemSize (ardvga::horizontalChars * verticalPixels)
#define aMemSize (ardvga::horizontalChars * ardvga::verticalChars)//hacer la estructura de atributos flexible en cuanto a la dimension horizontal y vertical de la pantalla, en horizontal  el numero total de pixeles dividido por multiplos de 8 y en vertical el numero total de pixeles dividido por potencias de dos

#define nop() __asm__("nop\n\t")
#define waitsendbyte() nop();nop();nop();nop();nop()
#define waitfrontporch() nop();nop();nop()

extern "C" void TIMER2_OVF_vect(void) __attribute__ ((signal));
extern "C" void TIMER2_COMPB_vect(void) __attribute__ ((signal));

class ardvga{

  public:
    static void begin(uint8_t height , uint8_t width,uint8_t doSplash);
    static void end();
    static bool setResolution(uint8_t height , uint8_t width);
    static void setupIO();
    static void setCRTCTRL(uint8_t ht);
    static void loadzxscr(const uint8_t scrBitmaps[] PROGMEM , const uint8_t scrAttributes [] PROGMEM , uint16_t offset);
    static void loadbitmap(const uint8_t scrBitmaps[] PROGMEM , const uint8_t scrAttributes [] PROGMEM , uint16_t offset);
    static void cls();
    static void clb();
    static void ssa();
    static bool setattr(uint8_t line, uint8_t column, uint8_t paper, uint8_t ink, uint8_t paperbright, uint8_t inkbright);
    static bool plot(uint16_t x , uint16_t y); //the use of  should be tested against speed
    static bool cplot(uint16_t x , uint16_t y);
    static void line(int16_t x0 , int16_t y0 , int16_t x1 , int16_t y1);
    static void cline(int16_t x0 , int16_t y0 , int16_t x1 , int16_t y1);
    static void plotCircle(int16_t xm , int16_t ym , int16_t r);
    static void clearCircle(int16_t xm , int16_t ym , int16_t r);
    static void plotEllipseRect(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
    static void clearEllipseRect(int16_t x0, int16_t y0, int16_t x1, int16_t y1);
    static void scrollbu(uint16_t i);
    static void scrollau(uint8_t i);
    static void scrolltext(uint8_t i);
    static void delay(uint32_t i);
    static uint32_t countLines();
    static uint16_t freeRam ();
    static void tone (uint16_t frequency , uint32_t duration);
    static void setSkipLine();
    static void setNoSkipLine();
    static void toggleSkipLine();
    static void toggleMode();
    static uint8_t safeReadFlashByte (const uint8_t pgmArray[] , uint16_t index);
    static uint16_t safeReadFlashWord (const uint16_t pgmArray[] , uint16_t index);
    static uint8_t vFreq(uint8_t ht);
    static uint16_t hFreq(uint8_t ht);
    static void setMode_640();
    static void setMode_720();
    static void paper(uint8_t p);
    static void ink(uint8_t i);
    static void bPaper(uint8_t pb);
    static void bInk(uint8_t ib);
    static uint8_t getHChars();
    static uint8_t getVChars();
    static uint16_t getHPixels();
    static uint16_t getVPixels();
    static bool putChar (uint8_t c,  uint8_t line,  uint8_t column);
    static bool putStr (char str[], size_t strSize,  uint8_t line, uint8_t column, uint8_t paper, uint8_t ink, uint8_t paperbright, uint8_t inkbright);
    static bool print (char* buf);
    static bool setCursor (uint8_t x, uint8_t y);
    static uint8_t getXPos();
    static uint8_t getYPos();
    static bool setBox(uint8_t y, uint8_t height);
    static void resetBox();
    friend void TIMER2_OVF_vect(void);
    friend void TIMER2_COMPB_vect(void);
    static uint8_t isDoingLine();


	private:

    static volatile uint32_t lineCounter;
    static volatile uint32_t sndDur;
    static volatile uint16_t scanLine;
    static volatile uint16_t drawLine;
    static volatile uint16_t hLine;
    static volatile uint8_t doLine;
    static uint16_t sndFreq;
    static uint16_t vFrontPorch;
    static uint8_t skipLine;
    static uint8_t *bitmask;
    static uint8_t *attributes;
    static uint8_t verticalChars;
    static uint8_t horizontalChars;
    static uint8_t *bitmaskBck;
    static uint8_t *attributesBck;
    static uint8_t verticalCharsBck;
    static uint8_t mode;
    static uint8_t Ink;
    static uint8_t Paper;
    static uint8_t InkBright;
    static uint8_t PaperBright;
    static uint8_t hT;
    static uint8_t xPos;
    static uint8_t yPos;
    static void lineProc_noskipLine();
    static void lineProc_skipLine();
};

#endif
