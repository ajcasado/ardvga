#include <ardvga.h>

#define PIN_SS 10
#define PIN_MOSI 11
#define PIN_MISO 12
#define PIN_SCK 13
#define PIN_RTR 4
#define MAX_BUFLEN 16
#define START_TIMER()
#define STOP_TIMER()

typedef enum states {ocioso, puedo_recibir, recibo_byte, proceso_mensaje, buffer_overrun, timeout} state_t;

state_t state;
char buff[MAX_BUFLEN] = {0};
char buf[17] = {0};

ardvga mivga;

void setup(){
  mivga.begin(12, 12, 0);
  mivga.ink(Cyan); mivga.paper(Blue); mivga.bPaper(0); mivga.bInk(1);
  sprintf_P(buf, PSTR("SLAVE TEST\n\0"));
  mivga.print(buf);
  state = ocioso;
  pinMode (PIN_RTR , OUTPUT);
  /* Set MISO output, all others input */
  pinMode (PIN_MISO , OUTPUT);
  pinMode (PIN_MOSI , INPUT);
  pinMode (PIN_SCK , INPUT);
  pinMode (PIN_SS , INPUT_PULLUP);
  /* Enable SPI in Slave mode, mode 0 , 4MHz , MSBFIRST*/
  SPCR = (1<<SPE);
}

void loop(){
  uint8_t buflen , i;
  switch (state) {
  case ocioso:
    STOP_TIMER();
    digitalWrite(PIN_RTR , 0);
    buflen = 0;
    memset(buffer, 0, MAX_BUFLEN);
    while (digitalRead(PIN_SS));
    state = puedo_recibir;
  case puedo_recibir:
    digitalWrite(PIN_RTR , 1);
    START_TIMER();
    while (SPIF == 0);
    state = recibo_byte;
  case recibo_byte:
    digitalWrite(PIN_RTR , 0);
    buff[buflen++] = SPDR;
    if (buflen > MAX_BUFLEN){
      state = buffer_overrun;
      break;
    }
    if (digitalRead(PIN_SS) == 0){
      state = puedo_recibir;
      break;
    }
    state = proceso_mensaje;
  case proceso_mensaje:
    buff[buflen] = 0;
    mivga.print(buf);
    break;
  case buffer_overrun:
    state = ocioso;
  }

}
