#include <SPI.h>

#define PIN_SS 10
#define PIN_RTR 4
#define MAX_BUFLEN 16
#define START_TIMER()
#define STOP_TIMER()

typedef enum states {ocioso, quiero_mandar, mando_byte, byte_mandado, timeout} state_t;

state_t state;
char buff[MAX_BUFLEN] = {0};

void setup(){
  state = ocioso;
  pinMode (PIN_SS , OUTPUT);
  pinMode (PIN_RTR , INPUT);
  Serial.begin(115200);
}

void loop(){
  uint8_t buflen , i;
  switch (state) {
  case ocioso:
    digitalWrite(PIN_SS , 1);
    STOP_TIMER();
    i = 0;
    buflen = 0;
    while Serial.available() {
      buff[buflen++] = Serial.read();
    }
    if (buflen > 0) state = quiero_mandar;
    break;
  case quiero_mandar:
    SPI.beginTransaction(SPISettings (4000000 , MSBFIRST , SPI_MODE0));
    digitalWrite(PIN_SS , 0);
    START_TIMER();
    while (digitalRead(PIN_RTR) == 0);
    state = mando_byte;
  case mando_byte:
    digitalWrite(PIN_SS , 1);
    digitalWrite(PIN_SS , 0); //this reset the SPI buffer on slave to synchronize byte transfer
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
