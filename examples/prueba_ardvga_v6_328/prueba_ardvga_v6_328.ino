#include <ardvga.h>

#include "rtype.h"
#include "cauldron.h"
#include "spectrum.h"
#include "gameover.h"
#include "saturn.h"
#include "melody.h"

ardvga mivga;

void setup() {
  mivga.begin(12,16,1); //107599 80939 56759 131112 106340 100155
  mivga.setSkipLine();
  mivga.setMode_720();
  mivga.tone (NOTE_A4,1000);
  mivga.delay(5000);


}

void loop() {
  uint32_t ciclo[5] = {0};
  /*for (uint8_t r=0;r<5;r++){
  uint32_t cic = mivga.countLines();*/
  /*const uint8_t PROGMEM *tmpBitmaps;
  const uint8_t PROGMEM *tmpAttributes;
  tmpBitmaps = spectrumBitmaps;
  tmpAttributes = spectrumAttributes;
  for (uint16_t z = 0; z < 17; z++){
    mivga.setSkipLine();
    mivga.loadzxscr(tmpBitmaps, tmpAttributes, 8*z*mivga.getHChars() + z);
    mivga.setNoSkipLine();
  }
  mivga.delay(5000);*/
  mivga.ink(inkBlue); mivga.paper(paperWhite); mivga.bPaper(brightPaper);mivga.bInk(noBright);
  mivga.cls();
  char buf[45]={0};
//  sprintf_P(buf,PSTR("%u\n%u,%u\n%u,%u\n%u,%u\n%u,%u\n%u,%u\n%u,%u\n%u,%u\n%u,%u\n%u,%u\n\0"),inkBlue | paperWhite | brightPaper | noBright,inkBlue,paperBlue,inkRed,paperRed,inkGreen,paperGreen,inkMagenta,paperMagenta,inkCyan,paperCyan,inkYellow,paperYellow,inkWhite,paperWhite,inkBlack,paperBlack,brightInk,brightPaper);
//  mivga.print(buf);
//  mivga.delay(60000);
  for (uint8_t l = 0; l < mivga.getVChars(); l++)
    for (uint8_t c = 0; c < mivga.getHChars(); c++) {
      mivga.setattr(l, c, (l & 7) << 3, c & 7, (c & 1) << 6, (l & 1) << 7);
      //putChar(((l*mivga.horizontalChars+c) % 95)+31, l, c); //ZX Charset
      mivga.putChar((l*mivga.getHChars()+c) & 0xff, l, c);//cp437 Charset
    }
  mivga.delay(10000);
  for (uint16_t thisNote = 0; thisNote < 53; thisNote++) {
    uint16_t noteDuration = ((2000) / mivga.safeReadFlashWord(noteDurations , thisNote));
    uint16_t noteTone = mivga.safeReadFlashWord(melody , thisNote);
    sprintf_P(buf,PSTR("             "));
    mivga.setCursor(0,0);
    mivga.print(buf);
    sprintf_P(buf,PSTR("%u,%u\0"),noteTone,noteDuration);
    mivga.setCursor(0,0);
    mivga.print(buf);
    mivga.tone(noteTone , noteDuration);
    mivga.delay(noteDuration);
    soundoff();
  }
  /*mivga.setSkipLine();
  mivga.ink(Black); mivga.paper(White); mivga.bPaper(1);mivga.bInk(0);mivga.ssa();
  for (uint8_t i = 0; i < (mivga.getVChars() * 8); i++) {
    mivga.scrollbu(1);
    mivga.delay(F_HORIZONTAL/10);
  }

  tmpBitmaps = rtypeBitmaps;
  tmpAttributes = rtypeAttributes;
  for (uint16_t z = 17; z > 0; z--){
    mivga.loadzxscr(tmpBitmaps, tmpAttributes, 8*(z-1)*mivga.getHChars() + z-1);
  }
  mivga.setNoSkipLine();
  mivga.delay(5000);

  for (int i=0;i<100;i++){
    for (int paper=White;paper>=Blue;paper--){
      for (int ink=Black;ink<=White;ink++){
        for (int paperbright=1;paperbright>=0;paperbright--){
          for (int inkbright=0;inkbright<=1;inkbright++){
              mivga.ink(ink); mivga.paper(paper); mivga.bPaper(paperbright);mivga.bInk(inkbright);mivga.ssa();
          }
        }
      }
    }
  }
  mivga.setSkipLine();
  for (uint8_t i = 0; i < (mivga.getVChars() * 8); i++) mivga.scrollbu(1);
  mivga.loadbitmap(saturnBitmaps, saturnAttributes,0);
  mivga.setNoSkipLine();
  mivga.delay(10000);
  mivga.setSkipLine();
  mivga.ink(Black); mivga.paper(White); mivga.bPaper(1);mivga.bInk(0);mivga.ssa();
  for (int x = 0; x < (mivga.getHPixels()) ; x++)
    for (int y = 0; y < (mivga.getVPixels()) ; y++) {
      mivga.plot(x, y);
      mivga.setattr(y / 8, x / 8, 7, y / (mivga.getVChars()) , 0, x / (mivga.getHChars() * 4));
    }
  mivga.setNoSkipLine();
  mivga.delay(5000);*/

  mivga.ink(inkYellow); mivga.paper(paperBlue); mivga.bPaper(noBright);mivga.bInk(brightInk);
  mivga.cls();
  /*for (uint8_t i = 0; i < mivga.verticalChars; i++) mivga.scrolltext(1);

  a0 = analogRead(0);
  float Vcc = (float)a0;
  Vcc = Vcc * 4.8876;
  mVcc = (int)Vcc;*/
  for (uint8_t r=0;r<5;r++){
    uint32_t cic = mivga.countLines();
    sprintf_P(buf, PSTR("Hola mundo!\0"));
    mivga.setCursor(2,5);
    mivga.print(buf);
    /*sprintf((char*)((void*)state), "mV=%i:C=%i", mVcc, ciclo);*/

    /*mivga.delay(2000);
    mivga.setSkipLine();*/
    mivga.line(0, 0, (mivga.getHPixels()) - 1, (mivga.getVPixels()) - 1);
    for (uint8_t x = 1; x < (mivga.getHPixels()) ; x++) {
      mivga.line(x, 0, (mivga.getHPixels()) - 1 - x, (mivga.getVPixels()) - 1);
      mivga.cline(x - 1, 0, (mivga.getHPixels()) - 1 - (x - 1), (mivga.getVPixels()) - 1);
    }
    for (uint8_t y = 1; y < (mivga.getVPixels()) ; y++) {
      mivga.line((mivga.getHPixels()) - 1, y, 0, (mivga.getVPixels()) - 1 - y);
      mivga.cline((mivga.getHPixels()) - 1, y - 1, 0, mivga.getVPixels() - y);
    }
    mivga.cline((mivga.getHPixels()) - 1, (mivga.getVPixels()) - 1, 0, 1);

    /*char buf[32]={0};
    for (int i=0;i<10000;i+=25){
      beep(i,1);
      pause(1);
      sprintf(buf, "Frecuencia=%i", i);
      putStr(buf, sizeof (buf), 11, 11,Paper,Ink,PaperBright,InkBright);
    }*/
    mivga.ink(inkGreen); mivga.paper(paperWhite); mivga.bPaper(brightPaper); mivga.bInk(noBright);
    mivga.cls();
    uint16_t hc = mivga.getHPixels();
    uint16_t vc = mivga.getVPixels();
    uint16_t R = min(hc / 2 , vc / 2);
    for (uint16_t i = 0; i < R; i += 3){
      mivga.plotEllipseRect(i , i , hc - i -1, vc - i - 1);
    }
    /*mivga.delay(2000);*/
    for (uint8_t j = 0 ; j <= 2 * mivga.getVChars() / 3 ; j += mivga.getVChars() / 3){
      mivga.setBox(j , mivga.getVChars() / 3);
      hc = mivga.getHPixels();
      vc = mivga.getVPixels();
      R = min(hc / 2 , vc / 2);
      for (uint16_t i = 0; i < R; i += 3){
        mivga.plotEllipseRect(i , i , hc - i - 1 , vc - i - 1);
    }
    /*mivga.delay(2000);*/
    mivga.cls();
    mivga.resetBox();
  }
  mivga.ink(inkMagenta); mivga.paper(paperYellow); mivga.bPaper(brightPaper); mivga.bInk(noBright);
  mivga.cls();
  /*mivga.setSkipLine();
  mivga.setMode_640();*/
  for (uint8_t j = 0 ; j < 1 ; j++){
    for (uint16_t i = 0; i < min(mivga.getHPixels() , mivga.getVPixels()); i += 4){
      mivga.plotCircle(i , i , i);
      mivga.plotCircle((mivga.getHPixels()) - i , i , i);
      mivga.plotCircle(i , (mivga.getVPixels()) - i , i);
      mivga.plotCircle((mivga.getHPixels()) - i , (mivga.getVPixels()) - i , i);
    }
     for (uint16_t i = 0; i < min(mivga.getHPixels() , mivga.getVPixels()); i += 4){
      mivga.clearCircle(i , i , i);
      mivga.clearCircle((mivga.getHPixels()) - i , i , i);
      mivga.clearCircle(i , (mivga.getVPixels()) - i , i);
      mivga.clearCircle((mivga.getHPixels()) - i , (mivga.getVPixels()) - i , i);
    }
  }
  /*mivga.setNoSkipLine();
  mivga.setMode_720();*/

//  mivga.delay(10000);
  ciclo[r]=mivga.countLines()-cic;
 }
  uint32_t maxc=0,minc=MAX_ULONG;
  uint32_t avgc=0;
  for (uint8_t r=0;r<5;r++){
    if (ciclo[r]>maxc) maxc=ciclo[r];
    if (ciclo[r]<minc) minc=ciclo[r];
    avgc+=ciclo[r];
  }
  avgc/=5;
  mivga.paper(paperBlue);
  mivga.ink(inkMagenta);
  mivga.bPaper(noBright);
  mivga.bInk(brightInk);
  mivga.setCursor(0, 0);

  sprintf_P(buf, PSTR("MAX:%lu,MIN:%lu,AVG:%lu\nRAM:%u\0"), maxc,minc,avgc,mivga.freeRam());
  mivga.print(buf);
  mivga.delay(30000);
}//59244 59240 59241
//58650 58593 58616
//58570 58513 58546
