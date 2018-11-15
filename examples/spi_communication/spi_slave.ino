#include "ardvga.h"

#define PIN_SS 10
#define PNI_MOSI 11
#define PNI_MISO 12
#define PNI_SCK 13
#define PIN_RTR 4
#define MAX_BUFLEN 16
#define START_TIMER()
#define STOP_TIMER()

typedef enum states {ocioso, puedo_recibir, recibo_byte, proceso_byte, buffer_overrun, timeout} state_t;

state_t state;
char buff[MAX_BUFLEN] = {};

void setup(){
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
    digitalWrite(PIN_RTR , 0);
    STOP_TIMER();
    while (digitalRead(PIN_SS) == 0);
    state = puedo_recibir;
  case quiero_mandar:
    SPI.beginTransaction(SPISettings (8000000 , MSBFIRST , SPI_MODE0));
    digitalWrite(PIN_SS , 0);
    START_TIMER();
    while (digitalRead(PIN_RTR) == 0);
    state = mando_byte;
  case mando_byte:
    SPI.transfer(buff[i++]); //I guess from SPI source that is a blocking transfer
    while (digitalRead(PIN_RTR) == 1);
    state = byte_mandado;
  case byte_mandado:
    buflen--;
    if (buflen == 0){
      state = ocioso;
      break;
    }
    while (digitalRead(PIN_RTR) == 0);
    state = mando_byte;
  }

}
