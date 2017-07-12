#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include<avr/interrupt.h>  //15625
int lcount=0,rcount=0;
double ltim=0,ltimo=0,rtim=0,rtimo=0,lfradps=0,rfradps=0;
int counter=0,i=0,lrevs=0,rrevs=0,lstopf=0,rstopf=0;
int ovf=0,j=0;
void timer0init(){
  TCCR0A|=(1<<WGM00)|(1<<WGM01)|(1<<COM0A1)|(1<<COM0B1);
  TCCR0B|=(1<<CS01);
  TIMSK0|=(1<<OCIE0A)|(1<<OCIE0B);
}
void timer4init(){
  TCCR4A |= (1 << WGM41);
  TCCR4B |=  (1 << CS42);
  TIMSK4 |= 1 << OCIE4A | 1 << ICIE4;
  TCCR4B |= 1 << ICES4; 
  OCR4A=15625;
}
void timer5init(){
  TCCR5A |= (1 << WGM51);
  TCCR5B |=  (1 << CS52);
  TIMSK5 |= 1 << OCIE5A | 1 << ICIE5;
  TCCR5B |= 1 << ICES5; 
  OCR5A=15625;
}
void setduty(int l,int r){
  OCR0A=255*(l)/100;
  OCR0B=255*(r)/100;
}
float distst(){
  return (float(rrevs)+(float(rcount))/40)*(8*34)*.8;
}
void odoreset(){
  cli();
  lrevs=0;
  lcount=0;
  rrevs=0;
  rcount=0;
  sei();
}
float angturn(){
  return distst()*180.0/(3.1*80);
}
void revtrans(int dis){
  odoreset();
  PORTA=0;
  double e,eo,sum,kp,ki,kd;
  while((e=dis-distst())>=0){
    PORTA|=(1<<1)|(1<<2);//REVERSE
  }
  PORTA=0;
  odoreset();
}
void fronttrans(int dis){
  odoreset();
  PORTA=0;
  double e,eo,sum,kp,ki,kd;
  while((e=dis-distst())>=0){
    PORTA|=(1<<0)|(1<<3);//FRONT
    Serial.println(lfradps);
  }
  PORTA=0;
  odoreset();
}
void leftrot(int ang){
  odoreset();
  PORTA=0;
  while(angturn()<=ang){
     PORTA|=(1<<0)|(1<<2);//LEFT
  }
  PORTA=0;
  odoreset();
}
void rightrot(int ang){
  odoreset();
  PORTA=0;
  while(angturn()<=ang){
     PORTA|=(1<<1)|(1<<3);//RIGHT
  }
  PORTA=0;
  odoreset();
}
int main(void){
  DDRB|=1<<PB7;
  DDRG|=1<<PG5;
  DDRL=0;
  PORTL=0xff;
  timer0init();
  setduty(75,75);
  DDRA=0b11111111;
  sei();
  Serial.begin(9600);
  /*EICRA|=(1<<ISC00);
  EIMSK|=(1<<INT0);
  EICRA|=(1<<ISC10);
  EIMSK|=(1<<INT1);*/
  while(1){
    fronttrans(100);
    rightrot(90);
    }
  
  
}
/*ISR(INT0_vect){
  lcount++;
  if(lcount>=40){
    lcount=0;
    //PORTB&=(0<<1);
    lrevs++;
  }
}
ISR(INT1_vect){
  rcount++;
  if(rcount>=40){
    rcount=0;
    //PORTB&=(0<<2);
    rrevs++;
  }
}*/
ISR(TIMER0_COMPA_vect){
  
}
ISR(TIMER0_COMPB_vect){
  
}
ISR(TIMER4_COMPA_vect){
  lstopf=1;
  odoreset();
}
ISR(TIMER5_COMPA_vect){
  rstopf=1;
  odoreset();
}
ISR(TIMER4_CAPT_vect){
  lcount++;
  if(lcount==40){
    lrevs++;
    lcount=0;
  }
  ltim=ICR4;
  lfradps=40/(2*3.14*0.000016*(ltim-ltimo));
  ltimo=ltim;
  TCNT4=0;
}
ISR(TIMER5_CAPT_vect){
  rcount++;
  if(rcount==40){
    rrevs++;
    rcount=0;
  }
  rtim=ICR5;
  rfradps=40/(2*3.14*0.000016*(rtim-rtimo));
  rtimo=rtim;
  TCNT5=0;
}
