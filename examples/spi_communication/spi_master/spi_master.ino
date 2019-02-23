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
#define PIN_MOSI 11
#define PIN_MISO 12
#define PIN_MOSI 13

volatile uint8_t timeoutflag = 0;


void setup(){
  pinMode (PIN_SS , OUTPUT);
  pinMode (PIN_SCK , OUTPUT);
  pinMode (PIN_MOSI , OUTPUT);
  digitalWrite(PIN_SS , HIGH);
  digitalWrite(PIN_MOSI , HIGH);
  digitalWrite(PIN_MOSI , HIGH);
  pinMode (PIN_MISO , INPUT);
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
void loop(){
  char buff[MAX_BUFLEN] = {0};
  uint8_t buflen , error;
  SET_SS_HIGH();//redundant but safe
  STOP_TIMER();//redundant but safe
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
  error = sendSPI(buff , buflen);
  switch (error){
    case 0:
      Serial.print(F(">OK!"));
      break;
    case 1:
      Serial.print(F(">TIMEOUT!"));
      break;
    default:
      Serial.print(F(">unknown error"));
  }
}
uint8_t sendSPI (char *buff , uint8_t buflen){
  uint8_t i = 0;
  SET_SS_LOW();
  START_TIMER();
  SPI.beginTransaction(SPISettings (250000 , MSBFIRST , SPI_MODE3));//250kHz tested as max usable SPI frequency for slave to catch clock transitions
  for (;;){ // actual sending loop
    while (GET_RTR_VALUE() && !timeoutflag);// wait for slave to be ready
    if (timeoutflag) break;//check timeout on waiting slave to be ready
  //    SET_SS_HIGH();//this resets the SPI buffer on the slave to synchronize byte transfer
  //    nop();
  //    nop(); //Keep SS HIGH for 4 cycles to make effect on slave
  //    SET_SS_LOW();
    SPI.transfer(buff[i++]); //I guess from SPI library source that this is a blocking transfer
    cli(); //disable interrupts to avoid extra delay at setting SS HIGH as this could let slave waiting until timepout
    if (--buflen == 0){// check that sending buffer is empty
      SET_SS_HIGH(); // tell slave we're done
      sei(); //reenable interrupts
      STOP_TIMER(); //stop timeout timer counter
      SPI.endTransaction();//release SPI port
      return 0;//return with 0 error code (no error)
    }
    sei();//reenable interrupts
    while (!GET_RTR_VALUE() && !timeoutflag);//wait slave ready for next buffer element
    if (timeoutflag) break;//check timeout on waiting slave to be ready
}
SPI.endTransaction(); // only can reach this point because of timeout, release SPI port
SET_SS_HIGH();//tell slave not sending anything more
STOP_TIMER();//stop timeout timer counter
Serial.println(F("TIMEOUT!"));
timeoutflag = 0; //reset timeout flag
return 1;//return with 1 error code (timeout)
}
ISR (TIMER1_COMPA_vect){
  timeoutflag = 1; //set timeout flag
}
