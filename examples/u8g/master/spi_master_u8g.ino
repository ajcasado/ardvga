//#include <avr/interrupt.h>

//OK Hacer spi_master_u8g con bucle para timeout en vez de usar timer para no interferir con marlin
//OK Hacer spi_master_u8g sin usar librería SPI
//OK hacer sendByte y sendBuffer (64 bytes->4 lineas) en spi_master_u8g
//OK hacer initCom
//por último intentar spi_master_u8g, spi_master, spi_slave_u8g y spi_slave usando rtr como retroaviso de bit recibido (cambio de sck) y no de byte recibido (spif=1)=> especie de SW_ASPI a ver si se consigue mayor velocidad de transferencia
#define PIN_SS 10 //PB2
#define SET_SS_HIGH() PORTB |= (1 << PB2) //digitalWrite(PIN_SS , 1)
#define SET_SS_LOW() PORTB &= ~(1 << PB2) //digitalWrite(PIN_SS , 0)
#define PIN_RTR 6 //PD6
#define GET_RTR_VALUE() (PIND & (1<<PD6)) //digitalRead(PIN_RTR)// 0 for LOW or > 0 for HIGH
#define MAX_BUFLEN 128
#define nop() __asm__("nop\n\t") //Wait one clock cycle
#define PIN_MOSI 11
#define PIN_MISO 12
#define PIN_SCK 13
#define TIMEOUT_COUNT 20000


void setup(){
  pinMode (PIN_SS , OUTPUT);
  pinMode (PIN_SCK , OUTPUT);
  pinMode (PIN_MOSI , OUTPUT);
  digitalWrite(PIN_SS , HIGH);
  digitalWrite(PIN_MOSI , HIGH);
  digitalWrite(PIN_SCK , HIGH);
  pinMode (PIN_MISO , INPUT);
  pinMode (PIN_RTR , INPUT);
  Serial.begin(115200);
  initCom();
  }
void loop(){
  char buff[MAX_BUFLEN] = {0};
  uint8_t buflen , error;
  SET_SS_HIGH();//redundant but safe
  buflen = 0;
  while (!Serial.available());
  while ((Serial.available()) && (buflen < MAX_BUFLEN)) {
    buff[buflen++] = Serial.read();
  }
  buff[buflen] = 0;
  //buffer de argumentos para setpixel o putchar y setattr
  sprintf_P(buff, PSTR("abcdefghijklmnopqrstuvwz012345678abcdefghijklmnopqrstuvwz0123456"));
  buflen = 64;
  Serial.print(F("Sending:"));
  Serial.println(buff);
  Serial.println(buflen);
  error = sendBuffer(buff , buflen);
  switch (error){
    case 0:
      Serial.println(F(">OK!"));
      break;
    case 1:
      Serial.println(F(">TIMEOUT!"));
      break;
    default:
      Serial.println(F(">unknown error"));
  }
  error = sendByte('$');
  switch (error){
    case 0:
      Serial.println(F(">OK!"));
      break;
    case 1:
      Serial.println(F(">TIMEOUT!"));
      break;
    default:
      Serial.println(F(">unknown error"));
  }
}

void initCom(){
  SPCR = (1 << SPE) | (1 << MSTR) | (1 << CPOL) | (1 << CPHA) | (1 << SPR1);
//  SPSR |= (1 << SPI2X);
}

uint8_t sendByte (char data){
  SET_SS_LOW();
  uint16_t c = 0;
  while (GET_RTR_VALUE() && (c < TIMEOUT_COUNT)) c++;// wait for slave to be ready
  if (c == TIMEOUT_COUNT) return 1;//check timeout on waiting slave to be ready
  SPDR = data;
  while(!(SPSR & (1<<SPIF)));
  SET_SS_HIGH();
  c = 0;
  while (!GET_RTR_VALUE() && (c < TIMEOUT_COUNT)) c++;//wait slave ack
  if (c == TIMEOUT_COUNT) return 1;//check timeout on waiting slave ack
  return 0;
}

uint8_t sendBuffer (char *buff , uint8_t buflen){
  uint8_t i = 0;
  SET_SS_LOW();
  for (;;){ // actual sending loop
    uint16_t c = 0;
    while (GET_RTR_VALUE() && (c < TIMEOUT_COUNT)) c++;// wait for slave to be ready
    if (c == TIMEOUT_COUNT) break;//check timeout on waiting slave to be ready
    SPDR = buff[i++];
    while(!(SPSR & (1<<SPIF)));//while ( UCSRA & (1<<RXC) ) dummy = UDR;
    cli(); //disable interrupts to avoid extra delay at setting SS HIGH as this could let slave waiting until timeout
    if (--buflen == 0){// check that sending buffer is empty
      SET_SS_HIGH(); // tell slave we're done
      sei(); //reenable interrupts
      return 0;//return with 0 error code (no error)
    }
    sei();//reenable interrupts
    c = 0;
    while (!GET_RTR_VALUE() && (c < TIMEOUT_COUNT)) c++;//wait slave ready for next buffer element
    if (c == TIMEOUT_COUNT) break;//check timeout on waiting slave to be ready
  }
  SET_SS_HIGH();//tell slave not sending anything more
  return 1;//return with 1 error code (timeout)
}
