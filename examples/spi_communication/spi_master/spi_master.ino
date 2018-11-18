#include <SPI.h>

#define VGA_SS 8
#define VGA_ACK 2
#define ACK_PORTIN PIND
#define ACK_PIN PD2

volatile uint8_t i = 0;
char buf[15] = {0} , rec[15] = {0};


void setup() {
  Serial.begin(115200);
  Serial.println(F("Puerto serie inicializado"));
  pinMode(VGA_ACK,INPUT);
  pinMode(VGA_SS,OUTPUT);
  digitalWrite(VGA_SS,HIGH);
  Serial.println(F("Abriendo SPI"));
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));
  SPI.begin();
  attachInterrupt(digitalPinToInterrupt(VGA_ACK),isCts,CHANGE);
}

void isCts(){
  SPI.transfer(buf[i++]);
  SPI.transfer(buf[i]);
}
void loop() {
  i=0;
  unsigned long timeout = 0;
  while (i<16){
    Serial.print(i,DEC);
    Serial.print(F(":Cmd="));
    while (!Serial.available());
    buf[i]=Serial.read();
    Serial.println(buf[i++]);
  }
  i=0;
  digitalWrite(VGA_SS,LOW);
  timeout = millis();
  while ((i<16) && ((millis() - timeout) < 20));
  digitalWrite(VGA_SS,HIGH);
}
