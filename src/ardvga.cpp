#include "ardvga.h"


/*en linea 196:

      if (ardvga::hLine > (ardvga::sndFreq / 2/*^(255/vol) o (1<<vol)))*/
/*cambiando el 2 debería poder hacer "PWM" y controlar el volumen de salida*/
volatile uint8_t ardvga::doLine = 0;
volatile uint16_t ardvga::drawLine = 0;
volatile uint16_t ardvga::scanLine = 0;
volatile uint16_t ardvga::hLine = 0;
volatile uint32_t ardvga::sndDur = 0;
volatile uint32_t ardvga::lineCounter = 0;
uint8_t ardvga::skipLine = 0;
uint8_t* ardvga::bitmask = 0;
uint8_t* ardvga::attributes = 0;
uint8_t ardvga::verticalChars = 0;
uint8_t ardvga::horizontalChars = 0;
uint8_t* ardvga::bitmaskBck = 0;
uint8_t* ardvga::attributesBck = 0;
uint8_t ardvga::verticalCharsBck = 0;
uint8_t ardvga::mode = 0;
uint8_t ardvga::Ink = 0;
uint8_t ardvga::Paper = 0;
uint8_t ardvga::InkBright = 0;
uint8_t ardvga::PaperBright = 0;
uint8_t ardvga::hT = 0;
uint8_t ardvga::xPos = 0;
uint8_t ardvga::yPos = 0;
uint16_t ardvga::sndFreq = 0;
uint16_t ardvga::vFrontPorch = 0;

void ardvga::begin(uint8_t height , uint8_t width , uint8_t doSplash /*añadir modo y hT como argumntos?*/){
  hT = HT;
  skipLine = 0;
  mode = _720;
  setResolution(height , width);
  setupIO();
  cli();
  setCRTCTRL(hT);
  sei();
  if (doSplash) {
    Ink = inkWhite; Paper = paperBlack; PaperBright=noBright;InkBright=noBright;cls();
    char initMessage[15] = {0};
    sprintf_P(initMessage, PSTR("ArdVga Library\n\0"));
    print(initMessage);
    sprintf_P(initMessage, PSTR("==============\n\0"));
    print(initMessage);
    sprintf_P(initMessage, PSTR("|Version 0.10|\n\0"));
    print(initMessage);
    sprintf_P(initMessage, PSTR("|AJCA<-->2017|\n\0"));
    print(initMessage);
    sprintf_P(initMessage, PSTR("==============\n\0"));
    print(initMessage);
    sprintf_P(initMessage, PSTR("Free RAM:~%u\n\0"), freeRam());
    print(initMessage);
    sprintf_P(initMessage, PSTR("Mode:%ux%u@%u\n\0"), horizontalPixels , verticalPixels , vFreq(hT));
    print(initMessage);
    sprintf_P(initMessage, PSTR("Pixel RAM:%u\n\0"), height*width*8 );
    print(initMessage);
    sprintf_P(initMessage, PSTR("Color RAM:%u\n\0"), height*width);
    print(initMessage);
    sprintf_P(initMessage, PSTR("H.Freq:%uHz\n\0"), hFreq(hT));
    print(initMessage);
    for (uint8_t i=Blue;i<White;i++) setattr(0, i-1, Paper, i, PaperBright, InkBright);
    for (uint8_t i=Blue;i<=White;i++) setattr(0, i+6, Paper, i, PaperBright, 1);
  }
  tone (NOTE_A4,1000);
}
void ardvga::end(){}
void ardvga::setSkipLine(){
  if (skipLine) return;
  skipLine = 1;
}
void ardvga::setNoSkipLine(){
  if (!skipLine) return;
  skipLine = 0;
}
void ardvga::setMode_720(){
  if (mode == _720) return;
  mode = _720;
}
void ardvga::setMode_640(){
  if (mode == _640) return;
  mode = _640;
}
void ardvga::toggleSkipLine(){
  skipLine = !skipLine;
}
void ardvga::toggleMode(){
  switch (mode) {
    case _720:
      mode = _640;
      break;
    case _640:
      mode = _720;
  }
}
bool ardvga::setResolution(uint8_t height , uint8_t width){
  if (attributes) free(attributes);
  if (bitmask) free(bitmask);
  if (bitmask = (uint8_t *) malloc (((height*8)*width)*sizeof(uint8_t)))
    if (attributes = (uint8_t *) malloc ((height*width)*sizeof(uint8_t))){
      verticalChars = height;
      horizontalChars = width;
      vFrontPorch = (((verticalChars*8)+9)*4);
      bitmaskBck = bitmask;
      attributesBck = attributes;
      verticalCharsBck = verticalChars;
      return 1;
    }
  return 0;
}
bool ardvga::setBox(uint8_t y, uint8_t height){
  if ((y < 0) || ((y > verticalCharsBck - 1))) return 1;
  if ((height < 0) || ((height > (verticalCharsBck - y)))) return 1;
  bitmask = bitmaskBck + (y * horizontalChars * 8);
  attributes = attributesBck + (y * horizontalChars);
  verticalChars = height;
  return 0;
}
void ardvga::resetBox(){
  bitmask = bitmaskBck;
  attributes = attributesBck;
  verticalChars = verticalCharsBck;
}
void ardvga::paper(uint8_t p){
  Paper = p;
}
void ardvga::ink(uint8_t i){
  Ink = i;
}
void ardvga::bPaper(uint8_t pb){
  PaperBright = pb;
}
void ardvga::bInk(uint8_t ib){
  InkBright = ib;
}
uint8_t ardvga::getHChars(){
  return horizontalChars;
}
uint8_t ardvga::getVChars(){
  return verticalChars;
}
uint16_t ardvga::getHPixels(){
  return (horizontalChars * 8);
}
uint16_t ardvga::getVPixels(){
  return (verticalChars * 8);
}
ISR (TIMER2_OVF_vect){

  switch (ardvga::mode){
    case _720:
      switch(ardvga::scanLine){
      case 1:
        V_SYNC_PORT |= VSYNC_PIN_UP_MASK;
        break;
      case 3:
        V_SYNC_PORT &= VSYNC_PIN_DOWN_MASK;
        break;
      case 36: //Standard back porch in lines
        ardvga::doLine = 1;
        break;
      case 449:
        ardvga::scanLine = 0;
      }
      break;
    case _640:
      switch(ardvga::scanLine){
      case 1:
        V_SYNC_PORT |= VSYNC_PIN_UP_MASK;
        break;
      case 3:
        V_SYNC_PORT &= VSYNC_PIN_DOWN_MASK;
        break;
      case 35: //Standard back porch in lines
        ardvga::doLine = 1;
        break;
      case 525:
        ardvga::scanLine = 0;
      }
  }
  sei();
  sleep_mode ();
}

#if defined(__AVR_ATmega168__) || defined(__AVR_ATmega168P__) || defined(__AVR_ATmega328P__)

ISR (TIMER2_COMPB_vect){
if (ardvga::doLine && ((ardvga::skipLine && (ardvga::scanLine & 1)) || !(ardvga::skipLine))){
    uint8_t i = ardvga::horizontalChars;
    uint8_t j = ardvga::drawLine / 4; //este cuatro se puede calcular en función de verticalCharsBck
    uint8_t * attrPtr = ardvga::attributesBck + ((j/8) * i);
    uint8_t * bmskPtr = ardvga::bitmaskBck + (j * i);
    uint8_t aux = VGA_ATTRIBUTE_B_PIN;
    pixel_ton();
    nop();
    while (i--)
    {
      uint8_t k = *(attrPtr);
      j = (k & VGA_ATTRIBUTE_B_MASK);
      PIXEL_DR = *(bmskPtr);
      j |= aux;
      nop();
      VGA_ATTRIBUTE_B_PORT = j;
      VGA_ATTRIBUTE_PORT = k;
      attrPtr++;
      bmskPtr++;
    }
    i = 1 + ardvga::skipLine;
    PIXEL_DR = 0;
    pixel_toff();
    VGA_ATTRIBUTE_B_PORT &= ~VGA_ATTRIBUTE_B_MASK;
    VGA_ATTRIBUTE_PORT = BLANK;
    ardvga::drawLine += i;
  }
  ardvga::lineCounter++;
  if (ardvga::scanLine++ == ardvga::vFrontPorch){
    ardvga::doLine = 0;
    ardvga::drawLine = 0;
  }
  if (ardvga::sndDur){
    ardvga::sndDur--;
    if(ardvga::hLine++ > ardvga::sndFreq)
      ardvga::hLine=0;
    else
      if (ardvga::hLine > (ardvga::sndFreq / 2)) // (4/5) es el volumen
        soundon();
      else
        soundoff();
  }
  else{
    //gestionar buffer de sonido
  }

}


  void ardvga::setupIO() {

    V_SYNC_CR |= (1 << VSYNC_PIN);
    H_SYNC_CR |= (1 << HSYNC_PIN);
    VGA_PIXEL_CR |= (1 << VGA_PIXEL_PIN) | (1 << VGA_CLOCK_PIN);
    VGA_ATTRIBUTE_CR = B11111111;
    VGA_ATTRIBUTE_B_CR |= VGA_ATTRIBUTE_B_MASK;
    SOUNDPORTCR |= (1<<SOUNDPIN);
    soundoff();
  }
  void ardvga::setCRTCTRL(uint8_t ht){

    TIMSK0 = 0;
    TCCR2A = (1 << COM2B1) | (1 << WGM20) | (1 << WGM21);
    TCCR2B = (1 << WGM22) | (1 << CS21);
    OCR2A = ht;
    OCR2B = ((ht+1)/8)-1;
    TIMSK2 = (1 << TOIE2)  | (1 << OCIE2B);
    TIFR2  = (1 << TOV2)   | (1 << OCF2B);
    UBRR0 = 0;  // USART Baud Rate Register
    UCSR0B = 0;
    UCSR0C = (1 << UMSEL00) | (1 << UMSEL01) | (1 << UCPHA0) & ~(1 << UCPOL0);  // Master SPI mode
    set_sleep_mode (SLEEP_MODE_EXT_STANDBY);

  }
  void ardvga::loadzxscr(const uint8_t scrBitmaps[] PROGMEM , const uint8_t scrAttributes [] PROGMEM ,uint16_t offset) {
    uint16_t j , k = 0 , m , n;
    uint8_t l , c , hb , lb , paper , ink , bright , val , i;
    while (k < aMemSize) {
      l = (k / ardvga::horizontalChars) + ((offset / 8) / 32);// hay que cambiar 32 por el valor de ancho de la estructura bitmap
      c = (k % ardvga::horizontalChars) + (offset % 32);// hay que cambiar 32 por el valor de ancho de la estructura bitmap
      m = l * 32 + c ;// hay que cambiar 32 por el valor de ancho de la estructura bitmap
      val = safeReadFlashByte(scrAttributes , m);
      bright = (val & B01000000) / 64;
      if (bright) bright = B00000011;
      paper = (val & B00111000) * 4;
      ink = (val & B00000111) * 4;
      if (paper == 0) bright &= B00000001;
      if (ink == 0) bright &= B00000010;
      *(attributes + k) = paper | ink | bright;
      i = 0;
      j = (k % ardvga::horizontalChars);
      j += (k - j) * 8 ;// hay que cambiar 8 por el valor de alto del bloque unitario de atributos
      k++;
      while (i < 8) {// hay que cambiar 8 por el valor de alto del bloque unitario de atributos
        n = j + (i * ardvga::horizontalChars);
        l = (n / ardvga::horizontalChars) + (offset / 32);// hay que cambiar 32 por el valor de ancho de la estructura bitmap
        hb = l & B00000111;
        hb = hb | ((l / 8) & B00011000);
        lb = c | ((l * 4) & B11100000);
        m = hb;
        m = m * 256;
        m = m | lb;
        *(bitmask + n) = safeReadFlashByte(scrBitmaps , m);
        i++;
      }
    }
  }
  void ardvga::loadbitmap(const uint8_t scrBitmaps[] PROGMEM , const uint8_t scrAttributes [] PROGMEM , uint16_t offset) {
    uint16_t k = 0;
    uint8_t l;
    uint8_t c;
    uint16_t m;
    while (k < vMemSize) {
      l = k / ardvga::horizontalChars;
      c = k % ardvga::horizontalChars;
      m = l * 16 + c ; // hay que cambiar 16 por el valor de ancho de la estructura bitmap
      *(bitmask + k++)=safeReadFlashByte(scrBitmaps , m);
    }
    k = 0;
    uint8_t paper;
    uint8_t ink;
    uint8_t bright;
    uint8_t val;
    while (k < aMemSize) {
      l = k / ardvga::horizontalChars;
      c = k % ardvga::horizontalChars;
      m = l * 16 + c ;// hay que cambiar 16 por el valor de ancho de la estructura bitmap
      val = safeReadFlashByte(scrAttributes , m);
      bright = (val & B01000000) >> 6;
      if (bright) bright = B00000011;
      paper = (val & B00111000) << 2;
      ink = (val & B00000111) << 2;
      if (paper == 0) bright &= B00000001;
      if (ink == 0) bright &= B00000010;
      *(attributes + k++) = paper | ink | bright;
    }
  }
  void ardvga::ssa() {
    memset(attributes, PaperBright | InkBright | Paper | Ink, aMemSize);
  }
  bool ardvga::setattr(uint8_t line, uint8_t column, uint8_t paper, uint8_t ink, uint8_t paperbright, uint8_t inkbright){
    if ((column < 0) || (column > horizontalChars - 1) || (line < 0) || (line > verticalChars - 1)) return 1;
    //*(attributes + (line * horizontalChars) + column) = ((((inkbright)&1)) | (((paperbright)&1) << 1 ) | (paper << 5) | (ink << 2));
    //attributes[line][column] = ((((inkbright)&1) << 7) | (((paperbright)&1) << 6) | (paper << 3) | (ink));
    //*(attributes + (line * horizontalChars) + column) = ((((inkbright)&1) << 7) | (((paperbright)&1) << 6) | (paper << 3) | (ink));
    *(attributes + (line * horizontalChars) + column) = (paperbright | inkbright | paper | ink) ;
    return 0;
  }
  void ardvga::scrollau( uint8_t i) {
    if (i > verticalChars) i = verticalChars;
    memmove(attributes, attributes + (i * ardvga::horizontalChars), aMemSize - ardvga::horizontalChars * i);
    memset(attributes + aMemSize - ardvga::horizontalChars * i + 1, PaperBright | InkBright | Paper | Ink, ardvga::horizontalChars * i);
  }
#endif

/*#if defined(__AVR_ATmega644__) || defined(__AVR_ATmega644P__) || defined(__AVR_ATmega1284P__)

  void ardvga::setupIO() {

    VGA_SYNC_CR |= (1 << VSYNC_PIN) | (1 << HSYNC_PIN);
    VGA_PIXEL_CR |= (1 << VGA_PIXEL_PIN) | (1 << VGA_CLOCK_PIN);
    VGA_ATTRIBUTE_CR = B11111111;
    SOUNDPORTCR |= (1<<SOUNDPIN);
    soundoff();
  }

  void ardvga::setCRTCTRL(uint8_t ht){
    cli();
    TIMSK0 = 0;
    TIMSK2 = 0;
    TCCR2A = (1 << COM2B1) | (1 << WGM20) | (1 << WGM21);
    TCCR2B = (1 << WGM22) | (1 << CS21);
    OCR2A = HT-1;//(P_8=107)(P_32=27)(P_64=13)
    OCR2B = HPT-1;//(P_8=13)(P_32=3)(P_64=2)
    TIMSK2 = bit(TOIE2)  | bit(OCIE2B);
    TIFR2  = bit(TOV2)   | bit(OCF2B);
    UBRR1 = 0;  // USART Baud Rate Register   //XXX
    UCSR1B = 0;
    UCSR1C = bit (UMSEL10) | bit (UMSEL11) | bit (UCPHA1) | bit (UCPOL1);  // Master SPI mode   //XXX
    sei();
}


    void ardvga::lineProc_skipLine()
    {
      if (scanLine++ & 1){
        if (doLine){
           uint8_t * attrPtr = & (attributes [drawLine / 8] [0] );
           uint8_t * bmskPtr = & (bitmask [drawLine ] [0] );
           uint8_t i = horizontalChars;
          pixel_ton();
          nop();
          while (i--)
          {
             uint8_t k = *(attrPtr);
             uint8_t l = *(bmskPtr);
            PIXEL_DR = l;
            VGA_ATTRIBUTE_PORT = k;
            bmskPtr++;
            attrPtr++;
            waitsendbyte();
          }
           uint16_t dl = drawLine;
          dl++;
          nop();
          pixel_toff();
          VGA_ATTRIBUTE_PORT = BLANK;
          drawLine=dl;
        }
      }
      lineCounter++;
      if (sndDur--){
        if(hLine++ > sndFreq)
          hLine=0;
        else
          if (hLine > (sndFreq / 2))
            soundon();
          else
            soundoff();
      }
      else soundoff();
    }

    void ardvga::lineProc_noskipLine()
    {
        if (doLine){
           uint8_t * attrPtr = & (attributes [drawLine / 16] [0] );
           uint8_t * bmskPtr = & (bitmask [drawLine / 2] [0] );
           uint8_t i = horizontalChars;
          pixel_ton();
          while (i--)
          {
             uint8_t k = *(attrPtr);
             uint8_t l = *(bmskPtr);
            PIXEL_DR = l;
            VGA_ATTRIBUTE_PORT = k;
            bmskPtr++;
            attrPtr++;
            waitsendbyte();
          }
           uint16_t dl = drawLine;
          dl++;
          nop();
          pixel_toff();
          VGA_ATTRIBUTE_PORT = BLANK;
          drawLine=dl;
        }
        scanLine++;
        lineCounter++;
        if (sndDur--){
          if(hLine++ > sndFreq)
            hLine=0;
          else
            if (hLine > (sndFreq / 2))
              soundon();
            else
              soundoff();
        }
        else soundoff();
    }

    void ardvga::loadzxscr(const uint8_t scrBitmaps[] PROGMEM , const uint8_t scrAttributes [] PROGMEM ) {
      Ink = Black; Paper = Black; PaperBright=0,InkBright=0; ssa();
      uint16_t k=0;
      uint8_t val = 0;
      uint8_t l;
      uint8_t c;
      uint16_t m;
      uint8_t hb,lb;
      while (k < vMemSize) {
        l=k/horizontalChars;//l=k>>5;//
        c=k%horizontalChars;//c=k&31;//
        hb=l&B00000111;
        hb=hb|((l>>3)&B00011000);
        lb=c|((l<<2)&B11100000);
        m=hb;
        m=m<<8;
        m=m|lb;
        //while (val != pgm_read_byte_near(scrBitmaps + m)) val = pgm_read_byte_near(scrBitmaps + m);
        val=safeReadFlashByte(scrBitmaps,m);
        bitmask[l][c] = val;
        k++;
      }
      k = 0;
      uint8_t paper = 0;
      uint8_t ink = 0;
      uint8_t bright = 0;
      while (k < aMemSize) {
        //while (val != pgm_read_byte_near(scrAttributes + k)) val = pgm_read_byte_near(scrAttributes + k);
        val=safeReadFlashByte(scrAttributes,k);
        if ((val & B01000000) > 0) {
          val |= B11000000;
        }
        else {
          val &= B00111111;
        }
        paper = val & B00111000;
        ink = val & B00000111;
        if (paper == 0) val &= B10111111;
        if (ink == 0) val &= B01111111;
        attributes[k / horizontalChars][k % horizontalChars] =  val;
        k++;
      }
    }
    void ardvga::ssa() {
      memset(attributes, ((((InkBright)&1) << 7) | (((PaperBright)&1) << 6) | (Paper << 3) | (Ink)), aMemSize);
    }

    bool ardvga::setattr(uint8_t line, uint8_t column, uint8_t paper, uint8_t ink, uint8_t paperbright, uint8_t inkbright){
      if ((column < 0) || ((column > horizontalChars - 1))) return 1;
      if ((line < 0) || ((line > verticalChars - 1))) return 1;
      attributes[line][column] = ((((inkbright)&1) << 7) | (((paperbright)&1) << 6) | (paper << 3) | (ink));
      return 0;
    }

    void ardvga::scrollau( uint8_t i) {
      if (i > verticalChars) i = verticalChars;
      memmove(&attributes[0][0], &attributes[i][0], aMemSize - horizontalChars * i);
      memset(&attributes[verticalChars - 1 * i][0], ((((InkBright)&1) << 7) | (((PaperBright)&1) << 6) | (Paper << 3) | (Ink)), horizontalChars * i);
    }

#endif*/
void ardvga::cls() {
  ardvga::clb();
  ardvga::ssa();
}
void ardvga::clb() {
  memset(bitmask , 0 , vMemSize);
}
void ardvga::scrollbu( uint16_t i) {
  if (i > verticalPixels) i = verticalPixels;
  memmove(bitmask , bitmask + (i * horizontalChars) , vMemSize - (horizontalChars * i));
  memset(bitmask  + vMemSize - (horizontalChars * i) , 0 , (horizontalChars * i));
}
void ardvga::scrolltext( uint8_t i) {
  if (i > ardvga::verticalChars) i = ardvga::verticalChars;
  scrollbu(8 * i);
  scrollau(i);
}
bool ardvga::putStr (char str[], size_t strSize,  uint8_t line,  uint8_t column, uint8_t paper, uint8_t ink, uint8_t paperbright, uint8_t inkbright) {//hacer una igual para posicionar al pixel, hacer un tipo estructura para atributos

  for ( uint16_t i = 0; i < (strSize-1); i++) {
    if (str[i] == 0x0D) { //CR
       column=0 ;
       continue;
    }
    if (str[i] == 0x0A){ //CR+LF
       line++ ;
       column=0;
       continue;
    }
    /*if (str[i] == 0x0){ //null
       continue;
    }*/
    if (column > (horizontalChars - 1)) {
        column = 0;
        line++;
    }
    if (line > (verticalChars - 1)) {
        line--;
        scrolltext(1);
    }
    if (putChar(str[i], line, column)) return 1;
    setattr(line, column, paper, ink, paperbright, inkbright);
    column++;
  }
  xPos = column;
  yPos = line;
  return 0;
}
bool ardvga::putChar (uint8_t c,  uint8_t line,  uint8_t column){//hacerla para fuente generica y para alinear a pixel o a atributo
  if ((column < 0) || (column > horizontalChars - 1) || (line < 0) || (line > verticalChars - 1)) return 1;
  uint8_t aux;// k;
  //register uint8_t *ptr = (bitmask + horizontalChars * ((line * 8) + i) + column);
  register uint8_t *ptr =  bitmask + horizontalChars * line * 8 + column;
  for ( uint8_t i = 0; i < 8; i++) {
    //k = c + (i << 8);//cp437 Charset
    //k = ((c-32) << 3) + i;//ZX Charset
    //while (aux != pgm_read_byte_near(&screen_font[c][i])) aux = pgm_read_byte_near(&screen_font[c][i]);
    aux = safeReadFlashByte (screen_font[c],i);//crear fuente con Fony (compatible con Marlin) para evitar espejar
    aux = ((aux / 2) & 0x55) | ((aux * 2) & 0xaa);
    aux = ((aux / 4) & 0x33) | ((aux * 4) & 0xcc);
    aux = ((aux / 16) & 0x0f) | ((aux * 16) & 0xf0);
    *(ptr + horizontalChars * i) = aux; //~cp437
  }
  return 0;
}
bool ardvga::print(char* buf){
  uint8_t length=0;
  while (buf[length]) length++;
  return putStr (buf, ++length, yPos, xPos, Paper, Ink, PaperBright, InkBright);
}
bool ardvga::setCursor(uint8_t x, uint8_t y){
  if ((x < 0) || (x > horizontalChars - 1) || (y < 0) || (y > verticalChars - 1)) return 1;
  xPos = x;
  yPos = y;
}
uint8_t ardvga::getXPos(){
  return xPos;
}
uint8_t ardvga::getYPos(){
  return yPos;
}
uint16_t ardvga::freeRam () {
  extern uint16_t __heap_start, *__brkval;
  uint16_t v;
  return (uint16_t) &v - (__brkval == 0 ? (uint16_t) &__heap_start : (uint16_t) __brkval);
}
uint8_t ardvga::safeReadFlashByte (const uint8_t pgmArray[] , uint16_t index){
  register uint8_t aux=0;
  while ( aux != pgm_read_byte_near (pgmArray + index)){
    aux = pgm_read_byte_near (pgmArray + index);
  }
  return aux;
}
uint16_t ardvga::safeReadFlashWord (const uint16_t pgmArray[] , uint16_t index){
  uint16_t aux=0;
  while ( aux != pgm_read_word_near (pgmArray + index))
    aux = pgm_read_word_near (pgmArray + index);
  return aux;
}
void ardvga::delay(uint32_t i) {
  uint32_t med = lineCounter ; // implementar overflow flag o subir a uint64_t
  //while ((lineCounter- med) < i);
  while (((lineCounter- med) * 1000) < (i * hFreq(hT)));
}
uint32_t ardvga::countLines(){
  return ardvga::lineCounter;
}
uint8_t ardvga::vFreq(uint8_t ht){
  if (ardvga::mode == _640) return (ardvga::hFreq(ht)/525);
  if (ardvga::mode == _720) return (ardvga::hFreq(ht)/449);
}
uint16_t ardvga::hFreq(uint8_t ht){
  return ((F_CPU / 8) / (ht + 1));
}
bool ardvga::plot( uint16_t x ,  uint16_t y){
  if ((x < 0) || (x > horizontalTop) || (y < 0) || (y > verticalTop)) return 1;
  register uint8_t *ptr = (bitmask + (horizontalChars * y) + (x / 8));
  #ifdef __AVR__ //From Adafruit_GFX
    // Bitmask tables of 0x80>>X and ~(0x80>>X), because X>>Y is slow on AVR
    static const uint8_t PROGMEM GFXsetBit[] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
    *ptr |= safeReadFlashByte(GFXsetBit,(x & 7));
    /*switch (x & 7) {
      case 0:
        *ptr |= 0x80;
        break;
      case 1:
        *ptr |= 0x40;
        break;
      case 2:
        *ptr |= 0x20;
        break;
      case 3:
        *ptr |= 0x10;
        break;
      case 4:
        *ptr |= 0x08;
        break;
      case 5:
        *ptr |= 0x04;
        break;
      case 6:
        *ptr |= 0x02;
        break;
      case 7:
        *ptr |= 0x01;
    }*/
  #else
    *ptr |= 128 >> (x & 7);
  #endif
  return 0;
}
bool ardvga::cplot( uint16_t x ,  uint16_t y) {
  if ((x < 0) || (x > horizontalTop) || (y < 0) || (y > verticalTop)) return 1;
  register uint8_t *ptr = (bitmask + (horizontalChars * y) + (x / 8));
  #ifdef __AVR__ //From Adafruit_GFX
      // Bitmask tables of 0x80>>X and ~(0x80>>X), because X>>Y is slow on AVR
      static const uint8_t PROGMEM GFXclrBit[] = { 0x7F, 0xBF, 0xDF, 0xEF, 0xF7, 0xFB, 0xFD, 0xFE };
      *ptr &= safeReadFlashByte(GFXclrBit,(x & 7));
    /*switch (x & 7) {
      case 0:
        *ptr &= 0x7F;
        break;
      case 1:
        *ptr &= 0xBF;
        break;
      case 2:
        *ptr &= 0xDF;
        break;
      case 3:
        *ptr &= 0xEF;
        break;
      case 4:
        *ptr &= 0xF7;
        break;
      case 5:
        *ptr &= 0xFB;
        break;
      case 6:
        *ptr &= 0xFD;
        break;
      case 7:
        *ptr &= 0xFE;
    }*/
  #else
    *ptr &= ~(128 >> (x & 7));
  #endif
  return 0;
}
//http://members.chello.at/~easyfilter/bresenham.html
void ardvga::line( int16_t x0,  int16_t y0,  int16_t x1,  int16_t y1){
  /*if ((x0 < 0) || ((x0 > horizontalTop))) return 1;
  if ((y0 < 0) || ((y0 > verticalTop))) return 1;
  if ((x1 < 0) || ((x1 > horizontalTop))) return 1;
  if ((y1 < 0) || ((y1 > verticalTop))) return 1;*/
  int16_t dx =  abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int16_t dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int16_t err = dx + dy, e2; /* error value e_xy */
  for (;;) { /* loop */
    plot(x0 , y0);
    if (x0 == x1 && y0 == y1) break;
    e2 = err << 1;
    if (e2 >= dy) {
      err += dy;  /* e_xy+e_x > 0 */
      x0 += sx;
    }
    if (e2 <= dx) {
      err += dx;  /* e_xy+e_y < 0 */
      y0 += sy;
    }
  }
}
void ardvga::cline( int16_t x0,  int16_t y0,  int16_t x1,  int16_t y1){
  /*if ((x0 < 0) || ((x0 > horizontalTop))) return 1;
  if ((y0 < 0) || ((y0 > verticalTop))) return 1;
  if ((x1 < 0) || ((x1 > horizontalTop))) return 1;
  if ((y1 < 0) || ((y1 > verticalTop))) return 1;*/
  int16_t dx =  abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
  int16_t dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
  int16_t err = dx + dy, e2; /* error value e_xy */
  for (;;) { /* loop */
    cplot(x0 , y0);
    if (x0 == x1 && y0 == y1) break;
    e2 = err << 1;
    if (e2 >= dy) {
      err += dy;  /* e_xy+e_x > 0 */
      x0 += sx;
    }
    if (e2 <= dx) {
      err += dx;  /* e_xy+e_y < 0 */
      y0 += sy;
    }
  }
}
void ardvga::plotCircle(int16_t xm, int16_t ym, int16_t r){
  int16_t x = -r, y = 0, err = 2-2*r;
  do {
    plot(xm-x, ym+y); /*   I. Quadrant */
    plot(xm-y, ym-x); /*  II. Quadrant */
    plot(xm+x, ym-y); /* III. Quadrant */
    plot(xm+y, ym+x); /*  IV. Quadrant */
    r = err;
    if (r <= y) err += ++y*2+1;           /* e_xy+e_y < 0 */
    if (r > x || err > y) err += ++x*2+1; /* e_xy+e_x > 0 or no 2nd y-step */
  } while (x < 0);
}
void ardvga::clearCircle(int16_t xm, int16_t ym, int16_t r){
  int16_t x = -r, y = 0, err = 2-2*r;
  do {
    cplot(xm-x, ym+y); /*   I. Quadrant */
    cplot(xm-y, ym-x); /*  II. Quadrant */
    cplot(xm+x, ym-y); /* III. Quadrant */
    cplot(xm+y, ym+x); /*  IV. Quadrant */
    r = err;
    if (r <= y) err += ++y*2+1;           /* e_xy+e_y < 0 */
    if (r > x || err > y) err += ++x*2+1; /* e_xy+e_x > 0 or no 2nd y-step */
  } while (x < 0);
}
void ardvga::plotEllipseRect(int16_t x0, int16_t y0, int16_t x1, int16_t y1){
   int32_t a = abs(x1-x0), b = abs(y1-y0), b1 = b&1; /* values of diameter */
   int32_t dx = 4*(1-a)*b*b, dy = 4*(b1+1)*a*a; /* error increment */
   int32_t err = dx+dy+b1*a*a, e2; /* error of 1.step */

   if (x0 > x1) { x0 = x1; x1 += a; } /* if called with swapped points */
   if (y0 > y1) y0 = y1; /* .. exchange them */
   y0 += (b+1)/2; y1 = y0-b1;   /* starting pixel */
   a *= 8*a; b1 = 8*b*b;

   do {
       plot(x1, y0); /*   I. Quadrant */
       plot(x0, y0); /*  II. Quadrant */
       plot(x0, y1); /* III. Quadrant */
       plot(x1, y1); /*  IV. Quadrant */
       e2 = 2*err;
       if (e2 <= dy) { y0++; y1--; err += dy += a; }  /* y step */
       if (e2 >= dx || 2*err > dy) { x0++; x1--; err += dx += b1; } /* x step */
   } while (x0 <= x1);

   while (y0-y1 < b) {  /* too early stop of flat ellipses a=1 */
       plot(x0-1, y0); /* -> finish tip of ellipse */
       plot(x1+1, y0++);
       plot(x0-1, y1);
       plot(x1+1, y1--);
   }
}
void ardvga::clearEllipseRect(int16_t x0, int16_t y0, int16_t x1, int16_t y1){
   int32_t a = abs(x1-x0), b = abs(y1-y0), b1 = b&1; /* values of diameter */
   int32_t dx = 4*(1-a)*b*b, dy = 4*(b1+1)*a*a; /* error increment */
   int32_t err = dx+dy+b1*a*a, e2; /* error of 1.step */

   if (x0 > x1) { x0 = x1; x1 += a; } /* if called with swapped points */
   if (y0 > y1) y0 = y1; /* .. exchange them */
   y0 += (b+1)/2; y1 = y0-b1;   /* starting pixel */
   a *= 8*a; b1 = 8*b*b;

   do {
       cplot(x1, y0); /*   I. Quadrant */
       cplot(x0, y0); /*  II. Quadrant */
       cplot(x0, y1); /* III. Quadrant */
       cplot(x1, y1); /*  IV. Quadrant */
       e2 = 2*err;
       if (e2 <= dy) { y0++; y1--; err += dy += a; }  /* y step */
       if (e2 >= dx || 2*err > dy) { x0++; x1--; err += dx += b1; } /* x step */
   } while (x0 <= x1);

   while (y0-y1 < b) {  /* too early stop of flat ellipses a=1 */
       cplot(x0-1, y0); /* -> finish tip of ellipse */
       cplot(x1+1, y0++);
       cplot(x0-1, y1);
       cplot(x1+1, y1--);
   }
}
void ardvga::tone (uint16_t frequency,uint32_t duration){
  if ((frequency > hFreq(hT)) || (frequency < 0)) return;
  //sndDur=duration ;
  sndDur = (duration * hFreq(hT)) / 1000;
  sndFreq = hFreq(hT) / frequency;
  hLine = 0;
}
uint8_t ardvga::isDoingLine(){
  //return (ardvga::doLine && ((ardvga::skipLine && (ardvga::scanLine & 1)) || !(ardvga::skipLine)));
  return ardvga::doLine;
}
