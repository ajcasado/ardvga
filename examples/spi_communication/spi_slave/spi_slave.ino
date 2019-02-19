#include <ardvga.h>

#define PIN_SS 10 //PB2
#define GET_SS_VALUE() (PINB & (1<<PB2)) /*digitalRead(PIN_SS)*/// 0 for LOW or > 0 for HIGH
#define PIN_MOSI 11
#define PIN_MISO 12
#define PIN_SCK 13
#define PIN_RTR 9 //PB1
#define SET_RTR_HIGH() PORTB |= (1 << PB1) // digitalWrite(PIN_SS , 1);
#define SET_RTR_LOW() PORTB &= ~(1 << PB1) // digitalWrite(PIN_SS , 0);

#define MAX_BUFLEN 16
#define START_TIMER() TCNT1 = 0 ; TCCR1B |= (1 << CS12) | (1 << CS10)   // enable timer with 1024 prescaler, clear counter.
#define STOP_TIMER() TCCR1B &= ~(1 << CS12) & ~(1 << CS10) // disable timer.

typedef enum states {ocioso, puedo_recibir, recibo_byte, proceso_mensaje, buffer_overrun, timeout} state_t;

volatile state_t state;
char buff[MAX_BUFLEN] = {0};
char buf[24] = {0};
uint8_t buflen , i;

ardvga mivga;

void setup(){
  mivga.begin(12, 16, 0);
  mivga.setSkipLine();
  mivga.ink(inkGreen); mivga.paper(paperGreen); mivga.bPaper(noBright); mivga.bInk(brightInk);
  mivga.cls();
  sprintf_P(buf, PSTR("<<<SLAVE TEST>>>\n\0"));
  mivga.print(buf);
  mivga.ink(inkBlue); mivga.paper(paperWhite); mivga.bPaper(brightPaper); mivga.bInk(noBright);
  pinMode (PIN_RTR , OUTPUT);
  /* Set MISO output, all others input */
  pinMode (PIN_MISO , OUTPUT);
  digitalWrite(PIN_RTR,1);
  digitalWrite(PIN_MISO,1);
  pinMode (PIN_MOSI , INPUT);
  pinMode (PIN_SCK , INPUT);
  pinMode (PIN_SS , INPUT);
  /* Enable SPI in Slave mode, mode 3 , MSBFIRST  (clock speed driven by master)*/
  SPCR = (1<<SPE) | (1<<CPOL) | (1<<CPHA);
  //set timer1 interrupt at 1Hz -amandaghassaei https://www.instructables.com/id/Arduino-Timer-Interrupts/)
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TIMSK1 = (1 << OCIE1A);// enable timer compare interrupt
  // set compare match register for 1hz increments
  OCR1A = (F_CPU / 1024) -1;//15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);//CTC mode
  TIFR1 |= (1<<OCF1A); //clear interrupt flag
  state = ocioso;
}

void loop(){
  switch (state) {
  case ocioso:
    STOP_TIMER();
    SET_RTR_HIGH();
    buflen = SPSR; //Clear SPIF
    buflen = SPDR; //Clear SPIF (datasheet)
    buflen = 0; // init buflen
    memset(buff, 0, MAX_BUFLEN);
    while (GET_SS_VALUE() > 0);
  case puedo_recibir:
    SET_RTR_LOW();
    START_TIMER();
    while ((!(SPSR & (1<<SPIF))) && (state != timeout));
    if (state == timeout) break;
  case recibo_byte://este estado no se usa -> quitar
    SET_RTR_HIGH();
    buff[buflen++] = SPDR;
    if (buflen > MAX_BUFLEN){
      state = buffer_overrun;
      break;
    }
    if (GET_SS_VALUE() == 0){
      state = puedo_recibir;
      break;
    }
  case proceso_mensaje://este estado no se usa -> quitar
    buff[buflen] = 0;
    sprintf_P(buf, PSTR("Received message\n\0"));
    mivga.print(buf);
    mivga.print(buff);
    sprintf_P(buf , PSTR("Received bytes:%u\n\0") , buflen);
    mivga.print(buf);
    state = ocioso;
    break;
  case buffer_overrun:
    sprintf_P(buf, PSTR("BUFFER OVERRUN!\n\0"));
    mivga.print(buf);
    sprintf_P(buf, PSTR("%u\n\0"),buflen);
    mivga.print(buf);
    state = ocioso;
    break;
  case timeout:
    sprintf_P(buf, PSTR("TIMEOUT!\n\0"));
    mivga.print(buf);
    sprintf_P(buf, PSTR("%u\n\0"),SPDR);
    mivga.print(buf);
    state = ocioso;
  }
}

ISR (TIMER1_COMPA_vect /*, ISR_NOBLOCK*/){
  state = timeout;
}
