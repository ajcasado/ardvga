//#include <avr/interrupt.h>
#include <SPI.h>
#define PIN_SS 10 //PB2
#define SET_SS_HIGH() PORTB |= (1 << PB2) //digitalWrite(PIN_SS , 1)
#define SET_SS_LOW() PORTB &= ~(1 << PB2) //digitalWrite(PIN_SS , 0)
#define PIN_RTR 6 //PD6
#define GET_RTR_VALUE() (PIND & (1<<PD6)) //digitalRead(PIN_RTR)// 0 for LOW or > 0 for HIGH
#define MAX_BUFLEN 16
#define START_TIMER() TCNT1 = 0 ; TCCR1B |= (1 << CS12) | (1 << CS10)   // enable timer with 1024 prescaler, clear counter.
#define STOP_TIMER() TCCR1B &= ~(1 << CS12) & ~(1 << CS10) // disable timer.
#define nop() __asm__("nop\n\t") //Wait one clock cycle
typedef enum states {ocioso, quiero_mandar, mando_byte, byte_mandado, timeout} state_t;
volatile state_t state;
char buff[MAX_BUFLEN] = {0};
uint8_t buflen , i;

void setup(){
  pinMode (PIN_SS , OUTPUT);
  pinMode (13 , OUTPUT);
  pinMode (12 , INPUT);
  pinMode (11, OUTPUT);
  digitalWrite(PIN_SS,HIGH);
  digitalWrite(11,HIGH);
  digitalWrite(13,HIGH);
  pinMode (PIN_RTR , INPUT);
  Serial.begin(115200);
  //set timer1 interrupt at 1Hz -amandaghassaei https://www.instructables.com/id/Arduino-Timer-Interrupts/
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
    Serial.print(F("O\n"));
    SET_SS_HIGH();
    STOP_TIMER();
    i = 0;
    buflen = 0;
    Serial.print(F("Insert a string of at most "));
    Serial.print(MAX_BUFLEN , DEC);
    Serial.println(F(" characters:"));
    while (!Serial.available());
    while ((Serial.available()) && (buflen < MAX_BUFLEN)) {
      buff[buflen++] = Serial.read();
    }
    Serial.print(F("Sending:"));
    Serial.println(buff);
    Serial.println(buflen);
    //if (buflen > 0) state = quiero_mandar;
    state = quiero_mandar;
  case quiero_mandar:
    Serial.print(F("Q\n"));
    SPI.beginTransaction(SPISettings (4000000 , MSBFIRST , SPI_MODE3));
    SET_SS_LOW();
    START_TIMER();
    while ((GET_RTR_VALUE() > 0) && (state != timeout)){
    }
    if (state == timeout) break;
    state = mando_byte;
  case mando_byte:
    Serial.print(F("M\n"));
    SET_SS_HIGH();//this resets the SPI buffer on the slave to synchronize byte transfer
//    nop();
//    nop(); //Keep SS HIGH for 4 cycles to make effect on slave
    SET_SS_LOW();
    SPI.transfer(buff[i++]); //I guess from SPI library source that this is a blocking transfer
    while ((GET_RTR_VALUE() == 0) && (state != timeout));
    if (state == timeout) break;
    state = byte_mandado;
  case byte_mandado:
    Serial.print(F("B\n"));
    if (--buflen == 0){
      SPI.endTransaction();
      state = ocioso;
      break;
    }
    Serial.println(buflen);
    while ((GET_RTR_VALUE() > 0) && (state != timeout));
    if (state == timeout) break;
    state = mando_byte;
    break;
  case timeout:
    SPI.endTransaction();
    Serial.println(F("TIMEOUT!"));
    state = ocioso;
  }
}
ISR (TIMER1_COMPA_vect){
  state = timeout;
}
