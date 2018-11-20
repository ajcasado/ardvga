#include <ardvga.h>

#define PIN_SS 10 //PB2
#define GET_SS_VALUE() (PINB & (1<<PB2)) /*digitalRead(PIN_SS)*/// 0 for LOW or > 0 for HIGH
#define PIN_MOSI 11
#define PIN_MISO 12
#define PIN_SCK 13
#define PIN_RTR 9 //PB1
#define SET_RTR_HIGH() PORTD |= (1 << PB1) // digitalWrite(PIN_SS , 1);
#define SET_RTR_LOW() PORTD &= ~(1 << PB1) // digitalWrite(PIN_SS , 0);

#define MAX_BUFLEN 16
#define START_TIMER() TIMSK1 |= (1 << OCIE1A) ; TCNT1 = 0  // enable timer compare interrupt, clear counter.
#define STOP_TIMER() TIMSK1 &= ~(1 << OCIE1A)

typedef enum states {ocioso, puedo_recibir, recibo_byte, proceso_mensaje, buffer_overrun, timeout} state_t;

volatile state_t state;
char buff[MAX_BUFLEN] = {0};
char buf[24] = {0};

ardvga mivga;

void setup(){
  mivga.begin(12, 14, 0);
  mivga.ink(inkGreen); mivga.paper(paperGreen); mivga.bPaper(noBright); mivga.bInk(brightInk);
  sprintf_P(buf, PSTR("__SLAVE TEST__\n\0"));
  mivga.print(buf);
  mivga.ink(inkBlue); mivga.paper(paperWhite); mivga.bPaper(brightPaper); mivga.bInk(noBright);
  state = ocioso;
  pinMode (PIN_RTR , OUTPUT);
  /* Set MISO output, all others input */
  pinMode (PIN_MISO , OUTPUT);
  pinMode (PIN_MOSI , INPUT_PULLUP);
  pinMode (PIN_SCK , INPUT_PULLUP);
  pinMode (PIN_SS , INPUT_PULLUP);
  /* Enable SPI in Slave mode, mode 3 , MSBFIRST  (clock speed driven by master)*/
  SPCR = (1<<SPE) | (1<<CPOL) | (1<<CPHA);
  //set timer1 interrupt at 1Hz -amandaghassaei https://www.instructables.com/id/Arduino-Timer-Interrupts/)
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 15624;// = (16*10^6) / (1*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);
}

void loop(){
  uint8_t buflen , i;
  switch (state) {
  case ocioso:
    STOP_TIMER();
    SET_RTR_HIGH();
    buflen = 0;
    memset(buff, 0, MAX_BUFLEN);
    while (GET_SS_VALUE() > 0);
    state = puedo_recibir;
  case puedo_recibir:
    SET_RTR_LOW();
    START_TIMER();
    while ((SPIF == 0) && (state != timeout));
    if (state == timeout) break;
    state = recibo_byte;
  case recibo_byte:
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
    state = proceso_mensaje;
  case proceso_mensaje:
    buff[buflen] = 0;
    sprintf_P(buf, PSTR("Received message:\n\0"));
    mivga.print(buf);
    mivga.print(buff);
    sprintf_P(buf , PSTR("\nReceived bytes: %u\n\0") , buflen);
    mivga.print(buf);
    state = ocioso;
    break;
  case buffer_overrun:
    sprintf_P(buf, PSTR("BUFFER OVERRUN!\n\0"));
    mivga.print(buf);
    state = ocioso;
    break;
  case timeout:
    sprintf_P(buf, PSTR("TIMEOUT!\n\0"));
    mivga.print(buf);
    state = ocioso;
  }
}

ISR (TIMER1_COMPA_vect , ISR_NOBLOCK){
  state = timeout;
}
