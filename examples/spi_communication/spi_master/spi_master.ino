#include <SPI.h>

#define PIN_SS 10
#define PIN_RTR 4
#define MAX_BUFLEN 16
#define START_TIMER() TIMSK1 |= (1 << OCIE1A) ; TCNT1 = 0  // enable timer compare interrupt
#define STOP_TIMER() TIMSK1 &= ~(1 << OCIE1A)

typedef enum states {ocioso, quiero_mandar, mando_byte, byte_mandado, timeout} state_t;

volatile state_t state;
char buff[MAX_BUFLEN] = {0};

void setup(){
  state = ocioso;
  pinMode (PIN_SS , OUTPUT);
  pinMode (PIN_RTR , INPUT);
  Serial.begin(115200);

  //set timer1 interrupt at 1Hz -amandaghassaei https://www.instructables.com/id/Arduino-Timer-Interrupts/

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
    while ((digitalRead(PIN_RTR) == 0) && (state != timeout));
    if (state == timeout) break;
    state = mando_byte;
  case mando_byte:
    digitalWrite(PIN_SS , 1);
    digitalWrite(PIN_SS , 0); //this reset the SPI buffer on slave to synchronize byte transfer
    SPI.transfer(buff[i++]); //I guess from SPI source that is a blocking transfer
    while ((digitalRead(PIN_RTR) == 1) && (state != timeout));
    if (state == timeout) break;
    state = byte_mandado;
  case byte_mandado:
    buflen--;
    if (buflen == 0){
      SPI.endTransaction();
      state = ocioso;
      break;
    }
    while ((digitalRead(PIN_RTR) == 0) && (state != timeout));
    if (state == timeout) break;
    state = mando_byte;
    break;
  case timeout:
    SPI.endTransaction();
    Serial.print(F("TIMEOUT!"))
    state = ocioso;
  }
}

ISR (TIMER1_COMPA_vect){
  state = timeout;
}
