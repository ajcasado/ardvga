//#include <avr/interrupt.h>
#include <SPI.h>
#define PIN_SS 10 //PB2
#define SET_SS_HIGH() PORTB |= (1 << PB2) //digitalWrite(PIN_SS , 1)
#define SET_SS_LOW() PORTB &= ~(1 << PB2) //digitalWrite(PIN_SS , 0)
#define PIN_RTR 6 //PD6
#define GET_RTR_VALUE() (PIND & (1<<PD6)) //digitalRead(PIN_RTR)// 0 for LOW or > 0 for HIGH
#define MAX_BUFLEN 32
#define START_TIMER() TCNT1 = 0 ; TCCR1B |= (1 << CS12) | (1 << CS10)   // enable timer with 1024 prescaler, clear counter.
#define STOP_TIMER() TCCR1B &= ~(1 << CS12) & ~(1 << CS10) // disable timer.
#define nop() __asm__("nop\n\t") //Wait one clock cycle

volatile uint8_t timeoutflag = 0;
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
}
void loop(){ //hacer función para transferencia y dejar en loop solamente entrada de texto y llamada a función
  ocioso:
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
    buff[buflen] = 0;
    Serial.print(F("Sending:"));
    Serial.println(buff);
    Serial.println(buflen);
    SPI.beginTransaction(SPISettings (250000 , MSBFIRST , SPI_MODE3));
    SET_SS_LOW();
    START_TIMER();
  mando_byte:
    while (GET_RTR_VALUE() && !timeoutflag);
    if (timeoutflag) goto timeout;
//    SET_SS_HIGH();//this resets the SPI buffer on the slave to synchronize byte transfer
//    nop();
//    nop(); //Keep SS HIGH for 4 cycles to make effect on slave
//    SET_SS_LOW();
    cli(); //avoid delay setting SS HIGH
    SPI.transfer(buff[i++]); //I guess from SPI library source that this is a blocking transfer
    if (--buflen == 0){
      SET_SS_HIGH();
      sei();
      SPI.endTransaction();
      goto ocioso;
    }
    sei();
//    SET_SS_HIGH();//this resets the SPI buffer on the slave to synchronize byte transfer
//    nop();
//    nop(); //Keep SS HIGH for 4 cycles to make effect on slave
//    SET_SS_LOW();
    while (!GET_RTR_VALUE() && !timeoutflag);
//    while (GET_RTR_VALUE() && !timeoutflag);
    if (timeoutflag) goto timeout;
    goto mando_byte;
  timeout:
    SPI.endTransaction();
    Serial.println(F("TIMEOUT!"));
    timeoutflag = 0;
    goto ocioso;
}
ISR (TIMER1_COMPA_vect){
  timeoutflag =1;
}
